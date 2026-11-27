//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// Project: Rebol/Blend2D extension
// SPDX-License-Identifier: Apache-2.0
// =============================================================================
// Blend2D's image codecs - PNG, JPEG, BMP and QOI - as Rebol codecs and as
// the `encode`, `decode` and `codecs` commands.
//
// All four codecs both read and write, so a host built without its own image
// codecs gets them by importing this module. The dispatchers below have the
// signature DO-CODEC expects, so from sys-codec.reb's point of view they are
// indistinguishable from an internal codec:
//
//     decode 'png bin  ->  do-codec system/codecs/png/entry 'decode bin
//
// The module's mezzanine registers a format only where `system/codecs` has no
// codec of that name yet: importing the module for `draw` must not silently
// replace a codec the host already has.
//
// Two things to know about the pixels:
//
//   1. Blend2D works with PREMULTIPLIED pixels (BL_FORMAT_PRGB32) while a
//      Rebol image! keeps straight alpha, so every pixel crossing the border
//      goes through a BLPixelConverter. An opaque image is handed to the
//      encoders as XRGB32 - then the conversion is only a permutation of the
//      bytes and the result is exact (and the file smaller). An image with
//      partial alpha cannot round-trip exactly through Blend2D; see README.
//
//   2. DO-CODEC frees the codec's output with Free_Mem(ptr, size), which
//      pairs with RL_ALLOC only. RL_MEM_ALLOC must NOT be used for `data` or
//      `bits` - it returns a pointer past a hidden header and, for small
//      sizes, one which belongs to a memory pool.
//

#include "gen-blend2d.h"
#include "blend2d-command.h"

static const REBYTE *ERR_NO_CODEC_WORD =
	(const REBYTE*)"The `codec` word could not be mapped - the host is too old for this extension.";
static const REBYTE *ERR_BAD_FORMAT = (const REBYTE*)"Blend2D has codecs for png, jpeg, bmp and qoi only!";
static const REBYTE *ERR_BAD_IMAGE  = (const REBYTE*)"Blend2D failed to prepare the image!";
static const REBYTE *ERR_ENCODE     = (const REBYTE*)"Blend2D failed to encode the image!";
static const REBYTE *ERR_DECODE     = (const REBYTE*)"Blend2D failed to decode the image!";
static const REBYTE *ERR_NO_MEMORY  = (const REBYTE*)"Not enough memory for the result!";

// Word id of `codec`, the handle type DO-CODEC accepts. Resolved once by
// Blend2d_Init - RL_MAP_WORD interns a word and there is no reason to do that
// on every call.
u32 Blend2d_codec_word = 0;


//== codec names ==============================================================

// The dialect words are mapped to the names Blend2D registers its built-in
// codecs under. `bl_image_codec_find_by_name` is case sensitive.
static const char *b2d_codec_name(REBCNT word) {
	switch (RL_FIND_WORD(Blend2d_arg_words, word)) {
	case W_BLEND2D_ARG_PNG:  return "PNG";
	case W_BLEND2D_ARG_JPEG: return "JPEG";
	case W_BLEND2D_ARG_BMP:  return "BMP";
	case W_BLEND2D_ARG_QOI:  return "QOI";
	}
	return NULL;
}

static BLResult b2d_find_codec(BLImageCodecCore *codec, const char *name) {
	bl_image_codec_init(codec);
	// A NULL array means the built-in codecs.
	return bl_image_codec_find_by_name(codec, name, SIZE_MAX, NULL);
}


//== pixels ===================================================================

// TRUE when no pixel is translucent. Such an image is given to the encoders
// as XRGB32, which needs no premultiplication and so loses nothing.
static REBOOL b2d_pixels_are_opaque(const REBYTE *src, REBCNT pixels) {
	for (; pixels > 0; pixels--, src += 4) {
		if (src[C_A] != 0xFF) return FALSE;
	}
	return TRUE;
}

// Copies a decoded image into a Rebol pixel buffer of w * h * 4 bytes,
// undoing the premultiplication the decoders applied.
static BLResult b2d_image_to_pixels(const BLImageCore *img, REBYTE *dst) {
	BLImageData data;
	BLFormatInfo dst_info, src_info;
	BLPixelConverterCore pc;
	BLResult r;

	r = bl_image_get_data(img, &data);
	if (r != BL_SUCCESS) return r;

	r = bl_format_info_query(&src_info, (BLFormat)data.format);
	if (r != BL_SUCCESS) return r;

	b2d_rebol_pixel_format(&dst_info);

	bl_pixel_converter_init(&pc);
	r = bl_pixel_converter_create(&pc, &dst_info, &src_info, BL_PIXEL_CONVERTER_CREATE_NO_FLAGS);
	if (r == BL_SUCCESS) {
		r = bl_pixel_converter_convert(&pc,
			dst, (intptr_t)data.size.w * 4,
			data.pixel_data, data.stride,
			(uint32_t)data.size.w, (uint32_t)data.size.h, NULL);
	}
	bl_pixel_converter_reset(&pc);
	return r;
}

