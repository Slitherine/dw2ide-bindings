#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;


void TryConvertImageToBufferWebpImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG TryConvertImageToBufferWebpImpl entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    if (info.Length() != 3) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 3 arguments", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    if (!info[0]->IsExternal()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(info[0].As<v8::External>()->Value());
    //auto handle = dw2ide::ExtractHandle(info[0]);
    auto mipLevel = info[1]->Int32Value(context).ToChecked();
    auto arg2 = info[2];
    if (!arg2->IsArrayBufferView() && !arg2->IsArrayBuffer()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 3rd argument to be an ArrayBuffer or an ArrayBufferView", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto bufferData = arg2->IsArrayBufferView()
            ? arg2.As<v8::ArrayBufferView>()->Buffer()
            : arg2.As<v8::ArrayBuffer>();
    auto bufferLength = bufferData->ByteLength();
    bool success = DistantWorlds::Ide::TryConvertImageToBufferWebp(handle, mipLevel,
            reinterpret_cast<uint8_t *>(bufferData->Data()), bufferLength);
    info.GetReturnValue().Set(v8::Integer::New(isolate, success ? 1 : 0));
}
