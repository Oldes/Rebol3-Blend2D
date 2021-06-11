//
// Blend2D experimental Rebol extension
// ====================================
// Use on your own risc!

#include "blend2d-rebol-extension.h"

// Function for quick native experimenting
int b2d_draw_test(RXIFRM *frm, void *reb_ctx) {
	BLResult r;
	BLImageCore img;
	BLContextCore ctx;
	REBXYF size;
	REBINT w, h;
	REBSER *reb_img = 0;
	
	if (RXA_TYPE(frm, 1) == RXT_PAIR) {
		size = RXA_PAIR(frm, 1);
		w = ROUND_TO_INT(size.x);
		h = ROUND_TO_INT(size.y);
		reb_img = (REBSER *)RL_MAKE_IMAGE(w,h);
	}
	else {
		w = RXA_IMAGE_WIDTH(frm, 1);
		h = RXA_IMAGE_HEIGHT(frm, 1);
		reb_img = (REBSER *)RXA_ARG(frm,1).image;
	}
	RXA_TYPE(frm, 1) = RXT_IMAGE;
	RXA_ARG(frm, 1).width = w;
	RXA_ARG(frm, 1).height = h;
	RXA_ARG(frm, 1).image = reb_img;

	blImageInit(&img);
	r = blImageCreateFromData(&img, w, h, BL_FORMAT_PRGB32, reb_img->data, (intptr_t)w * 4, NULL, NULL);
	if (r != BL_SUCCESS) return r;

	r = blContextInitAs(&ctx, &img, NULL);
	if (r != BL_SUCCESS) return r;

	// now process some drawing...

	BLGradientCore gradient;
	BLLinearGradientValues values = { 0, 0, 256, 256 };
	r = blGradientInitAs(&gradient,
	BL_GRADIENT_TYPE_LINEAR, &values,
	BL_EXTEND_MODE_PAD, NULL, 0, NULL);
	if (r != BL_SUCCESS) return 1;

	blGradientAddStopRgba32(&gradient, 0.0, 0xFFFFFFFFu);
	blGradientAddStopRgba32(&gradient, 0.5, 0xFFFFAF00u);
	blGradientAddStopRgba32(&gradient, 1.0, 0xFFFF0000u);

	blContextSetFillStyleObject(&ctx, &gradient);
	blContextFillAll(&ctx);
	blGradientReset(&gradient);

	BLCircle circle;
	circle.cx = 128;
	circle.cy = 128;
	circle.r = 64;

	blContextSetCompOp(&ctx, BL_COMP_OP_EXCLUSION);
	blContextSetFillStyleRgba32(&ctx, 0xFF00FFFFu);
	blContextFillGeometry(&ctx, BL_GEOMETRY_TYPE_CIRCLE, &circle);

	blContextSetCompOp(&ctx, BL_COMP_OP_DIFFERENCE);
	blContextSetFillStyleRgba32(&ctx, 0xFF00FF10u);

	double dArgs[3] = {0.785398, 80.0, 80.0};

	blContextMatrixOp(&ctx, BL_MATRIX2D_OP_ROTATE_PT, dArgs);

	BLRoundRect rr;
	rr.x = 50;
	rr.y = 25;
	rr.w = 170;
	rr.h = 170;
	rr.rx = 25;
	rr.ry = 25;
	r = blContextFillGeometry(&ctx, BL_GEOMETRY_TYPE_ROUND_RECT, &rr);

	blContextMatrixOp(&ctx, BL_MATRIX2D_OP_RESET, NULL);

	// GRADIENT test
	blContextSetCompOp(&ctx, BL_COMP_OP_SRC_OVER);

	BLGradientCore gr;
	blGradientInit(&gr);

	double gradValues[4] = {-819.2, 0, 819.2, 0};

	BLRectI ri;
	ri.x = 0;
	ri.y = 0;
	ri.w = 100;
	ri.h = 100;

	blGradientCreate(&gr, BL_GRADIENT_TYPE_LINEAR, gradValues, BL_EXTEND_MODE_PAD, NULL, 0, NULL);

	blGradientAddStopRgba32(&gr, 0.0, 0xFFFFFFFFu);
	blGradientAddStopRgba32(&gr, 1.0, 0xFF000000u);

	dArgs[0] = dArgs[1] = 0.06103515625;
	blGradientApplyMatrixOp(&gr, BL_MATRIX2D_OP_POST_SCALE, dArgs);

	dArgs[0] = dArgs[1] = 50;
	blGradientApplyMatrixOp(&gr, BL_MATRIX2D_OP_POST_TRANSLATE, dArgs);

/*
	double gradValues[4] = {0, 0, 255, 255};

	blGradientCreate(&gr, BL_GRADIENT_TYPE_LINEAR, gradValues, BL_EXTEND_MODE_PAD, NULL, 0, NULL);
*/
	blContextSetFillStyleObject(&ctx, &gradient);
	r = blContextFillGeometry(&ctx, BL_GEOMETRY_TYPE_RECTI, &ri);


	// TEXT test....

	BLPointI pos = {60, 80};
	//REBYTE txt[100]; sprintf_s(txt, 100, "%s", "Hello\nBlend2D!");
	REBYTE txt[] =  "Hello Blend2D!";

	BLFontFaceCore face;
	blFontFaceInit(&face);
	r = blFontFaceCreateFromFile(&face, "NotoSans-Regular.ttf", BL_FILE_READ_MMAP_ENABLED | BL_FILE_READ_MMAP_AVOID_SMALL);
	if (r != BL_SUCCESS) {
		debug_print("Failed to load a font-face (err=%u)\n", r);
		goto end_ctx;
	}
	BLFontCore font;
	blFontInit(&font);
	blFontCreateFromFace(&font, &face, 24.0f);
	blContextSetFillStyleRgba32(&ctx, 0xFFFF22FFu);
	blContextFillTextI(&ctx, &pos, &font, txt, sizeof(txt), BL_TEXT_ENCODING_UTF8);

end_ctx:

	// END ...
	blContextEnd(&ctx);
	trace("ok");

	// output to file...
	BLImageCodecCore codec;
	blImageCodecInit(&codec);
	blImageCodecFindByName(&codec, "BMP", SIZE_MAX, NULL);
	blImageWriteToFile(&img, "test.bmp", &codec);
	blImageCodecReset(&codec);

	blImageReset(&img);
	blContextReset(&ctx);
	return 0;
}