// Makes an image the encoders accept out of a Rebol pixel buffer. The image
// owns its pixels - `src` is only read.
static BLResult b2d_image_from_pixels(BLImageCore *img, const REBYTE *src, int w, int h) {
	BLImageData data;
	BLFormatInfo src_info, dst_info;
	BLPixelConverterCore pc;
	BLFormat format;
	BLResult r;

	if (w <= 0 || h <= 0) return BL_ERROR_INVALID_VALUE;

	format = b2d_pixels_are_opaque(src, (REBCNT)w * (REBCNT)h)
		? BL_FORMAT_XRGB32 : BL_FORMAT_PRGB32;

	r = bl_image_create(img, w, h, format);
	if (r != BL_SUCCESS) return r;

	r = bl_image_get_data(img, &data);
	if (r != BL_SUCCESS) return r;

	r = bl_format_info_query(&dst_info, format);
	if (r != BL_SUCCESS) return r;

	b2d_rebol_pixel_format(&src_info);

	bl_pixel_converter_init(&pc);
	r = bl_pixel_converter_create(&pc, &dst_info, &src_info, BL_PIXEL_CONVERTER_CREATE_NO_FLAGS);
	if (r == BL_SUCCESS) {
		r = bl_pixel_converter_convert(&pc,
			data.pixel_data, data.stride,
			src, (intptr_t)w * 4,
			(uint32_t)w, (uint32_t)h, NULL);
	}
	bl_pixel_converter_reset(&pc);
	return r;
}


//== encode / decode ==========================================================

// Decodes `data` with the named codec. The name is never guessed from the
// data here: `decode 'png bin` must refuse a JPEG, like the internal codecs
// do, and the codec dispatchers below are only reached once ENCODING? has
// already identified the format.
static BLResult b2d_decode_image(BLImageCore *img, const void *data, size_t size, const char *name) {
	BLImageCodecCore codec;
	BLImageDecoderCore decoder;
	BLResult r;

	r = b2d_find_codec(&codec, name);
	if (r == BL_SUCCESS) {
		bl_image_decoder_init(&decoder);
		r = bl_image_codec_create_decoder(&codec, &decoder);
		if (r == BL_SUCCESS) {
			r = bl_image_decoder_read_frame(&decoder, img, (const uint8_t*)data, size);
		}
		bl_image_decoder_reset(&decoder);
	}
	bl_image_codec_reset(&codec);
	return r;
}

static BLResult b2d_encode_image(BLArrayCore *out, const BLImageCore *img, const char *name) {
	BLImageCodecCore codec;
	BLResult r = b2d_find_codec(&codec, name);
	if (r == BL_SUCCESS) {
		r = bl_image_write_to_data(img, out, &codec);
	}
	bl_image_codec_reset(&codec);
	return r;
}


//== codec actions ============================================================

static void b2d_codec_decode(REBCDI *codi, const char *name) {
	BLImageCore img;
	BLImageData data;
	size_t size;

	codi->bits = NULL;
	bl_image_init(&img);

	if (BL_SUCCESS != b2d_decode_image(&img, codi->data, (size_t)codi->len, name)
	 || BL_SUCCESS != bl_image_get_data(&img, &data)) {
		bl_image_reset(&img);
		codi->error = CODI_ERR_BAD_DATA;
		return;
	}

	codi->w = (int)data.size.w;
	codi->h = (int)data.size.h;
	size = (size_t)codi->w * (size_t)codi->h * 4;

	codi->bits = (unsigned int*)RL_ALLOC(size);
	if (codi->bits == NULL) {
		bl_image_reset(&img);
		codi->error = CODI_ERR_BAD_DATA;
		return;
	}

	if (BL_SUCCESS != b2d_image_to_pixels(&img, (REBYTE*)codi->bits)) {
		RL_FREE(codi->bits, size);
		codi->bits  = NULL;
		bl_image_reset(&img);
		codi->error = CODI_ERR_BAD_DATA;
		return;
	}

	bl_image_reset(&img);
	codi->error = 0;
}

