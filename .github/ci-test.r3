Rebol [
    title: "Rebol/Blend2D extension CI test"
]

;; for the CI test the module is in current directory 
system/options/modules: what-dir
system/modules/blend2d: none
b2d: import 'blend2d

? b2d

print b2d/info