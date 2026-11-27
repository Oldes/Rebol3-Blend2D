[![rebol-blend2d](https://github.com/user-attachments/assets/7ad6b294-8bcb-4e2e-ab45-d34003c4cff0)](https://github.com/Siskin-framework/Rebol-Blend2D)

[![Rebol-Blend2D CI](https://github.com/Siskin-framework/Rebol-Blend2D/actions/workflows/main.yml/badge.svg)](https://github.com/Siskin-framework/Rebol-Blend2D/actions/workflows/main.yml)
[![Gitter](https://badges.gitter.im/rebol3/community.svg)](https://app.gitter.im/#/room/#Rebol3:gitter.im)
[![Zulip](https://img.shields.io/badge/zulip-join_chat-brightgreen.svg)](https://rebol.zulipchat.com/)

# Rebol/Blend2D

[Blend2D](https://github.com/blend2d/blend2d) extension for [Rebol3](https://github.com/Oldes/Rebol3) (drawing dialect)

## Usage

This extension requires Oldes' version of *Rebol* language interpreter (at least `3.22.5`),
which can be downloaded [here](https://github.com/Oldes/Rebol3/releases).
To use Blend2D's `draw` dialect, the extension must be loaded using:
```rebol
import 'blend2d
```
Once the module is imported, the new `draw` function may be used to draw into any image.
```rebol
>> help draw
USAGE:
     DRAW image commands

DESCRIPTION:
     Draws scalable vector graphics to an image.
     DRAW is a command! value.

ARGUMENTS:
     image         [image! pair!]
     commands      [block!]
```

The dialect is similar but not exactly same like the [original Rebol2 implementation](http://www.rebol.com/r3/docs/view/draw.html) or [Red language draw](https://github.com/red/docs/blob/master/en/draw.adoc).
Not all commands are implemented... it was more considered as a proof of concept.

**For some code examples, visit [test/README.md](test/README.md).**

## Image codecs

Blend2D carries its own PNG, JPEG, BMP and QOI codecs. Importing the module registers them
in `system/codecs`, so they are used by `LOAD`, `SAVE`, `DECODE`, `ENCODE` and `ENCODING?`
like any internal codec:

```rebol
import 'blend2d

save %photo.qoi img
img: load %photo.png
encoding? bin                  ;; == png
```

A format which `system/codecs` already has is left alone - importing the extension for `draw`
never replaces a codec the interpreter came with. A *Rebol/Core* build, which has no image
codecs of its own, gets all four.

To use Blend2D's codec instead of the interpreter's own, remove the existing one *before*
importing the module:

```rebol
unset in system/codecs 'png    ;; or: attempt [system/codecs/png: none]
import 'blend2d
```

Either form leaves the word in `system/codecs`, which is why the registration asks whether the
slot still holds a codec object rather than whether the word is there.

The codecs are reachable directly as well:

```rebol
b2d: import 'blend2d

b2d/codecs                     ;; == [png jpeg bmp qoi]
bin: b2d/encode 'png img
img: b2d/decode bin            ;; the format is recognized from the data
```

Two things to know:

* **JPEG can only be read.** Blend2D has no JPEG encoder yet, so `encode 'jpeg` reports an error.
* **Partial alpha does not survive a round trip exactly.** Blend2D's encoders take premultiplied
  pixels while a Rebol `image!` keeps straight alpha. A fully opaque image is handed over as it
  is, so nothing is lost (and the file is smaller); an image with translucent pixels is
  premultiplied on the way in and undone on the way out, which rounds twice.

The module is loaded on demand, so the codecs appear once something imports it - on a host
without its own PNG codec, `load %photo.png` needs `import 'blend2d` first.

## Building

The extension is built with the [Siskin builder](https://github.com/Siskin-framework/Builder/) using the
included `RebolBlend2D.nest` project file. The C header (`src/gen-blend2d.h`) and the command table
(`src/gen-blend2d.c`) are generated from the declarative specification in `src/blend2d.reb` by
`make-extension.r3`, which also refreshes the reference sections of this file. Do not modify the
generated sources or the reference below - modify the specification instead.

## Testing

`ci-test.r3` exercises every command, the draw dialect and the handle accessors, and exits
with a non-zero code when anything fails, so it can gate CI:

```
r3 ci-test.r3            ;; full run
r3 ci-test.r3 --quick    ;; skips the recycle loop
```

It imports the module by name, so point it at a freshly built library with
`REBOL_MODULES_DIR` (or run it from the directory holding the built `.rebx` under CI).

## Extension commands:


#### `draw` `:image` `:commands`
Draws scalable vector graphics to an image
* `image` `[image! pair!]` Target image, or a size of a new one
* `commands` `[block!]` The draw dialect

#### `path` `:commands`
Prepares path object
* `commands` `[block!]` Path dialect: move, line, curve, curv, qcurve, qcurv, arc, hline, vline, close

#### `font` `:file`
Prepares font handle
* `file` `[file! string!]` Font location or name

#### `image` `:from`
Prepares Blend2D's native image
* `from` `[pair! image! file!]` Size of a new image, an image to wrap, or a file to load

#### `info`
Returns info about Blend2D library
* `/of`
* `handle` `[handle!]` Blend2D object

#### `set-threads` `:count`
Sets number of threads to be used
* `count` `[integer!]` 0 means synchronous rendering; 1 is for async on main thread only else thread pool is used

#### `codec-handle` `:format`
Returns a codec dispatcher as a `codec` handle, for use as `entry`
* `format` `[word!]` png, jpeg, bmp or qoi

#### `encode` `:format` `:image`
Encodes an image into a binary
* `format` `[word!]` png, bmp or qoi (Blend2D has no JPEG encoder)
* `image` `[image! handle!]` A Rebol image or a Blend2D image handle

#### `decode` `:data`
Decodes an image from a binary; the format is recognized from the data
* `data` `[binary!]`

#### `codecs`
Returns the words of the image formats this build can handle


## Used handles and its getters / setters

#### __IMAGE__ - Blend2D image (BLImage)

```rebol
;Refinement       Gets                Sets                          Description
/size             pair!               none                          "Image size in pixels"
/width            integer!            none                          "Image width in pixels"
/height           integer!            none                          "Image height in pixels"
/format           integer!            none                          "Pixel format (BLFormat value; 1 is PRGB32)"
/stride           integer!            none                          "Number of bytes between two consecutive rows"
```

#### __PATH__ - Blend2D path (BLPath)

```rebol
;Refinement       Gets                Sets                          Description
/size             integer!            none                          "Number of vertices"
/capacity         integer!            none                          "Number of vertices which fit without reallocating"
```

#### __FONT-FACE__ - Blend2D font face (BLFontFace)

```rebol
;Refinement       Gets                Sets                          Description
/glyphs           integer!            none                          "Number of glyphs the face provides"
/face-type        integer!            none                          "Face type (BLFontFaceType value)"
/outline-type     integer!            none                          "Outline type (BLFontOutlineType value)"
/revision         integer!            none                          "Face revision"
/face-index       integer!            none                          "Index of the face in a font collection"
/face-flags       integer!            none                          "Face flags (BLFontFaceFlags bit set)"
/diag-flags       integer!            none                          "Diagnostic flags (BLFontFaceDiagFlags bit set)"
```


## Other extension values:
```rebol
foreach [name title suffixes] [
	png  "Portable Network Graphics"        [%.png]
	jpeg "Joint Photographic Experts Group" [%.jpg %.jpeg %.jfif]
	bmp  "Device independent bitmap"        [%.bmp]
	qoi  "Quite OK Image"                   [%.qoi]
][
	unless object? select system/codecs name [
		register-codec compose/only [
			name:     (to lit-word! name)
			type:     'image
			title:    (title)
			suffixes: (suffixes)
			entry:    (codec-handle name)
		]
	]
]
protect/hide 'codec-handle
```
