//
// Blend2D experimental Rebol extension
// ====================================
// Use on your own risc!

#include "blend2d-rebol-extension.h"
#include <stdio.h> // used in b2d_info function

#ifdef TO_WINDOWS
//#include <windows.h>
#endif

//#define USE_TRACES
#ifdef  USE_TRACES
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str) 
#endif

#define DOUBLE_BUFFER_SIZE 16
#define ARG_BUFFER_SIZE 8

const double pi1 = 3.14159265358979323846; 

static u32* b2d_cmd_words;
static u32* b2d_arg_words;

REBCNT b2d_draw( RXIFRM *frm, void *ctx );
int b2d_draw_test( RXIFRM *frm, void *ctx );
void b2d_info(void);

#define FETCH_3_PAIRS(c,i, a1, a2, a3) (\
	   RXT_PAIR == RL_GET_VALUE(c, i, &a1) \
	&& RXT_PAIR == RL_GET_VALUE(c, i+1, &a2) \
	&& RXT_PAIR == RL_GET_VALUE(c, i+2, &a3))

RXIEXT const char *RX_Init(int opts, RL_LIB *lib) {
    RL = lib;
    debug_print("RXinit b2d %zi %zi\n", sizeof(REBREQ) , sizeof(REBEVT));
    if (!CHECK_STRUCT_ALIGN) {
    	trace("CHECK_STRUCT_ALIGN failed!");
    	return 0;
    }
    return init_block;
}

RXIEXT int RX_Quit(int opts) {
    return 0;
}

RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *ctx) {
	REBINT r;
	switch (cmd) {
	case CMD_B2D_INIT_WORDS:
		//trace("init-words");
		b2d_cmd_words = RL_MAP_WORDS(RXA_SERIES(frm,1));
		b2d_arg_words = RL_MAP_WORDS(RXA_SERIES(frm,2));
		goto done;
	case CMD_B2D_DRAW:
		//trace("draw");
		r = b2d_draw(frm, ctx);
		if (r == BL_SUCCESS) return RXR_VALUE;
		debug_print("error: %d\n", r);
		RXA_SERIES(frm,1) = "Hay dood... what's up?";
		return RXR_ERROR;

	case CMD_B2D_DRAW_TEST:
		trace("draw test");
		r = b2d_draw_test(frm, ctx);
		if (r == BL_SUCCESS) return RXR_VALUE;
		debug_print("error: %d\n", r);
		
		goto done;
	case CMD_B2D_INFO:
		b2d_info();
		goto done;
	}

	if (ctx == NULL) {
		return RXR_ERROR;
	}

	switch (cmd) {
	case CMD_B2D_FILL_PEN:
		trace("fill-pen");
		break;
	}

done:
    return RXR_NONE;
}

REBOOL fetch_word(REBSER *cmds, REBCNT index, u32* words, REBCNT *cmd) {
	RXIARG arg;
	REBCNT type = RL_GET_VALUE(cmds, index, &arg);
	//debug_print("fetch_word: %u type: %u\n", index, type);
	return (RXT_WORD == type && (cmd[0] = RL_FIND_WORD(words, arg.int32a)));
}

REBOOL fetch_mode(REBSER *cmds, REBCNT index, REBCNT *result, REBCNT start, REBCNT max) {
	RXIARG arg;
	REBCNT wrd = max;
	REBCNT type = RL_GET_VALUE(cmds, index, &arg);

	if (RXT_WORD == type || RXT_LIT_WORD == type) {
		wrd = RL_FIND_WORD(b2d_arg_words, arg.int32a) - start;
	} else if (RXT_INTEGER == type) wrd = arg.int64;
	if (wrd >= 0 && wrd < max) {
		result[0] = wrd;
		return TRUE;
	}
	return FALSE;
}


#define TUPLE_TO_COLOR(t) (REBCNT)((t.bytes[0]==3?255:t.bytes[4])<<24 | (t.bytes[1])<<16 | (t.bytes[2])<<8 |  (t.bytes[3]))

REBOOL fetch_color(REBSER *cmds, REBCNT index, REBCNT *cmd) {
	RXIARG arg;
	REBCNT type = RL_GET_VALUE(cmds, index, &arg);
	//debug_print("fetch_command: %u type: %u\n", index, type);
	if (RXT_WORD == type) {
		cmd[0] = RL_FIND_WORD(b2d_arg_words, arg.int32a);
		return TRUE;
	}
	return FALSE;
}


