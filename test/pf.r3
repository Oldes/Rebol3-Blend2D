Rebol []

import 'blend2d

data: {
................
................
...####..#####..
...#...#.#......
...#####.###....
...#.....#......
...#.....#......
................
...#####.#####..
.......#.#......
...#####.#####..
...#.........#..
...#####.#####..
................
................}

hgrid: draw 5x10 [fill 0.0.0 box 0x4 5x4]
vgrid: draw 10x5 [fill 0.0.0 box 4x0 4x5]

pos: 0x0
blk: [
	fill :hgrid fill-all
	fill :vgrid
]
parse data [
	opt LF
	any [
		  LF   (pos/y: pos/y + 50 pos/x: 0)
		| #"." (pos/x: pos/x + 50)
		| #"#" (
			repend blk ['box pos 50x50]
			pos/x: pos/x + 50
		)
	]
]
view img: draw 17x16 * 50 blk
save %PF25.png img
