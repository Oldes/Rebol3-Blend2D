//
// Blend2D experimental Rebol extension
// ====================================
// Use on your own risc!

#include "blend2d-rebol-extension.h"

#ifndef BL_BUILD_STATIC
#pragma comment(lib,"blend2d.lib")
#endif

RL_LIB *RL; // Link back to reb-lib from embedded extensions

#define MIN_REBOL_VER 3
#define MIN_REBOL_REV 5
#define MIN_REBOL_UPD 4
#define VERSION(a, b, c) (a << 16) + (b << 8) + c
#define MIN_REBOL_VERSION VERSION(MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD)


//==== Globals ===============================//
u32*   b2d_cmd_words;
u32*   b2d_arg_words;
REBCNT Handle_BLPath;
REBCNT Handle_BLFontFace;

REBDEC doubles[DOUBLE_BUFFER_SIZE];
RXIARG arg[ARG_BUFFER_SIZE];
//============================================//

static const char* init_block = B2D_EXT_INIT_CODE;


RXIEXT const char *RX_Init(int opts, RL_LIB *lib) {
    RL = lib;
	REBYTE ver[8];
	RL_VERSION(ver);
	debug_print("RXinit b2d; Rebol v%i.%i.%i\n", ver[1], ver[2], ver[3]);

	if (MIN_REBOL_VERSION > VERSION(ver[1], ver[2], ver[3])) {
		printf("Needs at least Rebol v%i.%i.%i!\n", MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD);
		return 0;
	}
    if (!CHECK_STRUCT_ALIGN) {
    	trace("CHECK_STRUCT_ALIGN failed!");
    	return 0;
    }
	Handle_BLPath     = RL_REGISTER_HANDLE("BLPath", sizeof(BLPathCore), releasePath);
	Handle_BLFontFace = RL_REGISTER_HANDLE("BLFontFace", sizeof(BLFontFaceCore), releaseFontFace);
	debug_print("BLPath id: %i\n", Handle_BLPath);
	debug_print("BLFont id: %i\n", Handle_BLFontFace);
    return init_block;
}

RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *ctx) {
	REBINT r;
	switch (cmd) {
	case CMD_B2D_INIT_WORDS:
		b2d_cmd_words = RL_MAP_WORDS(RXA_SERIES(frm,1));
		b2d_arg_words = RL_MAP_WORDS(RXA_SERIES(frm,2));
		goto done;

	case CMD_B2D_DRAW:
		r = b2d_draw(frm, ctx);
		if (r == BL_SUCCESS) return RXR_VALUE;
		debug_print("error: %d\n", r);
		RXA_SERIES(frm,1) = "Blend2D draw failed!";
		return RXR_ERROR;

	case CMD_B2D_PATH:
		b2d_path(frm, ctx);
		return RXR_VALUE;

	case CMD_B2D_FONT:
		if (BL_SUCCESS == b2d_font(frm, ctx)) return RXR_VALUE;
		return RXR_NONE;

	case CMD_B2D_INFO:
		b2d_info(frm, ctx);
		return RXR_VALUE;
	}

//	case CMD_B2D_DRAW_TEST:
//		trace("draw test");
//		r = b2d_draw_test(frm, ctx);
//		if (r == BL_SUCCESS) return RXR_VALUE;
//		debug_print("error: %d\n", r);
//		goto done;

	if (ctx == NULL) {
		return RXR_ERROR;
	}

done:
    return RXR_NONE;
}

RXIEXT int RX_Quit(int opts) {
    return 0;
}