REBCNT b2d_draw(RXIFRM *frm, void *reb_ctx) {
	BLResult r;
	BLImageCore img_target, img_pattern, img;
	BLPatternCore pattern;
	BLGradientCore gradient;
	BLPathCore path;
	BLFontCore font;
	BLFontFaceCore font_face;

	REBDEC doubles[DOUBLE_BUFFER_SIZE];
	RXIARG arg[ARG_BUFFER_SIZE];

	BLContextCore ctx;
	REBXYF size;
	REBINT w, h;
	REBSER *reb_img = 0;
	REBDEC alpha = 1, offset, sz = 0.0;
	REBCNT err = 0;
	REBSER *cmds;
	REBCNT index, cmd_pos, cmd, type, cap, mode, count, i;
	REBOOL has_fill = FALSE, has_stroke = FALSE;
	REBDEC x, y, width = 1;
	BLRect rect;
	BLRectI rectI;
	BLPoint pt;
	REBDEC font_size = 10.0;

#define RESOLVE_ARG(a) type = RL_GET_VALUE_RESOLVED(cmds, index++, &arg[a]);

#define RESOLVE_PAIR_ARG(a, d) RESOLVE_ARG(a) \
	if (type != RXT_PAIR) goto error; \
	doubles[d]   = arg[a].pair.x; \
	doubles[d+1] = arg[a].pair.y;

#define RESOLVE_PAIR_ARG_OPTIONAL(a, d) RESOLVE_ARG(a) \
	if (type == RXT_PAIR) { \
		doubles[d]   = arg[a].pair.x; \
		doubles[d+1] = arg[a].pair.y; \
	} else { type = NULL; index--; }

#define RESOLVE_INT_ARG(a) RESOLVE_ARG(a) \
	if (type != RXT_INTEGER) goto error;

#define RESOLVE_INT_ARG_OPTIONAL(a) RESOLVE_ARG(a) \
	if (type != RXT_INTEGER) {type = NULL; index--; }

#define RESOLVE_NUMBER_ARG(a, d) RESOLVE_ARG(a) \
	if (type == RXT_DECIMAL || type == RXT_PERCENT) doubles[d] = arg[a].dec64; \
	else if (type == RXT_INTEGER) doubles[d] = (double)arg[a].int64; \
	else goto error;

#define RESOLVE_NUMBER_ARG_OPTIONAL(a, d) RESOLVE_ARG(a) \
	if (type == RXT_DECIMAL || type == RXT_PERCENT) doubles[d] = arg[a].dec64; \
	else if (type == RXT_INTEGER) doubles[d] = (double)arg[a].int64; \
	else { type = NULL; index--;}

#define RESOLVE_NUMBER_OR_PAIR_ARG(a, d) RESOLVE_ARG(a) \
	if (type == RXT_DECIMAL || type == RXT_PERCENT) doubles[d] = doubles[d+1] = arg[a].dec64; \
	else if (type == RXT_INTEGER) doubles[d] = doubles[d+1] = (double)arg[a].int64; \
	else if (type == RXT_PAIR) {\
		doubles[d]   = arg[a].pair.x; \
		doubles[d+1] = arg[a].pair.y; \
	} else goto error;

#define RESOLVE_STRING_ARG(a) RESOLVE_ARG(a) \
	if (type != RXT_STRING) goto error;

#define GET_PAIR(a, d) RESOLVE_ARG(a) \
	if (type != RXT_PAIR) goto error; \
	doubles[d]   = arg[a].pair.x; \
	doubles[d+1] = arg[a].pair.y;

#define DRAW_GEOMETRY(ctx, mode, path) \
	if (has_fill  ) blContextFillGeometry  (&ctx, mode, path);\
	if (has_stroke) blContextStrokeGeometry(&ctx, mode, path);


	blPathInit(&path);
	blImageInit(&img_target);
	blImageInit(&img_pattern);
	blFontInit(&font);
	blFontFaceInit(&font_face);
	
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

	r = blImageCreateFromData(&img_target, w, h, BL_FORMAT_PRGB32, reb_img->data, (intptr_t)w * 4, NULL, NULL);
	if (r != BL_SUCCESS) return r;

	r = blContextInitAs(&ctx, &img_target, NULL);
	if (r != BL_SUCCESS) return r;

	cmds = RXA_SERIES(frm, 2);
	index = RXA_INDEX(frm, 2);
				
	while (index < cmds->tail) {
		if (!fetch_word(cmds, index++, b2d_cmd_words, &cmd)) {
			trace("expected word as a command!");
			goto error;
		}
	process_cmd: // label is used from error loop which skip all args until it reaches valid command

		cmd_pos = index; // this could be used to report error position
		//debug_print("cmd index: %u\n", index);
		switch (cmd) {

		case W_B2D_CMD_FILL	:
		case W_B2D_CMD_FILL_PEN:

			RESOLVE_ARG(0)
			if (RXT_TUPLE == type) {
				blContextSetFillStyleRgba32(&ctx, TUPLE_TO_COLOR(arg[0]));
				has_fill = TRUE;
			} else if (type == RXT_LOGIC) {
				if(!arg[0].int32a) has_fill = FALSE;
			} else if (type == RXT_IMAGE) {
				blImageInit(&img_pattern);
				r = blImageCreateFromData(&img_pattern, arg[0].width, arg[0].height, BL_FORMAT_PRGB32, ((REBSER*)arg[0].series)->data, (intptr_t)arg[0].width * 4, NULL, NULL);
				if (r != BL_SUCCESS) {
					trace("failed to init pattern image!");
					goto error;
				}

				if (fetch_mode(cmds, index, &mode, W_B2D_ARG_PAD, BL_EXTEND_MODE_COMPLEX_COUNT)) {
					index++;
				} else {
					mode = BL_EXTEND_MODE_REPEAT;
				}
				BLMatrix2D m;
				blMatrix2DSetIdentity(&m);
				blPatternInitAs(&pattern, &img_pattern, NULL, mode , NULL);
				blContextSetFillStyleObject(&ctx, &pattern);
				has_fill = TRUE;
				blPatternReset(&pattern);
			} else if (RXT_UNSET == type && fetch_mode(cmds, index-1, &mode, W_B2D_ARG_LINEAR, BL_GRADIENT_TYPE_COUNT)) {
				// gradient fill
				blGradientInit(&gradient);
				blGradientCreate(&gradient, mode, doubles, 0, NULL, 0, NULL);
				//blGradientSetExtendMode(&gradient, 0);
				RESOLVE_ARG(0)
				while (type == RXT_TUPLE) {
					RESOLVE_NUMBER_ARG(2, 1);
					offset = doubles[1];
					//debug_print("gradstop: %lf\n", offset);
					blGradientAddStopRgba32(&gradient, offset, TUPLE_TO_COLOR(arg[0]));
					RESOLVE_ARG(0)
				}
				index--;

				RESOLVE_PAIR_ARG(0, 0);
				RESOLVE_PAIR_ARG(0, 2);
				if (mode == BL_GRADIENT_TYPE_RADIAL) {
					RESOLVE_NUMBER_ARG(3, 4);
					blGradientSetValues(&gradient, 0, doubles, 5);
				}
				else {
					blGradientSetValues(&gradient, 0, doubles, 4);
				}
				blContextSetFillStyleObject(&ctx, &gradient);
				has_fill = TRUE;
				blGradientReset(&gradient);

			} else goto error;
			break;


		case W_B2D_CMD_PEN:
			RESOLVE_ARG(0)
			if (RXT_TUPLE == type) {
				blContextSetStrokeStyleRgba32(&ctx, TUPLE_TO_COLOR(arg[0]));
				has_stroke = TRUE;
			} else if (type == RXT_LOGIC) {
				if(!arg[0].int32a) has_stroke = FALSE;
			} else if (type == RXT_IMAGE) {
				blImageInit(&img_pattern);
				r = blImageCreateFromData(&img_pattern, arg[0].width, arg[0].height, BL_FORMAT_PRGB32, ((REBSER*)arg[0].series)->data, (intptr_t)arg[0].width * 4, NULL, NULL);
				if (r != BL_SUCCESS) {
					trace("failed to init pattern image!");
					goto error;
				}

				if (fetch_mode(cmds, index, &mode, W_B2D_ARG_PAD, BL_EXTEND_MODE_COMPLEX_COUNT)) {
					index++;
				} else {
					mode = BL_EXTEND_MODE_REPEAT;
				}
				BLMatrix2D m;
				blMatrix2DSetIdentity(&m);
				blPatternInitAs(&pattern, &img_pattern, NULL, mode , NULL);
				blContextSetFillStyleObject(&ctx, &pattern);
				has_stroke = TRUE;
				blPatternReset(&pattern);
			} else goto error;
			break;


		case W_B2D_CMD_LINE:
			if (has_stroke) {
				RESOLVE_ARG(0)
				if (RXT_PAIR == type) {
					blPathMoveTo(&path, (double)arg[0].pair.x, (double)arg[0].pair.y);
					type = RL_GET_VALUE(cmds, index, &arg[0]);
					while (RXT_PAIR == type) {
						blPathLineTo(&path, (double)arg[0].pair.x, (double)arg[0].pair.y);
						type = RL_GET_VALUE(cmds, ++index, &arg[0]);
					}
				}
				else if (RXT_VECTOR == type) {
					REBSER *ser_points = arg[0].series;
					if (VTSF64 != VECT_TYPE(ser_points)) goto error;
					REBCNT cnt_points = SERIES_LEN(ser_points)-1;
					REBDEC *points = (REBDEC*)SERIES_DATA(ser_points);
					type = RL_GET_VALUE(cmds, index++, &arg[0]);
					if (RXT_VECTOR != type) goto error;
					REBSER *ser_edges = arg[0].series;
					if (VTSI32 != VECT_TYPE(ser_edges)) goto error;
					REBINT *edges = (REBINT *)SERIES_DATA(ser_edges);
					REBCNT prev = 0;
					for (i = 0; i < SERIES_LEN(ser_edges) - 1; i+=2) {
						REBCNT p1 = (REBCNT)edges[i] << 1;
						REBCNT p2 = (REBCNT)edges[i+1] << 1;
						//debug_print("p1: %i p2: %i  %i\n", p1, p2, cnt_points );
						if (p1 >= 0 && p1 < cnt_points && p2 >= 0 && p2 < cnt_points) {
							if (i == 0 || prev != p1) {
							//	debug_print("%i M %lfx%lf %lfx%lf\n", i, points[p1], points[p1+1], points[p2], points[p2+1]);
								blPathMoveTo(&path, points[p1], points[p1+1]);
								blPathLineTo(&path, points[p2], points[p2+1]);
							}
							else {
							//	debug_print("%i L %lfx%lf %lfx%lf\n", i, points[p1], points[p1+1], points[p2], points[p2+1]);
								blPathLineTo(&path, points[p1], points[p1+1]);
								blPathLineTo(&path, points[p2], points[p2+1]);
							}
							prev = p2;
						}
					}
				} else goto error;
				blContextStrokeGeometry(&ctx, BL_GEOMETRY_TYPE_PATH , &path);
				blPathReset(&path);
			}
			break;

		case W_B2D_CMD_LINE_WIDTH:

			RESOLVE_NUMBER_ARG(0, 0)
			width = doubles[0];

			if (width == 0) {
				has_stroke = FALSE;
			}
			else {
				has_stroke = TRUE;
				blContextSetStrokeWidth(&ctx, width);
			}
			break;

		case W_B2D_CMD_LINE_JOIN:
			if (fetch_mode(cmds, index, &mode, W_B2D_ARG_MITER, 3)) {
				index++;
				switch (mode) {
				case  0: // mitter, check for bevel or round variant
					if (fetch_mode(cmds, index, &mode, W_B2D_ARG_BEVEL, 2)) {
						index++;
						type = mode + 1; //BL_STROKE_JOIN_MITER_BEVEL or BL_STROKE_JOIN_MITER_ROUND
					}
					else {
						type = BL_STROKE_JOIN_MITER_CLIP;
					}
					break;
				case  1: type = BL_STROKE_JOIN_BEVEL; break;
				default: type = BL_STROKE_JOIN_ROUND;
				}
			}
			else goto error;
			//debug_print("StrokeJoin: %u\n", type);
			blContextSetStrokeJoin(&ctx, type);
			break;

		case W_B2D_CMD_LINE_CAP:

			//TODO: use words instead of integers?
			RESOLVE_INT_ARG(0);
			cap = arg[0].int64;
			if (cap >= BL_STROKE_CAP_COUNT) goto error; //invalid cap value
			
			RESOLVE_INT_ARG_OPTIONAL(1); // end cap
			if (RXT_INTEGER == type) {
				blContextSetStrokeCap(&ctx, BL_STROKE_CAP_POSITION_START, cap);
				cap = arg[1].int64;
				if (cap >= BL_STROKE_CAP_COUNT) {
					//invalid cap value
				}
				blContextSetStrokeCap(&ctx, BL_STROKE_CAP_POSITION_END, cap);
			}
			else {
				blContextSetStrokeCaps(&ctx, cap);
			}
			break;

		case W_B2D_CMD_CUBIC:

			type = RL_GET_VALUE(cmds, index++, &arg[0]);
			if (RXT_PAIR == type) {
				blPathMoveTo(&path, (double)arg[0].pair.x, (double)arg[0].pair.y);
			} else goto error;
			while (FETCH_3_PAIRS(cmds, index, arg[1], arg[2], arg[3])) {
				r = blPathCubicTo(&path,
					(double)arg[1].pair.x, (double)arg[1].pair.y,
					(double)arg[2].pair.x, (double)arg[2].pair.y,
					(double)arg[3].pair.x, (double)arg[3].pair.y
				);
				index += 3;
			}
			if (has_fill) blContextFillGeometry(&ctx, BL_GEOMETRY_TYPE_PATH , &path);
			if (has_stroke) blContextStrokeGeometry(&ctx, BL_GEOMETRY_TYPE_PATH , &path);
			
			blPathReset(&path);
			break;


		case W_B2D_CMD_POLYGON:
			
			RESOLVE_PAIR_ARG(0, 0)
			blPathMoveTo(&path, doubles[0], doubles[1]);
			count = 0; i = 0;
			while (RXT_PAIR == RL_GET_VALUE(cmds, index, &arg[0])) {
				index++;
				count++;
				doubles[i++] = arg[0].pair.x;
				doubles[i++] = arg[0].pair.y;
				if(i >= DOUBLE_BUFFER_SIZE) {
					// we could extend the buffer here, or just continue processing in batches and save little memory.
					blPathPolyTo(&path, (BLPoint*)doubles, count);
					count = 0; i = 0;
				}
			}
			if (count > 0) blPathPolyTo(&path, (BLPoint*)doubles, count);
			blPathClose(&path);
			
			DRAW_GEOMETRY(ctx, BL_GEOMETRY_TYPE_PATH, &path)
			blPathReset(&path);
			break;


		case W_B2D_CMD_BOX:

			RESOLVE_PAIR_ARG(0, 0)
			RESOLVE_PAIR_ARG(1, 2) // bottom-right
			type = RL_GET_VALUE_RESOLVED(cmds, index, &arg[2]);
			if (type == RXT_DECIMAL || type == RXT_INTEGER) {
				index++;
				mode = BL_GEOMETRY_TYPE_ROUND_RECT;
				doubles[4] = (type == RXT_DECIMAL) ? arg[2].dec64 : (double)arg[2].int64;
				type = RL_GET_VALUE_RESOLVED(cmds, index, &arg[3]);
				if (type == RXT_DECIMAL || type == RXT_INTEGER) {
					index++;
					doubles[5] = (type == RXT_DECIMAL) ? arg[3].dec64 : (double)arg[3].int64;
				}
				else {
					doubles[5] = doubles[4];
				}
			} else {
				mode = BL_GEOMETRY_TYPE_RECTD;
			}
			//debug_print("box type: %u size: %f %f %f %f radius: %f %f\n", mode, doubles[0],doubles[1],doubles[2],doubles[3], doubles[4], doubles[5]);
			DRAW_GEOMETRY(ctx, mode, doubles);
			break;


		case W_B2D_CMD_CIRCLE:

			RESOLVE_PAIR_ARG(0, 0)   // center
			RESOLVE_NUMBER_ARG(1, 2) // radius or radius-x
			RESOLVE_NUMBER_ARG_OPTIONAL(2, 3)  // radius-y
			if (type) {
				DRAW_GEOMETRY(ctx, BL_GEOMETRY_TYPE_ELLIPSE, doubles);
			}
			else {
				DRAW_GEOMETRY(ctx, BL_GEOMETRY_TYPE_CIRCLE, doubles);
			}
			break;


		case W_B2D_CMD_ELLIPSE:

			RESOLVE_PAIR_ARG(0, 0) // top-left
			RESOLVE_PAIR_ARG(1, 2) // size
			doubles[2] *= 0.5;
			doubles[3] *= 0.5;
			doubles[0] += doubles[2];
			doubles[1] += doubles[3];
			DRAW_GEOMETRY(ctx, BL_GEOMETRY_TYPE_ELLIPSE, doubles);
			break;

		
		case W_B2D_CMD_ARC:

			RESOLVE_PAIR_ARG(0, 0)    // center
			RESOLVE_PAIR_ARG(1, 2)    // radius
			RESOLVE_NUMBER_ARG(2, 4); // begin
			RESOLVE_NUMBER_ARG(3, 5); // sweep

			doubles[4] *= pi1 / 180.0; // to radians
			doubles[5] *= pi1 / 180.0; // to radians

			// arc, pie or chord?
			type = RL_GET_VALUE(cmds, index, &arg);
			if (fetch_word(cmds, index, b2d_arg_words, &cmd) && cmd >= W_B2D_ARG_PIE && cmd <= W_B2D_ARG_CHORD) {
				index++;
				type = (cmd == W_B2D_ARG_CHORD) ? BL_GEOMETRY_TYPE_CHORD : BL_GEOMETRY_TYPE_PIE;
			}
			else {
				type = BL_GEOMETRY_TYPE_ARC;
			}
			DRAW_GEOMETRY(ctx, type, doubles);

			break;


		case W_B2D_CMD_IMAGE:

			blImageInit(&img);
			type = RL_GET_VALUE_RESOLVED(cmds, index++, &arg[0]);
			if (type == RXT_IMAGE) {
				r = blImageCreateFromData(&img, arg[0].width, arg[0].height, BL_FORMAT_PRGB32, ((REBSER*)arg[0].series)->data, (intptr_t)arg[0].width * 4, NULL, NULL);
				if (r != BL_SUCCESS) {
					trace("failed to init image!");
					goto error;
				}
			}
			else if (type == RXT_FILE) {
				BLArrayCore codecs;
				blImageCodecArrayInitBuiltInCodecs(&codecs);
				r = blImageReadFromFile(&img, ((REBSER*)arg[0].series)->data, &codecs);
				if (r != BL_SUCCESS) {
					trace("failed to load image!");
					goto end_ctx; //error!
				}
			}
			else goto error;

			// image area (could be used for texture atlas)
			rectI.x = 0;
			rectI.y = 0;
			rectI.w = arg[0].width;
			rectI.h = arg[0].height;
			
			
			RESOLVE_PAIR_ARG(1, 0) // top-left
			pt.x = doubles[0]; // x
			pt.y = doubles[1]; // y

			// bottom-right (optional):
			REBOOL scaleImage = FALSE;
			
			RESOLVE_PAIR_ARG_OPTIONAL(2, 0)
			if (type == RXT_PAIR) {
				scaleImage = TRUE;
				rect.x = pt.x;
				rect.y = pt.y;
				rect.w = doubles[0];
				rect.h = doubles[1];
				index++;
			}
			//debug_print("blitImage size: %i %i at: %f %f\n",  rectI.w, rectI.h, pt.x, pt.y);
			if (scaleImage) {
				blContextBlitScaledImageD(&ctx, &rect, &img, &rectI);
			}
			else {
				blContextBlitImageD(&ctx, &pt, &img, &rectI);
			}
			blImageReset(&img);
			break;

		case W_B2D_CMD_FONT:

			type = RL_GET_VALUE_RESOLVED(cmds, index++, &arg[0]);
			if (type == RXT_FILE || type == RXT_STRING) {
				REBSER *src = arg[0].series;
				REBSER *file = RL_ENCODE_UTF8_STRING(SERIES_DATA(src), SERIES_TAIL(src), SERIES_WIDE(src)>1, FALSE);
				r = blFontFaceCreateFromFile(&font_face, SERIES_DATA(file), BL_FILE_READ_MMAP_ENABLED | BL_FILE_READ_MMAP_AVOID_SMALL);
				debug_print("r: %i\n", r);
			//	if (BL_SUCCESS != r ) {
			//		debug_print("Failed to load font! (%s) %i\n", SERIES_DATA(file), r);
			//		goto error;
			//	}
			}
			BLFontDesignMetrics dm;
			BLFontMetrics fm;


			blFontReset(&font);
			blFontCreateFromFace(&font, &font_face, font_size);

			blFontGetDesignMetrics(&font, &dm);
			blFontGetMetrics(&font, &fm);

			debug_print("fm.size: %f\n", fm.size);
			debug_print("fm.lineGap: %f\n", fm.lineGap);
			debug_print("fm.xHeight: %f\n", fm.xHeight);
			debug_print("fm.capHeight: %f\n", fm.capHeight);
			debug_print("fm.underlinePosition: %f\n", fm.underlinePosition);
			debug_print("fm.underlineThickness: %f\n", fm.underlineThickness);
			debug_print("fm.strikethroughPosition: %f\n", fm.strikethroughPosition);
			debug_print("fm.strikethroughThickness: %f\n", fm.strikethroughThickness);

			debug_print("unitsPerEm: %i\n", dm.unitsPerEm);
			debug_print("ascent: %i\n", dm.ascent);
			debug_print("descent: %i\n", dm.descent);
			
			break;


		case W_B2D_CMD_TEXT:

			RESOLVE_PAIR_ARG(0, 0) // position
			pt.x = doubles[0];
			pt.y = doubles[1];
			// size (optional):
			sz = 0.0;
			RESOLVE_NUMBER_ARG_OPTIONAL(1, 3);
			if (type) {
				sz = doubles[3];
#ifdef TO_WINDOWS
				sz = (sz * 96.0) / 72.0;
#endif
			}
			if (sz != font_size && sz > 0.0) {
				font_size = sz;
				blFontReset(&font);
				blFontCreateFromFace(&font, &font_face, font_size);
				debug_print("font_size: %f\n", font_size);
				BLFontMatrix fmm;
				blFontGetMatrix(&font, &fmm);
				debug_print("fmm: %f %f %f %f\n", fmm.m00, fmm.m01, fmm.m10, fmm.m11);
			}

			RESOLVE_STRING_ARG(3) // text:

			BLGlyphBufferCore gb;
			blGlyphBufferInit(&gb);
			REBSER *str = (REBSER*)arg[3].series;
			debug_print("txt: %s\n", SERIES_DATA(str));
			blGlyphBufferSetText(&gb, SERIES_DATA(str), SERIES_LEN(str), SERIES_WIDE(str)==1?BL_TEXT_ENCODING_LATIN1:BL_TEXT_ENCODING_UTF16);
			BLTextMetrics tm;
			blFontGetTextMetrics(&font, &gb, &tm);
			debug_print("siz: %f %f %f %f\n", tm.boundingBox.x0, tm.boundingBox.y0, tm.boundingBox.x1, tm.boundingBox.y1);

			blContextFillTextD(&ctx, &pt, &font, SERIES_DATA(str), SERIES_TAIL(str), SERIES_WIDE(str)==1?BL_TEXT_ENCODING_LATIN1:BL_TEXT_ENCODING_UTF16);
			blGlyphBufferReset(&gb);

			break;

		case W_B2D_CMD_SCALE:
			RESOLVE_NUMBER_OR_PAIR_ARG(0, 0);
			blContextMatrixOp(&ctx, BL_MATRIX2D_OP_POST_SCALE, doubles);
			break;

		case W_B2D_CMD_ROTATE:
			RESOLVE_NUMBER_ARG(0, 0);
			doubles[0] *= pi1 / 180.0; // to radians
			RESOLVE_PAIR_ARG_OPTIONAL(1, 1);
			blContextMatrixOp(&ctx, type ? BL_MATRIX2D_OP_POST_ROTATE_PT : BL_MATRIX2D_OP_POST_ROTATE, doubles);
			break;

		case W_B2D_CMD_TRANSLATE:
			RESOLVE_PAIR_ARG(0, 0);
			blContextMatrixOp(&ctx, BL_MATRIX2D_OP_POST_TRANSLATE, doubles);
			break;

		case W_B2D_CMD_RESET_MATRIX:
			blContextMatrixOp(&ctx, BL_MATRIX2D_OP_RESET, NULL);
			break;

		case W_B2D_CMD_ALPHA:
			RESOLVE_NUMBER_ARG(0, 0);
			blContextSetGlobalAlpha(&ctx, doubles[0]);
			break;

		case W_B2D_CMD_BLEND:
		case W_B2D_CMD_COMPOSITE:
			type = RL_GET_VALUE_RESOLVED(cmds, index++, &arg[0]);
			if (fetch_mode(cmds, index - 1, &mode, W_B2D_ARG_SOURCE_OVER, BL_COMP_OP_COUNT)) {
				debug_print("mode: %i\n", mode);
				blContextSetCompOp(&ctx, mode);
			} else if (RXT_NONE == type || (RXT_LOGIC == type && !arg[0].int32a)) { // blend none or blend off
				blContextSetCompOp(&ctx, BL_COMP_OP_SRC_OVER);
			} else goto error;
			break;

		case W_B2D_CMD_FILL_ALL:
			blContextFillAll(&ctx);
			break;

		case W_B2D_CMD_CLEAR_ALL:
			blContextClearAll(&ctx);
			break;

		case W_B2D_CMD_CLIP:
			type = RL_GET_VALUE_RESOLVED(cmds, index, &arg[0]);
			if (RXT_NONE == type || (RXT_LOGIC == type && !arg[0].int32a)) {
				puts("clipoff");
				blContextRestoreClipping(&ctx);
			}
			else {
				RESOLVE_PAIR_ARG(0, 0);
				RESOLVE_PAIR_ARG(1, 2);
				rect.x = doubles[0];
				rect.y = doubles[1];
				rect.w = doubles[2] - doubles[0];
				rect.h = doubles[3] - doubles[1];
				blContextClipToRectD(&ctx, &rect);
			}
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

	goto end_ctx; // skip error setup
//end_error:
//	debug_print("fatal error...");
//	//TODO: provide some useful error info!
//	err = 1;

end_ctx:
	// END ...
	//trace("Cleaning...");
	blContextEnd(&ctx);
	blImageReset(&img_target);
	//blImageReset(&img_pattern);
	//blPatternReset(&pattern);
	blFontReset(&font);
	blFontFaceReset(&font_face);
	blContextReset(&ctx);
	return err;
}

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



void b2d_info(void) {
	// print library info...
	BLRuntimeBuildInfo buildInfo;
	blRuntimeQueryInfo(BL_RUNTIME_INFO_TYPE_BUILD, &buildInfo);

	printf(
		"Blend2D information:\n"
		"  Version      : %u.%u.%u\n"
		"  Build type   : %s\n"
		"  Compiled by  : %s\n",
		buildInfo.majorVersion,
		buildInfo.minorVersion,
		buildInfo.patchVersion,
		buildInfo.buildType == BL_RUNTIME_BUILD_TYPE_DEBUG ? "Debug" : "Release",
		buildInfo.compilerInfo);

//	BLRuntimeCpuInfo cpuInfo;
//	blRuntimeQueryInfo(BL_RUNTIME_INFO_TYPE_CPU, &cpuInfo);
//	debug_print(
//		"CPU information:\n"
//		"  arch         : %u\n"
//		"  features     : %u\n"
//		"  threadCount  : %u\n",
//		cpuInfo.arch,
//		cpuInfo.features,
//		cpuInfo.threadCount);
//
//	BLRuntimeMemoryInfo memoryInfo;
//	blRuntimeQueryInfo(BL_RUNTIME_INFO_TYPE_MEMORY, &memoryInfo);
//	debug_print(
//		"Blend2D memory:\n"
//		"  vmUsed      : %zu\n"
//		"  vmReserved  : %zu\n"
//		"  vmOverhead  : %zu\n"
//		"  vmBlockCount: %zu\n"
//		"  zmUsed      : %zu\n"
//		"  zmReserved  : %zu\n"
//		"  zmOverhead  : %zu\n"
//		"  zmBlockCount: %zu\n",
//		memoryInfo.vmUsed,
//		memoryInfo.vmReserved,
//		memoryInfo.vmOverhead,
//		memoryInfo.vmBlockCount,
//		memoryInfo.zmUsed,
//		memoryInfo.zmReserved,
//		memoryInfo.zmOverhead,
//		memoryInfo.zmBlockCount);
}