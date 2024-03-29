#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void GetObjectSizeImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG GetObjectSizeImpl entered" << std::endl;
#endif

    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();

    int argCount = info.Length();
    if (argCount != 1) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto isoCtxNum = 0;
    auto thisArg = info.This();
    if (thisArg->IsObject()) {
        auto ctx = Dw2IdeContext::FromFunctionCallbackInfo(info);
        auto sym = ctx->IsolationContextIdSymbol.Get(isolate);
        auto ctxId = thisArg->Get(context,sym).ToLocalChecked();
        if (ctxId->IsInt32())
            isoCtxNum = ctxId->Int32Value(context).ToChecked();
    }

    auto arg0 = info[0];
    if (!arg0->IsArrayBufferView() && !arg0->IsArrayBuffer()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be an ArrayBuffer or an ArrayBufferView", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto objectIdData = arg0->IsArrayBufferView()
            ? arg0.As<v8::ArrayBufferView>()->Buffer()
            : arg0.As<v8::ArrayBuffer>();

    auto objectIdLength = objectIdData->ByteLength();

    if (objectIdLength != DistantWorlds::Ide::ObjectIdByteSize) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be ObjectIdByteSize bytes in length", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    int64_t *pSize = nullptr;

    bool success = DistantWorlds::Ide::TryGetObjectSize(
            reinterpret_cast<uint8_t *>(objectIdData->Data()),
            pSize, isoCtxNum);

    if (!success) {
        return;
    }

    int64_t sizeValue = *pSize;

    if (sizeValue < 0) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Object size is negative, this is probably an error in the bundle data"
                " or the data type has no valid size",
                v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    if (sizeValue > 9007199254740991 /*MAX_SAFE_INTEGER*/) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Object size is too large to be represented as a JavaScript integer,"
                " this is probably an error in the bundle data",
                v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    info.GetReturnValue().Set(v8::Number::New(isolate, (double) sizeValue));
}