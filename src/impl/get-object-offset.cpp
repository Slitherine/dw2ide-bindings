#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void GetObjectOffsetImpl(const FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG GetObjectOffsetImpl entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();

    int argCount = info.Length();
    if (argCount != 2) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 2 arguments", v8::NewStringType::kInternalized);
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
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be ObjectIdByteSize bytes in length", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto arg1 = info[1];
    if (!arg1->IsBoolean()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 2nd argument to be a boolean", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto wantsSourceDirOrBundlePath = arg1->BooleanValue(isolate);

    int64_t *pOffset = nullptr;
    int64_t *pOffsetEnd = nullptr;

    bool success;
    wchar_t *pSourceDirOrBundlePath = nullptr;
    if (wantsSourceDirOrBundlePath) {
        success = DistantWorlds::Ide::TryGetObjectOffset(
                reinterpret_cast<uint8_t *>(objectIdData->Data()),
                pOffset, pOffsetEnd,
                &pSourceDirOrBundlePath, isoCtxNum);

    } else {
        success = DistantWorlds::Ide::TryGetObjectOffset(
                reinterpret_cast<uint8_t *>(objectIdData->Data()),
                pOffset, pOffsetEnd, nullptr, isoCtxNum);
    }
    if (!success) {
        return;
    }

    // return an object: { offset, offsetEnd, [sourceDirOrBundlePath] }
    auto obj = v8::Object::New(isolate);
    obj->Set(context, v8::String::NewFromUtf8Literal(isolate, "offset", v8::NewStringType::kInternalized),
            v8::Number::New(isolate, *pOffset)).FromJust();
    obj->Set(context, v8::String::NewFromUtf8Literal(isolate, "offsetEnd", v8::NewStringType::kInternalized),
            v8::Number::New(isolate, *pOffsetEnd)).FromJust();


    if (pSourceDirOrBundlePath != nullptr) {
        // pSourceDirOrBundlePath is optional output, not input
        // it will be a null terminated UTF-16 string allocated on a native heap
        // not subject to garbage collection or memory motion
        // it is mapped by an interned pool, don't have to free it
        // once written, it will not be freed until the heap is destroyed

        const v8::Local<v8::String> &str = v8::String::NewFromTwoByte(info.GetIsolate(),
                (const uint16_t *) pSourceDirOrBundlePath,
                v8::NewStringType::kNormal).ToLocalChecked();
        obj->Set(context,
                v8::String::NewFromUtf8Literal(isolate, "sourceDirOrBundlePath", v8::NewStringType::kInternalized),
                str).FromJust();
    }

    info.GetReturnValue().Set(obj);
}

