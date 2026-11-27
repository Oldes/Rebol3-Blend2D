//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// Project: Rebol/Blend2D extension
// SPDX-License-Identifier: Apache-2.0
// =============================================================================
// The `image` command and the BLImage handle.
//

#include "gen-blend2d.h"
#include "blend2d-command.h"

static const REBYTE *ERR_NO_HANDLE = (const REBYTE*)"Blend2D failed to make an image handle!";
static const REBYTE *ERR_BAD_IMAGE = (const REBYTE*)"Blend2D failed to initialize the image!";
static const REBYTE *ERR_PIXELS    = (const REBYTE*)"Blend2D failed to convert the pixels!";


BLResult b2d_init_image_from_file(BLImageCore *image, REBSER *file_name) {
	BLArrayCore codecs;
	BLResult r;
	bl_image_codec_array_init_built_in_codecs(&codecs);
	r = bl_image_read_from_file(image, SERIES_TEXT(file_name), &codecs);
	bl_array_reset(&codecs);
	return r;
}

// Initializes `image` from a Rebol value: an image! is wrapped in place (the
// pixels are shared, not copied), a file! is decoded, a pair! makes a new
// empty image of that size.
BLResult b2d_init_image_from_arg(BLImageCore *image, RXIARG *arg, REBCNT type) {
	REBSER *file;
	switch (type) {
	case RXT_IMAGE:
		bl_image_reset(image);
		return bl_image_create_from_data(image, arg->width, arg->height, BL_FORMAT_PRGB32,
			SERIES_DATA((REBSER*)arg->image), (intptr_t)arg->width * 4, BL_DATA_ACCESS_RW, NULL, NULL);
	case RXT_FILE:
		file = b2d_file_arg(arg, type);
		if (file == NULL) return BL_ERROR_INVALID_VALUE;
		bl_image_reset(image);
		return b2d_init_image_from_file(image, file);
	case RXT_PAIR:
		// NOTE: the size of a pair! is in `pair.x`/`pair.y`; the `width` and
		// `height` fields of the union belong to the image! layout only.
		bl_image_reset(image);
		return bl_image_create(image, ROUND_TO_INT(arg->pair.x), ROUND_TO_INT(arg->pair.y), BL_FORMAT_PRGB32);
	}
	return BL_ERROR_INVALID_VALUE;
}


COMMAND cmd_blend2d_image(RXIFRM *frm, void *ctx) {
	BLResult r;
	BLImageCore *image;
	REBHOB *hob = RL_MAKE_HANDLE_CONTEXT(Handle_BLImage);

	if (hob == NULL) RETURN_ERROR(ERR_NO_HANDLE);

	image = (BLImageCore*)hob->data;
	debug_print("New image handle: %u data: %p\n", hob->sym, (void*)hob->data);
	bl_image_init(image);

	r = b2d_init_image_from_arg(image, &RXA_ARG(frm, 1), RXA_TYPE(frm, 1));
	if (r != BL_SUCCESS) RETURN_ERROR(ERR_BAD_IMAGE);

	// When the pixels belong to a Rebol image!, they are shared and not
	// copied - keeping the series on the handle is what stops the garbage
	// collector from freeing the buffer Blend2D still writes into.
	if (RXA_TYPE(frm, 1) == RXT_IMAGE) hob->series = (REBSER*)RXA_ARG(frm, 1).image;

	RETURN_HANDLE(hob);
}



/* Converts the pixels of a Rebol image in place, between the straight alpha
   of an image! and Blend2D's premultiplied PRGB32. */
static BLResult b2d_premultiply_pixels(REBYTE *pixels, REBCNT w, REBCNT h, REBOOL back)
{
	BLFormatInfo straight, premul;
	BLPixelConverterCore pc;
	intptr_t stride = (intptr_t)w * 4;
	BLResult r;

	b2d_rebol_pixel_format(&straight);
	r = bl_format_info_query(&premul, BL_FORMAT_PRGB32);
	if (r != BL_SUCCESS) return r;

	bl_pixel_converter_init(&pc);
	r = bl_pixel_converter_create(&pc,
		back ? &straight : &premul,
		back ? &premul : &straight,
		BL_PIXEL_CONVERTER_CREATE_NO_FLAGS);
	if (r == BL_SUCCESS) {
		/* src == dst: every output pixel depends only on the input pixel at
		   the same offset, which is why bl_image_convert converts in place
		   itself whenever the depths match and the image is mutable. */
		r = bl_pixel_converter_convert(&pc, pixels, stride, pixels, stride, w, h, NULL);
	}
	bl_pixel_converter_reset(&pc);
	return r;
}

COMMAND cmd_blend2d_premultiply(RXIFRM *frm, void *ctx) {
	REBSER *ser  = (REBSER*)RXA_ARG(frm, 1).image;
	REBCNT  w    = RXA_IMAGE_WIDTH(frm, 1);
	REBCNT  h    = RXA_IMAGE_HEIGHT(frm, 1);
	REBOOL  back = (REBOOL)RXA_REF(frm, 2);

	if (ser == NULL || w == 0 || h == 0) return RXR_VALUE;
	if (BL_SUCCESS != b2d_premultiply_pixels(SERIES_DATA(ser), w, h, back))
		RETURN_ERROR(ERR_PIXELS);
	return RXR_VALUE;   /* slot 1 still holds the image - modified in place */
}


//== handle callbacks =========================================================

int BLImage_free(void *data) {
	debug_print("releasing image: %p\n", data);
	if (data) bl_image_destroy((BLImageCore*)data);
	return 0;
}

int BLImage_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	BLImageData data;
	if (bl_image_get_data((BLImageCore*)hob->data, &data) != BL_SUCCESS) return PE_BAD_SELECT;

	switch (RL_FIND_WORD(Blend2d_arg_words, word)) {
	case W_BLEND2D_ARG_SIZE:
		*type = RXT_PAIR;
		arg->pair.x = (float)data.size.w;
		arg->pair.y = (float)data.size.h;
		break;
	case W_BLEND2D_ARG_WIDTH:
		*type = RXT_INTEGER;
		arg->int64 = (i64)data.size.w;
		break;
	case W_BLEND2D_ARG_HEIGHT:
		*type = RXT_INTEGER;
		arg->int64 = (i64)data.size.h;
		break;
	case W_BLEND2D_ARG_FORMAT:
		*type = RXT_INTEGER;
		arg->int64 = (i64)data.format;
		break;
	case W_BLEND2D_ARG_STRIDE:
		*type = RXT_INTEGER;
		arg->int64 = (i64)data.stride;
		break;
	default:
		return PE_BAD_SELECT;
	}
	return PE_USE;
}

int BLImage_mold(REBHOB *hob, REBSER *str) {
	BLImageData data;
	int len = 0;

	if (!str || !hob || !hob->data) return 0;
	SERIES_TAIL(str) = 0;
	if (bl_image_get_data((BLImageCore*)hob->data, &data) != BL_SUCCESS) return 0;

	APPEND_STRING(str, "0#%lx %ix%i", (unsigned long)(uintptr_t)hob->data, (int)data.size.w, (int)data.size.h);
	return len;
}