//
// Blend2D experimental Rebol extension
// ====================================
// Use on your own risc!

#include "blend2d-rebol-extension.h"

void* releaseFontFace(void* font) {
	debug_print("releasing font: %p\n", font);
	blFontFaceDestroy((BLFontFaceCore*)font);
	return NULL;
}


REBCNT b2d_font(RXIFRM* frm, void* reb_ctx) {
	BLResult ret;
	REBSER* src;
	BLFontFaceCore* face = NULL;
	 
	REBHOB* hob = RL_MAKE_HANDLE_CONTEXT(Handle_BLFontFace);

	if (hob == NULL) {
		debug_print("Failed to make font handle!\n");
		return 1;
	}
	
	src = RXA_SERIES(frm, 1);
	src = RL_ENCODE_UTF8_STRING(SERIES_DATA(src), SERIES_TAIL(src), SERIES_WIDE(src) > 1, FALSE);

	face = (BLFontFaceCore*)hob->data;
	blFontInit(face);

	ret = blFontFaceCreateFromFile(face, SERIES_DATA(src), BL_FILE_READ_MMAP_ENABLED | BL_FILE_READ_MMAP_AVOID_SMALL);
	if (ret != BL_SUCCESS) {
		debug_print("Failed to load font: %s, reason: %i\n", SERIES_DATA(src), ret);
		return ret;
	}
	hob->flags |= HANDLE_CONTEXT; //@@ temp fix!
	RXA_HANDLE(frm, 1) = hob;
	RXA_HANDLE_TYPE(frm, 1) = hob->sym;
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
	RXA_TYPE(frm, 1) = RXT_HANDLE;
	return BL_SUCCESS;
}