static void b2d_codec_encode(REBCDI *codi, const char *name) {
	BLImageCore img;
	BLArrayCore out;
	const void *bytes;
	size_t size;

	codi->data = NULL;
	codi->len  = 0;

	if (codi->w <= 0 || codi->h <= 0 || codi->bits == NULL) {
		codi->error = CODI_ERR_BAD_DATA;
		return;
	}

	bl_image_init(&img);
	if (BL_SUCCESS != b2d_image_from_pixels(&img, (const REBYTE*)codi->bits, codi->w, codi->h)) {
		bl_image_reset(&img);
		codi->error = CODI_ERR_BAD_DATA;
		return;
	}

	bl_array_init(&out, BL_OBJECT_TYPE_ARRAY_UINT8);
	if (BL_SUCCESS != b2d_encode_image(&out, &img, name)) {
		bl_array_reset(&out);
		bl_image_reset(&img);
		codi->error = CODI_ERR_ENCODING;
		return;
	}
	bl_image_reset(&img);

	bytes = bl_array_get_data(&out);
	size  = bl_array_get_size(&out);

	if (size == 0 || (codi->data = (unsigned char*)RL_ALLOC(size)) == NULL) {
		bl_array_reset(&out);
		codi->error = CODI_ERR_ENCODING;
		return;
	}
	memcpy(codi->data, bytes, size);
	bl_array_reset(&out);

	codi->len   = (u32)size;
	codi->error = 0;
}

// ENCODING? hands every codec whatever binary it was given, including a very
// short one, so the length is the codec's own business - Blend2D's inspectors
// take the size and never read past it.
static void b2d_codec_identify(REBCDI *codi, const char *name) {
	BLImageCodecCore codec;
	uint32_t score = 0;

	if (BL_SUCCESS == b2d_find_codec(&codec, name)) {
		score = bl_image_codec_inspect_data(&codec, codi->data, (size_t)codi->len);
	}
	bl_image_codec_reset(&codec);

	codi->error = (score == 0);   // inverted result: not an image of this format
}


//== dispatchers ==============================================================

// Matches `codo` in reb-codec.h. There is no user data in that signature, so
// each format gets its own two line entry point.
static REBINT b2d_codec_action(REBCDI *codi, const char *name) {
	codi->error = 0;

	switch (codi->action) {
	case CODI_IDENTIFY:
		b2d_codec_identify(codi, name);
		return CODI_CHECK;   // the error code is the inverted result

	case CODI_DECODE:
		b2d_codec_decode(codi, name);
		return CODI_IMAGE;

	case CODI_ENCODE:
		b2d_codec_encode(codi, name);
		return CODI_BINARY;
	}

	codi->error = CODI_ERR_NA;
	return CODI_ERROR;
}

static REBINT Codec_B2D_PNG (REBCDI *codi) { return b2d_codec_action(codi, "PNG");  }
static REBINT Codec_B2D_JPEG(REBCDI *codi) { return b2d_codec_action(codi, "JPEG"); }
static REBINT Codec_B2D_BMP (REBCDI *codi) { return b2d_codec_action(codi, "BMP");  }
static REBINT Codec_B2D_QOI (REBCDI *codi) { return b2d_codec_action(codi, "QOI");  }

static void *b2d_codec_dispatcher(REBCNT word) {
	switch (RL_FIND_WORD(Blend2d_arg_words, word)) {
	case W_BLEND2D_ARG_PNG:  return (void*)Codec_B2D_PNG;
	case W_BLEND2D_ARG_JPEG: return (void*)Codec_B2D_JPEG;
	case W_BLEND2D_ARG_BMP:  return (void*)Codec_B2D_BMP;
	case W_BLEND2D_ARG_QOI:  return (void*)Codec_B2D_QOI;
	}
	return NULL;
}


//== commands =================================================================

COMMAND cmd_blend2d_codec_handle(RXIFRM *frm, void *ctx) {
	void *dispatcher;

	if (!Blend2d_codec_word) RETURN_ERROR(ERR_NO_CODEC_WORD);

	dispatcher = b2d_codec_dispatcher(RXA_WORD(frm, 1));
	if (dispatcher == NULL) RETURN_ERROR(ERR_BAD_FORMAT);

	// A plain function handle - there is no context to allocate and nothing
	// for the GC to mark, the same shape SET_HANDLE gives the internal
	// codecs in Register_Codec.
	RXA_HANDLE(frm, 1)       = dispatcher;
	RXA_HANDLE_TYPE(frm, 1)  = Blend2d_codec_word;
	RXA_HANDLE_FLAGS(frm, 1) = HANDLE_FUNCTION;
	RXA_TYPE(frm, 1)         = RXT_HANDLE;
	return RXR_VALUE;
}


