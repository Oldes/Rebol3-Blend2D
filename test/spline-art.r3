Rebol [
	Title:   "Rebol/Blend2D spline showcase"
	Needs:   3.22.5
	Author:  @Oldes
	License: Apache-2.0
	Purpose: {
		Draws a picture which uses the `spline` command in each of the ways
		the dialect offers it:

		* closed splines, rotated around a centre into a rosette,
		* open splines stroked as ribbons, with the knots given as a block,
		* and a panel along the bottom which puts the curve next to the
		  chords of the knots it interpolates, so that the one property a
		  spline has to have - passing through its knots - can be seen.

		Usage:
			r3 spline-art.r3
	}
]

;; Before Rebol will be able to handle it automatically,
;; manually remove possibly embedded older Blend2D module.
try [system/modules/blend2d: none]
b2d: import/version 'blend2d 0.21.3

size:   1000x700
centre: 500x290

;; ----------------------------------------------------------------------------
;; A ribbon of `n` knots sweeping across (and a little past) the canvas.
ribbon: func [
	"Returns a block of knots along a sine wave"
	base  [ number!] "Vertical centre of the wave"
	amp   [ number!] "Amplitude"
	phase [ number!]
	n     [integer!] "Number of knots"
	/local out x y
][
	out: make block! n
	;; the steps are counted once; the position and the angle are then just
	;; advanced by them
	x:  -40
	dx: 1080 / (n - 1)
	da: 2.6 * pi / (n - 1)
	loop n [
		append out as-pair x base + (amp * sin phase)
		x:     x + dx
		phase: phase + da
	]
	out
]

;; The knots of the panel. Kept as data so that the chords, the dots and the
;; spline are all drawn from the very same numbers.
knots: [70x638 213x582 356x658 499x590 642x642 785x586 928x630]

;; ----------------------------------------------------------------------------
;; Background, and the strip which the panel sits on.
cmds: copy [
	fill linear 20.26.51 0.0 6.8.15 1.0 0x0 0x700
	fill-all
	fill 255.255.255.11
	box 0x548 1000x152
]

;; A rosette of twelve closed splines. Each one is the same four knots; the
;; matrix is what turns them into petals, so the gradient turns with them.
;;
;; The spline is solved into a path once and only drawn twelve times - as a
;; `spline` command it would be parsed and solved again for every petal.
;; The gradient has to stay inside the repeated block: Blend2D freezes the
;; transformation of a style when the style is set, so a single `fill` in
;; front of the loop would leave all twelve petals with the same, unrotated
;; gradient.
petal-path: b2d/path [spline 500x290 546x194 500x60 454x194 closed]

petal: [
	fill linear 255.95.158.184 0.0 43.27.107.184 1.0 500x60 500x290
	pen 255.208.228.92
	line-width 1.2
	shape :petal-path
	rotate 30 500x290
]
append/dup cmds petal 12

append cmds [
	reset-matrix
	pen off
	fill 255.208.228.230
	circle 500x290 26
]

;; Three open splines. `spline` takes the knots as a block here, which is how
;; computed geometry is usually handed to the dialect.
foreach [color width wave] [
	127.227.255.204 6.0 [104 40 0.0 ]
	182.168.255.204 4.6 [280 52 1.25]
	255.210.127.204 3.2 [456 64 2.5 ]
][
	append cmds compose/only [
		fill off
		pen (color)
		line-width (width)
		line-cap 2
		spline (ribbon wave/1 wave/2 wave/3 8)
	]
]

;; The panel: the chords between the knots, the spline through them, and the
;; knots themselves on top.
append cmds [
	fill off
	pen 138.160.200.191
	line-width 1.4
	line :knots

	pen 255.255.255
	line-width 3
	line-cap 2
	spline :knots

	pen off
	fill 255.95.158
	point-size 11
	point :knots
]

;; ----------------------------------------------------------------------------
img:  draw size cmds
file: %spline-art.png

save file img
print ["Saved:" as-yellow to-local-file/full file "-" img/size]
