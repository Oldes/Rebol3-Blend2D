Rebol [
	title: "Basic Blend2D extension test"
]

CI?: "true" = get-env "CI"

; extension handling is still under development!
unless find system/modules 'blend2d [
	import rejoin [%../blend2d- system/build/os #"-" system/build/arch %.rebx]
]

unless function? :view [view: none] ;= for systems without view

b2d: system/modules/blend2d


;- internal built-in test                             
;view b2d/draw-test 256x256

;-----------------------------------------------------
print [
	as-red   "[Test 01]:"
	as-green "composition of multiple images with alpha channel"
]

plan: premultiply load %assets/Plan31.png

img: draw 680x380 [
	                    image :plan 00x100
	scale 0.8 rotate 10 image :plan 50x100
	scale 0.8 rotate 10 image :plan 50x100
	scale 0.8 rotate 10 image :plan 50x100
	scale 0.8 rotate 10 image :plan 50x100
]
save %test-result-01.png img
unless CI? [view img]

;-----------------------------------------------------
print [
	as-red   "[Test 02]:"
	as-green "fill, cubic, blend and box"
]

texture: load  %assets/texture.jpeg

img: draw 480x480 [
	;pen off
	fill linear 255.255.255 0 95.175.223 0.5 47.95.223 1 0x480 0x0
	fill-all ;55.0.0
	fill :texture flip
	cubic 26x31  642x132 587x-136 25x464  882x404 144x267 27x31
	blend color-burn
	fill linear 255.255.255 0 95.175.223 0.5 47.95.223 1 0x0 0x480
	box 40x40 200x400 45.5
	alpha 50%
	box 240x40 200x400 45.5
]
save %test-result-02.png img
unless CI? [view img]

;-----------------------------------------------------
print [
	as-red   "[Test 03]:"
	as-green "line-join and line-width"
]

img: draw 480x480 [
	line-width 20 pen gray
	line-join miter       line 140x20   40x80  140x80
	line-join round       line 300x20  200x80  300x80
	line-join bevel       line 460x20  360x80  460x80

	line-join miter       line 140x120  40x180 140x180
	line-join miter       line 300x140 200x180 300x180
	line-join miter       line 460x150 360x180 460x180

	line-join miter round line 140x220  40x280 140x280
	line-join miter round line 300x240 200x280 300x280
	line-join miter round line 460x250 360x280 460x280

	line-join miter bevel line 140x320  40x380 140x380
	line-join miter bevel line 300x340 200x380 300x380
	line-join miter bevel line 460x350 360x380 460x380
	

	line-join miter
	line-width 1 pen black
	line 140x20   40x80 140x80
	line 300x20  200x80 300x80
	line 460x20  360x80 460x80

	line 140x120  40x180 140x180
	line 300x140 200x180 300x180
	line 460x150 360x180 460x180

	line 140x220  40x280 140x280
	line 300x240 200x280 300x280
	line 460x250 360x280 460x280

	line 140x320  40x380 140x380
	line 300x340 200x380 300x380
	line 460x350 360x380 460x380
]
save %test-result-03.png img
unless CI? [view img]

;-----------------------------------------------------
print [
	as-red   "[Test 04]:"
	as-green "line-width"
]

w: 0.1 i: 1 code: {pen 0.0.0 line-width 10}
while[i < 480][
	append code rejoin ["^/line-width " w " line 10x" i " 470x" i]
	w: 1.1 * (w + 0.1)
	i: i + 1 + (1.1 * w)
]
img: draw 480x480 load code

save %test-result-04.png img
unless CI? [view img]


;-----------------------------------------------------
print [
	as-red   "[Test 05]:"
	as-green "fill, cubic, blend and box"
]

texture: load  %assets/texture.jpeg

img: draw 480x480 [
	fill 255.0.0
	circle 180x180 160
	fill linear 255.255.255 0 95.175.223 0.5 47.95.223 1 0x480 0x0
	;blend DIFFERENCE
	box 195x195 270x270 25
]

save %test-result-05.png img
unless CI? [view img]

print b2d/info

unless CI? [ask "done"]
unless CI? [wait 3]