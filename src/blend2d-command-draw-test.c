//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// Project: Rebol/Blend2D extension
// SPDX-License-Identifier: Apache-2.0
// =============================================================================
// The `draw-test` command - a scratch pad for quick native experiments.
// It draws a fixed gradient into the target image and returns it.
//
// NOTE: the previous version also wrote the result to %test.bmp in the current
// directory and returned unset; the image is returned instead, so a test can
// simply `save %test.bmp draw-test 256x256`.
//

#include "gen-blend2d.h"
#include "blend2d-command.h"

static const REBYTE *ERR_TARGET  = (const REBYTE*)"Blend2D failed to attach the target image!";
static const REBYTE *ERR_CONTEXT = (const REBYTE*)"Blend2D failed to initialize a rendering context!";


COMMAND cmd_blend2d_draw_test(RXIFRM *frm, void *ctx_unused) {
	BLResult r;
	BLImageCore img;
	BLContextCore ctx;
	BLGradientCore gradient;
	BLLinearGradientValues values = { 0, 0, 256, 256 };
	REBINT w, h;
	REBSER *reb_img;

	reb_img = b2d_target_image(frm, &w, &h);
	if (reb_img == NULL) RETURN_ERROR(ERR_TARGET);

	blImageInit(&img);
	r = blImageCreateFromData(&img, w, h, BL_FORMAT_PRGB32,
		SERIES_DATA(reb_img), (intptr_t)w * 4, BL_DATA_ACCESS_WRITE, NULL, NULL);
	if (r != BL_SUCCESS) {
		blImageReset(&img);
		RETURN_ERROR(ERR_TARGET);
	}

	r = blContextInitAs(&ctx, &img, NULL);
	if (r != BL_SUCCESS) {
		blImageReset(&img);
		RETURN_ERROR(ERR_CONTEXT);
	}

	// now process some drawing...
	r = blGradientInitAs(&gradient, BL_GRADIENT_TYPE_LINEAR, &values, BL_EXTEND_MODE_PAD, NULL, 0, NULL);
	if (r == BL_SUCCESS) {
		blGradientAddStopRgba32(&gradient, 0.0, 0xFFFFFFFFu);
		blGradientAddStopRgba32(&gradient, 0.5, 0xFFFFAF00u);
		blGradientAddStopRgba32(&gradient, 1.0, 0xFFFF0000u);

		blContextSetFillStyle(&ctx, &gradient);
		blContextFillAll(&ctx);
		blGradientReset(&gradient);
	}

	blContextEnd(&ctx);
	blContextReset(&ctx);
	blImageReset(&img);
	trace("ok");

	return RXR_VALUE;
}