//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// Project: Rebol/Blend2D extension
// SPDX-License-Identifier: Apache-2.0
// =============================================================================
// Spline support for the draw and path dialects.
//
// Blend2D has no spline primitive, so the curve is solved by Aurimas Gasiulis'
// S9 spline (https://github.com/aurimasg/splines, MIT) and appended to a
// BLPath as the cubics the solver already produces - one bl_path_cubic_to per
// curve, which Blend2D then flattens adaptively.
//
// This is the extension's only C++ translation unit; everything the dialect
// needs is declared in blend2d-spline.h with C linkage. It must be compiled
// as C++20: the solver's CurveArray uses `requires` clauses.
//

#include "blend2d-spline.h"

#include <stdlib.h> // malloc(), free()

#include "Splines/DeduplicatePoints.h"
#include "Splines/SplineS9.h"

// Knots up to this count are solved without touching the heap.
#define SPLINE_STACK_KNOTS 64


extern "C" uint32_t b2d_spline_to_path(BLPathCore *path, const double *xy,
	uint32_t count, int closed)
{
	if (path == NULL || xy == NULL || count == 0) return 0;

	FloatPoint stack_knots[SPLINE_STACK_KNOTS];
	FloatPoint *knots = stack_knots;
	BLPoint last = { 0.0, 0.0 };
	uint32_t n = 0;
	uint32_t segments = 0;

	// A spline continues the figure it is appended to: when the path has a
	// current position, that position is the first knot and no new subpath is
	// started. In the `draw` dialect the scratch path is always empty, so the
	// spline starts on its own first knot there.
	const bool continues = bl_path_get_size(path) != 0
		&& bl_path_get_last_vertex(path, &last) == BL_SUCCESS;

	const uint32_t needed = count + (continues ? 1u : 0u);
	if (needed > SPLINE_STACK_KNOTS) {
		knots = (FloatPoint *)malloc((size_t)needed * sizeof(FloatPoint));
		if (knots == NULL) return 0;
	}

	if (continues) knots[n++] = FloatPoint(last.x, last.y);
	for (uint32_t i = 0; i < count; i++) {
		knots[n++] = FloatPoint(xy[i * 2], xy[i * 2 + 1]);
	}

	// Every solver of the library requires consecutive knots to be distinct.
	// DeduplicatePoints never writes past the index it reads, so this is safe
	// in place.
	n = DeduplicatePoints(n, knots, knots);

	// A closed spline must not repeat its first knot at the end either - the
	// closing span is what joins the two.
	if (closed && n > 1 &&
		knots[n - 1].DistanceToSquared(knots[0]) <= DuplicatePointMaximumSquaredDistance)
	{
		n--;
	}

	if (n >= 3) {
		CurveArray<CubicCurve> curves;

		if (closed) {
			S9SolveCyclic(n, knots, curves);
		} else {
			S9SolveAcyclic(n, knots, curves);
		}

		const uint32_t count_out = curves.GetCurveCount();
		if (count_out != 0) {
			// The spline interpolates its knots, so the first point of the
			// solved curve is the current position already when we continue.
			const FloatPoint start = curves.GetPointAtIndex(0);
			if (!continues) bl_path_move_to(path, start.X, start.Y);

			for (uint32_t i = 0; i < count_out; i++) {
				const FloatPoint c1 = curves.GetPointAtIndex(i * 3 + 1);
				const FloatPoint c2 = curves.GetPointAtIndex(i * 3 + 2);
				const FloatPoint to = curves.GetPointAtIndex(i * 3 + 3);

				bl_path_cubic_to(path, c1.X, c1.Y, c2.X, c2.Y, to.X, to.Y);
			}
			segments = count_out;
		}
	}
	else if (n == 2) {
		// Documented behavior of the Red dialect: two knots are accepted and
		// give a straight line.
		if (!continues) bl_path_move_to(path, knots[0].X, knots[0].Y);
		bl_path_line_to(path, knots[1].X, knots[1].Y);
		segments = 1;
	}

	if (segments != 0 && closed) bl_path_close(path);

	if (knots != stack_knots) free(knots);
	return segments;
}
