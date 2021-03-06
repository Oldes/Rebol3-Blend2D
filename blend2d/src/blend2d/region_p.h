// Blend2D - 2D Vector Graphics Powered by a JIT Compiler
//
//  * Official Blend2D Home Page: https://blend2d.com
//  * Official Github Repository: https://github.com/blend2d/blend2d
//
// Copyright (c) 2017-2020 The Blend2D Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef BLEND2D_REGION_P_H_INCLUDED
#define BLEND2D_REGION_P_H_INCLUDED

#include "./api-internal_p.h"
#include "./array_p.h"
#include "./region.h"

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

// ============================================================================
// [BLRegion - Internal]
// ============================================================================

//! Internal implementation that extends `BLRegionImpl`.
struct BLInternalRegionImpl : public BLRegionImpl {
  // Internal members can be placed here in the future.
};

template<>
struct BLInternalCastImpl<BLRegionImpl> { typedef BLInternalRegionImpl Type; };

BL_HIDDEN BLResult blRegionImplDelete(BLRegionImpl* impl) noexcept;

//! \}
//! \endcond

#endif // BLEND2D_REGION_P_H_INCLUDED
