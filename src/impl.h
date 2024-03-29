#pragma once
#include "pch.h"

#define DEFINE_IMPL(name) \
    void name(const v8::FunctionCallbackInfo<v8::Value> &info)

DEFINE_IMPL(DebugBreakImpl);
DEFINE_IMPL(ClrDebugBreakImpl);
DEFINE_IMPL(ReleaseHandleImpl);
DEFINE_IMPL(HandleToStringImpl);
DEFINE_IMPL(LoadBundleImpl);
DEFINE_IMPL(QueryBundleObjectsImpl);
DEFINE_IMPL(ReadQueriedBundleObjectImpl);
DEFINE_IMPL(TryGetObjectIdImpl);
DEFINE_IMPL(GetObjectOffsetImpl);
DEFINE_IMPL(GetObjectTypeImpl);
DEFINE_IMPL(GetObjectSizeImpl);
DEFINE_IMPL(GetObjectSimplifiedTypeImpl);
DEFINE_IMPL(InstantiateBundleItemImpl);
DEFINE_IMPL(IsImageImpl);
DEFINE_IMPL(TryConvertImageToBufferWebpImpl);
DEFINE_IMPL(TryConvertImageToStreamWebpImpl);
DEFINE_IMPL(GetImageMipLevelsImpl);
DEFINE_IMPL(GetImageDimensionsImpl);
DEFINE_IMPL(GetImageWidthImpl);
DEFINE_IMPL(GetImageHeightImpl);
DEFINE_IMPL(GetImageDepthImpl);
DEFINE_IMPL(ShowMessageBoxImpl);
DEFINE_IMPL(GetImageFormatImpl);
DEFINE_IMPL(GetImageTextureTypeImpl);
DEFINE_IMPL(TryExportObjectImpl);
DEFINE_IMPL(TryExportImageAsWebpImpl);
DEFINE_IMPL(TryExportImageAsDdsImpl);
DEFINE_IMPL(NewIsolationContextImpl);
DEFINE_IMPL(DeisolateImpl);
DEFINE_IMPL(GetIsolationContextIdImpl);
DEFINE_IMPL(GetLastExceptionImpl);

