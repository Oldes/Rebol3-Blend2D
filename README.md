# Rebol/Blend2D

[Blend2D](https://github.com/blend2d/blend2d) extension for [Rebol3](https://github.com/Siskin-framework/Rebol) (drawing dialect)

## Usage

This extension requires Oldes' version of *Rebol* language interpreter, which can be downloaded [here](https://github.com/Siskin-framework/Rebol/releases).
To use Bland2D's `draw` dialect, the extension must be loaded using:
```rebol
import %blend2d-x64.rebx
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
