//   ____  __   __        ______        __
//  / __ \/ /__/ /__ ___ /_  __/__ ____/ /
// / /_/ / / _  / -_|_-<_ / / / -_) __/ _ \
// \____/_/\_,_/\__/___(@)_/  \__/\__/_// /
//  ~~~ oldes.huhuman at gmail.com ~~~ /_/
//
// Project: Rebol/Blend2D extension
// SPDX-License-Identifier: Apache-2.0
// =============================================================================
// The spline solver, as seen by the C sources of the extension.
//
// The implementation is C++ (blend2d-spline.cpp wrapping Aurimas Gasiulis'
// S9 spline), so this is the only thing the dialect code includes.
//

#ifndef BLEND2D_SPLINE_H
#define BLEND2D_SPLINE_H

#include <stdint.h>
#include "blend2d/blend2d.h"

#ifdef __cplusplus
extern "C" {
#endif

// Appends an interpolating spline through `count` knots to `path`.
//
//   xy      - knot coordinates, two doubles per knot (x y x y ...)
//   count   - number of knots, not of doubles
//   closed  - non-zero appends the span back to the first knot and closes
//             the figure
//
// When the path already has a current position, that position becomes the
// spline's first knot, so a spline continues the figure like `line` or `arc`
// do; otherwise the first knot starts a new one.
//
// Consecutive knots which coincide are dropped (the solver requires distinct
// knots), as is a final knot repeating the first one of a closed spline.
// Two knots give a straight line and a single knot gives nothing.
//
// Returns the number of segments appended to the path, or 0 when the knots
// gave nothing to draw.
uint32_t b2d_spline_to_path(BLPathCore *path, const double *xy, uint32_t count, int closed);

#ifdef __cplusplus
}
#endif

#endif // BLEND2D_SPLINE_H
