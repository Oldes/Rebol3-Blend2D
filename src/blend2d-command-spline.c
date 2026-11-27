//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// Project: Rebol/Blend2D extension
// SPDX-License-Identifier: Apache-2.0
// =============================================================================
// The `spline` command of the draw and path dialects.
//
// The arguments are parsed here for both dialects; the curve itself is solved
// in blend2d-spline.cpp.
//
//     spline [segments] [closed] <point> ... [closed]
//
// The points are given inline, as a block of pairs, or as an #(f64!) vector of
// x y pairs - the same three sources `line`, `point` and `triangle` accept.
//
// `closed` is accepted both before the points, where Rebol2 put it, and after
// them, where Red puts it. The leading segment count of the Rebol2 dialect is
// accepted and ignored: it told AGG how many line segments to emit per span,
// while here the solved curve is handed to Blend2D as cubics and flattened
// adaptively.
//

#include "gen-blend2d.h"
#include "blend2d-command.h"
#include "blend2d-spline.h"

// Points collected without touching the heap.
#define SPLINE_STACK_POINTS 64

typedef struct {
	REBDEC *xy;       // two doubles per point
	REBCNT  count;    // points collected so far
	REBCNT  capacity; // points the buffer can hold; 0 when borrowed
	REBOOL  heap;     // xy must be freed
	REBDEC  stack[SPLINE_STACK_POINTS * 2];
} SPLINE_POINTS;

static void spline_points_init(SPLINE_POINTS *p) {
	p->xy       = p->stack;
	p->count    = 0;
	p->capacity = SPLINE_STACK_POINTS;
	p->heap     = FALSE;
}

static REBOOL spline_points_add(SPLINE_POINTS *p, REBDEC x, REBDEC y) {
	if (p->count >= p->capacity) {
		REBCNT  capacity = p->capacity * 2;
		REBDEC *xy = (REBDEC *)RL_MEM_ALLOC(NULL, (size_t)capacity * 2 * sizeof(REBDEC));
		if (xy == NULL) return FALSE;
		memcpy(xy, p->xy, (size_t)p->count * 2 * sizeof(REBDEC));
		if (p->heap) RL_MEM_FREE(NULL, p->xy);
		p->xy       = xy;
		p->capacity = capacity;
		p->heap     = TRUE;
	}
	p->xy[p->count * 2    ] = x;
	p->xy[p->count * 2 + 1] = y;
	p->count++;
	return TRUE;
}

static void spline_points_free(SPLINE_POINTS *p) {
	if (p->heap) RL_MEM_FREE(NULL, p->xy);
	p->xy   = NULL;
	p->heap = FALSE;
}


// Evaluates the arguments of a `spline` command at *index_io and appends the
// solved spline to `path`. FALSE when the arguments were not understood - the
// caller then runs its own error recovery; *index_io is advanced either way.
REBOOL b2d_spline_command(BLPathCore *path, REBSER *cmds, REBCNT *index_io)
{
	SPLINE_POINTS pts;
	RXIARG arg[ARG_BUFFER_SIZE];
	REBCNT index = *index_io;
	REBCNT type, cmd = 0;
	REBOOL closed = FALSE;
	REBOOL ok = FALSE;

	spline_points_init(&pts);

	// Rebol2's number of segments per span (accepted, not used).
	RESOLVE_INT_ARG_OPTIONAL(0);

	// Rebol2 writes `closed` before the points...
	OPT_WORD_FLAG(closed, W_BLEND2D_ARG_CLOSED);

	type = RL_GET_VALUE_RESOLVED(cmds, index, &arg[0]);
	if (RXT_VECTOR == type) {
		REBSER *vect = (REBSER *)arg[0].vector.series;
		REBCNT  info = arg[0].vector.info;
		REBCNT  ind  = arg[0].vector.index;

		index++;
		if (!RXI_VECTOR_FLOAT(info) || RXI_VECTOR_BITS(info) != 64) goto error;

		// Used where it is - the solver only reads the coordinates.
		pts.xy       = (REBDEC *)SERIES_DATA(vect) + ind;
		pts.count    = (SERIES_TAIL(vect) - ind) >> 1; // 2 values per point
		pts.capacity = 0;
	}
	else if (RXT_BLOCK == type) {
		REBSER *blk = (REBSER *)arg[0].series;
		REBCNT  ind = arg[0].index;

		index++;
		while (RXT_PAIR == RL_GET_VALUE(blk, ind, &arg[0])) {
			if (!spline_points_add(&pts, arg[0].pair.x, arg[0].pair.y)) goto error;
			ind++;
		}
	}
	else {
		while (RXT_PAIR == RL_GET_VALUE_RESOLVED(cmds, index, &arg[0])) {
			if (!spline_points_add(&pts, arg[0].pair.x, arg[0].pair.y)) goto error;
			index++;
		}
	}

	// ...Red writes it after them.
	OPT_WORD_FLAG(closed, W_BLEND2D_ARG_CLOSED);

	if (pts.count == 0) goto error;

	debug_print("spline: %u knots%s\n", pts.count, closed ? " closed" : "");
	b2d_spline_to_path(path, pts.xy, pts.count, closed);
	ok = TRUE;

	// The RESOLVE_* macros jump here; it doubles as the cleanup path.
error:
	spline_points_free(&pts);
	*index_io = index;
	return ok;
}
