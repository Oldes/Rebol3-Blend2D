//
// Blend2D experimental Rebol extension
// ====================================
// Use on your own risc!

#include <stdlib.h>
#include <math.h>
#include "blend2d.h"
#include "reb-host.h"
#include "host-lib.h"

//#define USE_TRACES
#ifdef  USE_TRACES
#include <stdio.h>
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str) 
#endif

#define DOUBLE_BUFFER_SIZE 16
#define ARG_BUFFER_SIZE    8



REBCNT b2d_draw( RXIFRM* frm, void* ctx );
REBCNT b2d_path( RXIFRM* frm, void* ctx );
REBCNT b2d_font( RXIFRM* frm, void* ctx);
REBCNT b2d_image(RXIFRM* frm, void* ctx );
void   b2d_info( RXIFRM* frm, void* ctx );
//REBCNT b2d_draw_test( RXIFRM *frm, void *ctx );

BLResult b2d_init_image_from_file(BLImageCore* image, REBSER* fileName);
BLResult b2d_init_image_from_arg(BLImageCore* image, RXIARG arg, REBCNT type);

REBOOL fetch_word(REBSER *cmds, REBCNT index, u32* words, REBCNT *cmd);
REBOOL fetch_mode(REBSER *cmds, REBCNT index, REBCNT *result, REBCNT start, REBCNT max);
REBOOL fetch_color(REBSER *cmds, REBCNT index, REBCNT *cmd);

void* releaseFontFace(void* font);
void* releasePath(void* path);
void* releaseImage(void* image);


extern u32* b2d_cmd_words;
extern u32* b2d_arg_words;
extern REBCNT Handle_BLPath;
extern REBCNT Handle_BLFontFace;
extern REBCNT Handle_BLImage;

extern REBDEC doubles[DOUBLE_BUFFER_SIZE];
extern RXIARG arg[ARG_BUFFER_SIZE];


#define PI 3.14159265358979323846264338327950288

#define TO_RADIANS(value) ((value) *= PI / 180.0)

#define TUPLE_TO_COLOR(t) (REBCNT)((t.bytes[0]==3?255:t.bytes[4])<<24 | (t.bytes[1])<<16 | (t.bytes[2])<<8 |  (t.bytes[3]))

#define FETCH_3_PAIRS(c,i, a1, a2, a3) (\
	   RXT_PAIR == RL_GET_VALUE(c, i,   &a1) \
	&& RXT_PAIR == RL_GET_VALUE(c, i+1, &a2) \
	&& RXT_PAIR == RL_GET_VALUE(c, i+2, &a3))

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

#define ARG_X(n) (arg[n].pair.x)
#define ARG_Y(n) (arg[n].pair.y)

#define OPT_WORD_FLAG(flag, name) if (fetch_word(cmds, index, b2d_arg_words, &cmd) && cmd == name) { flag = TRUE; index++; }