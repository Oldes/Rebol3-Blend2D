//
// Blend2D experimental Rebol extension
// ====================================
// Use on your own risc!

#include "blend2d-rebol-extension.h"

void* releasePath(void* path) {
	//debug_print("releasing path: %p\n", path);
	blPathDestroy(path);
	return NULL;
}

REBCNT b2d_init_path_from_block(BLPathCore* path, REBSER* cmds, REBCNT index) {
	REBCNT cmd, type, cap, mode, count, i;
	REBCNT cmd_pos = 0;
	BLPoint pos;

	while (index < cmds->tail) {
		if (!fetch_word(cmds, index++, b2d_cmd_words, &cmd)) {
			trace("expected word as a command!");
			goto error;
		}
	process_cmd: // label is used from error loop which skip all args until it reaches valid command

		cmd_pos = index; // this could be used to report error position
		//debug_print("cmd index: %u\n", index);
		switch (cmd) {
		case W_B2D_CMD_MOVE:
			RESOLVE_PAIR_ARG(0, 0);
			blPathMoveTo(path, doubles[0], doubles[1]);
			break;
		case W_B2D_CMD_LINE:
			RESOLVE_PAIR_ARG(0, 0);
			blPathLineTo(path, doubles[0], doubles[1]);
			while (RXT_PAIR == RL_GET_VALUE(cmds, index, &arg[0])) {
				index++;
				blPathLineTo(path, (double)arg[0].pair.x, (double)arg[0].pair.y);
			}
			break;
		case W_B2D_CMD_ARC:
			RESOLVE_PAIR_ARG(0, 3);   // arc's end point
			RESOLVE_NUMBER_ARG(1, 0); // radius of the circle along x axis
			RESOLVE_NUMBER_ARG(2, 1); // radius of the circle along y axis
			RESOLVE_NUMBER_ARG(3, 2); // rotation angle of the underlying ellipse in degrees

			TO_RADIANS(doubles[2]);
			REBOOL sweepFlag = FALSE;
			REBOOL largeFlag = FALSE;

			OPT_WORD_FLAG(sweepFlag, W_B2D_ARG_SWEEP);
			OPT_WORD_FLAG(largeFlag, W_B2D_ARG_LARGE);

			blPathEllipticArcTo(path, doubles[0], doubles[1], doubles[2], largeFlag, sweepFlag, doubles[3], doubles[4]);
			break;
		case W_B2D_CMD_CURVE:
			// A cubic Bézier curve is defined by a start point, an end point, and two control points.
			while (
				RXT_PAIR == RL_GET_VALUE(cmds, index,     &arg[0]) &&
				RXT_PAIR == RL_GET_VALUE(cmds, index + 1, &arg[1]) &&
				RXT_PAIR == RL_GET_VALUE(cmds, index + 2, &arg[2])
			) {
				index += 3;
				blPathCubicTo(path, ARG_X(0), ARG_Y(0), ARG_X(1), ARG_Y(1), ARG_X(2), ARG_Y(2));
			}
			break;
		case W_B2D_CMD_CURV:
			while (
				RXT_PAIR == RL_GET_VALUE(cmds, index, &arg[0]) &&
				RXT_PAIR == RL_GET_VALUE(cmds, index + 1, &arg[1])
			) {
				index += 2;
				blPathSmoothCubicTo(path, ARG_X(0), ARG_Y(0), ARG_X(1), ARG_Y(1));
			}
			break;
		case W_B2D_CMD_QCURVE:
			// A quadratic Bézier curve is defined by a start point, an end point, and one control point.
			while (
				RXT_PAIR == RL_GET_VALUE(cmds, index, &arg[0]) &&
				RXT_PAIR == RL_GET_VALUE(cmds, index + 1, &arg[1])
			) {
				index += 2;
				blPathQuadTo(path, ARG_X(0), ARG_Y(0), ARG_X(1), ARG_Y(1));
			}
			break;
		case W_B2D_CMD_QCURV:
			while (RXT_PAIR == RL_GET_VALUE(cmds, index, &arg[0])) {
				index += 1;
				blPathSmoothQuadTo(path, ARG_X(0), ARG_Y(0));
			}
			break;
		case W_B2D_CMD_HLINE:
			RESOLVE_NUMBER_ARG(0, 0);
			blPathGetLastVertex(path, &pos);
			blPathLineTo(path, doubles[0], pos.y);
			break;
		case W_B2D_CMD_VLINE:
			RESOLVE_NUMBER_ARG(0, 0);
			blPathGetLastVertex(path, &pos);
			blPathLineTo(path, pos.x, doubles[0]);
			break;
		case W_B2D_CMD_CLOSE:
			blPathClose(path);
			break;
		default:
			debug_print("unknown command.. index: %u\n", index);
			goto error;
		} // switch end
		continue;
	error:
		// command errors does not stop evaluation... remaining commands may be processed..
		debug_print("CMD error at index... %u\n", cmd_pos);
		index = cmd_pos;
		// find next valid command name
		while (index < cmds->tail) {
			if (fetch_word(cmds, index++, b2d_cmd_words, &cmd)) {
				goto process_cmd;
			}
		}
	} // while end
}

REBCNT b2d_path(RXIFRM* frm, void* reb_ctx) {
	debug_print("pathHandleId: %u\n", Handle_BLPath);
	REBHOB* hob = RL_MAKE_HANDLE_CONTEXT(Handle_BLPath);
	if (hob == NULL) {
		trace("Failed to make path handle!");
		return 1;
	}
	BLPathCore* path = (BLPathCore*)hob->data;
	debug_print("New path handle: %u data: %p\n", hob->sym, hob->data);
	blPathInit(path);

	b2d_init_path_from_block(path, RXA_SERIES(frm, 1), RXA_INDEX(frm, 1));

	hob->flags |= HANDLE_CONTEXT; //@@ temp fix!
	RXA_HANDLE(frm, 1) = hob;
	RXA_HANDLE_TYPE(frm, 1) = hob->sym;
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
	RXA_TYPE(frm, 1) = RXT_HANDLE;
	return 0;
}
