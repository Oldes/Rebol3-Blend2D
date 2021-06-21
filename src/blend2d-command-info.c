//
// Blend2D experimental Rebol extension
// ====================================
// Use on your own risc!

#include "blend2d-rebol-extension.h"
#include <inttypes.h>

const char* RuntimeCpuArchString(u32 val) {
	switch (val) {
	case BL_RUNTIME_CPU_ARCH_X86:  return "x86";
	case BL_RUNTIME_CPU_ARCH_ARM:  return "ARM";
	case BL_RUNTIME_CPU_ARCH_MIPS: return "MIPS";
	default:                       return "unknown";
	}
}

void b2d_info(RXIFRM* frm, void* reb_ctx) {
	// return library info...
	REBI64 tail = 0;
	REBSER* str = RL_MAKE_STRING(1000, FALSE); // 1024 bytes, latin1 (must be large enough!)

	if (RXT_HANDLE == RXA_TYPE(frm, 2)) {
		REBHOB* hob = RXA_HANDLE(frm, 2);

		if (hob->sym == Handle_BLFontFace) {
			BLFontFaceInfo fontFaceInfo;
			blFontFaceGetFaceInfo((BLFontFaceCore*)hob->data, &fontFaceInfo);
			tail = sprintf_s(
				SERIES_DATA(str),
				SERIES_REST(str),
				"faceType:    %u\n"
				"outlineType: %u\n"
				"glyphCount:  %u\n"
				"revision:    %u\n"
				"faceIndex:   %u\n"
				"faceFlags:   %u\n"
				"diagFlags:   %u\n",
				fontFaceInfo.faceType,
				fontFaceInfo.outlineType,
				fontFaceInfo.glyphCount,
				fontFaceInfo.revision,
				fontFaceInfo.faceIndex,
				fontFaceInfo.faceFlags,
				fontFaceInfo.diagFlags
			);
		}
		else if (hob->sym == Handle_BLPath) {
			BLPathCore* path = (BLPathCore*)hob->data;
			tail = sprintf_s(
				SERIES_DATA(str),
				SERIES_REST(str),
				"size:     %" PRIu64 "\n"
				"capacity: %" PRIu64 "\n",
				blPathGetSize(path),
				blPathGetCapacity(path)
			);
		}
		if (hob->sym == Handle_BLImage) {
			BLImageData imgData;
			blImageGetData((BLImageCore*)hob->data, &imgData);
			tail = sprintf_s(
				SERIES_DATA(str),
				SERIES_REST(str),
				"width:  %i\n"
				"heigth: %i\n"
				"stride: %" PRIu64 "\n"
				"format: %u\n"
				"flags:  %u\n",
				imgData.size.w,
				imgData.size.h,
				imgData.stride,
				imgData.format,
				imgData.flags
			);
		}
	}
	else {
		BLRuntimeBuildInfo buildInfo;
		BLRuntimeResourceInfo resourceInfo;
		BLRuntimeSystemInfo systemInfo;

		blRuntimeQueryInfo(BL_RUNTIME_INFO_TYPE_BUILD, &buildInfo);
		blRuntimeQueryInfo(BL_RUNTIME_INFO_TYPE_SYSTEM, &systemInfo);
		blRuntimeQueryInfo(BL_RUNTIME_INFO_TYPE_RESOURCE, &resourceInfo);

		tail = sprintf_s(
			SERIES_DATA(str),
			SERIES_REST(str),
			"Version:     %u.%u.%u\n"
			"Build-type:  %s\n"
			"Compiled-by: %s\n\n"

			"System: [\n"
			"  cpuArch:      %s\n" //! Host CPU architecture, see `BLRuntimeCpuArch`.
			"  cpuFeatures:  %u\n" //! Host CPU features, see `BLRuntimeCpuFeatures`.
			"  coreCount:    %u\n" //! Number of cores of the host CPU/CPUs.
			"  threadCount:  %u\n" //! Number of threads of the host CPU/CPUs.
			"  threadStackSize:       %u\n" //! Minimum stack size of a worker thread used by Blend2D.
			"  allocationGranularity: %u\n" //! Allocation granularity of virtual memory (includes thread's stack).
			"]\n\n"

			"Resource: [\n"
			"  vmUsed:       %zu\n"
			"  vmReserved:   %zu\n"
			"  vmOverhead:   %zu\n"
			"  vmBlockCount: %zu\n"
			"  zmUsed:       %zu\n"
			"  zmReserved:   %zu\n"
			"  zmOverhead:   %zu\n"
			"  zmBlockCount: %zu\n"
			"  dynamicPipelineCount: %zu\n"
			"]\n",

			buildInfo.majorVersion, buildInfo.minorVersion, buildInfo.patchVersion,
			buildInfo.buildType == BL_RUNTIME_BUILD_TYPE_DEBUG ? "Debug" : "Release",
			buildInfo.compilerInfo,

			RuntimeCpuArchString(systemInfo.cpuArch),
			systemInfo.cpuFeatures,
			systemInfo.coreCount,
			systemInfo.threadCount,
			systemInfo.threadStackSize,
			systemInfo.allocationGranularity,

			resourceInfo.vmUsed,
			resourceInfo.vmReserved,
			resourceInfo.vmOverhead,
			resourceInfo.vmBlockCount,
			resourceInfo.zmUsed,
			resourceInfo.zmReserved,
			resourceInfo.zmOverhead,
			resourceInfo.zmBlockCount,
			resourceInfo.dynamicPipelineCount
		);
	}
	if (tail < 0) {
		RXA_TYPE(frm, 1) = RXT_NONE;
	}
	else {
		SERIES_TAIL(str) = tail;
		RXA_SERIES(frm, 1) = str;
		RXA_TYPE(frm, 1) = RXT_STRING;
		RXA_INDEX(frm, 1) = 0;
	}
}