COMMAND cmd_blend2d_encode(RXIFRM *frm, void *ctx) {
	BLImageCore img;
	BLImageCore *src;
	BLArrayCore out;
	BLResult r;
	REBSER *bin;
	size_t size;
	const char *name = b2d_codec_name(RXA_WORD(frm, 1));

	if (name == NULL) RETURN_ERROR(ERR_BAD_FORMAT);

	bl_image_init(&img);

	if (RXA_TYPE(frm, 2) == RXT_HANDLE) {
		// A Blend2D image handle is already in the engine's own format -
		// whatever the user put in it is what gets written.
		if (!VAL_IS_HANDLE(RXA_ARG(frm, 2), Handle_BLImage)) RETURN_ERROR(ERR_BAD_IMAGE);
		src = (BLImageCore*)RXA_ARG(frm, 2).handle.hob->data;
	}
	else {
		r = b2d_image_from_pixels(&img,
			(const REBYTE*)SERIES_DATA((REBSER*)RXA_ARG(frm, 2).image),
			RXA_IMAGE_WIDTH(frm, 2), RXA_IMAGE_HEIGHT(frm, 2));
		if (r != BL_SUCCESS) {
			bl_image_reset(&img);
			RETURN_ERROR(ERR_BAD_IMAGE);
		}
		src = &img;
	}

	bl_array_init(&out, BL_OBJECT_TYPE_ARRAY_UINT8);
	r = b2d_encode_image(&out, src, name);
	bl_image_reset(&img);

	if (r != BL_SUCCESS) {
		bl_array_reset(&out);
		RETURN_ERROR(ERR_ENCODE);
	}

	size = bl_array_get_size(&out);
	bin  = (REBSER*)RL_MAKE_BINARY((u32)size);
	if (bin == NULL) {
		bl_array_reset(&out);
		RETURN_ERROR(ERR_NO_MEMORY);
	}
	// NOTE: SERIES_DATA, not RL_SERIES - the latter returns a REBUPT, which is
	// `unsigned long` (32 bit) wherever the amalgamated header does not take
	// its C99 branch, so casting it to a pointer truncates on Windows x64.
	memcpy(SERIES_DATA(bin), bl_array_get_data(&out), size);
	SERIES_TAIL(bin) = (REBCNT)size;
	bl_array_reset(&out);

	RXA_SERIES(frm, 1) = bin;
	RXA_INDEX(frm, 1)  = 0;
	RXA_TYPE(frm, 1)   = RXT_BINARY;
	return RXR_VALUE;
}


COMMAND cmd_blend2d_decode(RXIFRM *frm, void *ctx) {
	BLImageCore img;
	BLImageData data;
	BLResult r;
	REBSER *ser;
	REBSER *bin = RXA_SERIES(frm, 1);
	REBCNT  idx = RXA_INDEX(frm, 1);
	const REBYTE *bytes = (const REBYTE*)SERIES_DATA(bin) + idx;
	size_t  size = (size_t)(SERIES_TAIL(bin) - idx);
	int w, h;

	bl_image_init(&img);

	// Without a format word the codec is the one which recognizes the data.
	r = bl_image_read_from_data(&img, bytes, size, NULL);
	if (r != BL_SUCCESS || BL_SUCCESS != bl_image_get_data(&img, &data)) {
		bl_image_reset(&img);
		RETURN_ERROR(ERR_DECODE);
	}

	w = (int)data.size.w;
	h = (int)data.size.h;

	// The image is made last and filled immediately: until the RXIARG below
	// reaches Rebol nothing references the new series, so no call which may
	// allocate is allowed in between.
	ser = (REBSER*)RL_MAKE_IMAGE((u32)w, (u32)h);
	if (ser == NULL) {
		bl_image_reset(&img);
		RETURN_ERROR(ERR_NO_MEMORY);
	}

	r = b2d_image_to_pixels(&img, SERIES_DATA(ser));
	bl_image_reset(&img);
	if (r != BL_SUCCESS) RETURN_ERROR(ERR_DECODE);

	RXA_ARG(frm, 1).image  = ser;
	RXA_ARG(frm, 1).width  = w;
	RXA_ARG(frm, 1).height = h;
	RXA_TYPE(frm, 1)       = RXT_IMAGE;
	return RXR_VALUE;
}


COMMAND cmd_blend2d_codecs(RXIFRM *frm, void *ctx) {
	// The four words are returned as a block, in the order the extension
	// knows them, so a script can ask what this build can read and write
	// without knowing the Blend2D version.
	static const REBCNT words[4] = {
		W_BLEND2D_ARG_PNG, W_BLEND2D_ARG_JPEG, W_BLEND2D_ARG_BMP, W_BLEND2D_ARG_QOI
	};
	RXIARG val;
	REBSER *blk = (REBSER*)RL_MAKE_BLOCK(4);
	REBCNT  i;

	if (blk == NULL) RETURN_ERROR(ERR_NO_MEMORY);

	memset(&val, 0, sizeof(val));
	for (i = 0; i < 4; i++) {
		val.int32a = (i32)Blend2d_arg_words[words[i]];
		RL_SET_VALUE(blk, i, val, RXT_WORD);
	}

	RXA_SERIES(frm, 1) = blk;
	RXA_INDEX(frm, 1)  = 0;
	RXA_TYPE(frm, 1)   = RXT_BLOCK;
	return RXR_VALUE;
}
