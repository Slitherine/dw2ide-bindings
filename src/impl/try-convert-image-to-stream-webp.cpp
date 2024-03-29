#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void TryConvertImageToStreamWebpImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG TryConvertImageToStreamWebpImpl entered" << std::endl;
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
    auto mipLevel = info[1]->Int32Value(context).ToChecked();
    auto arg2 = info[2];
    if (!arg2->IsFunction()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 3rd argument to be a function", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto fn = arg2.As<v8::Function>();
    auto success = DistantWorlds::Ide::TryConvertImageToStreamWebp(handle, mipLevel,
            [](void *pFn, unsigned char *buffer, size_t length) {
                auto *isolate = v8::Isolate::GetCurrent();
                auto context = isolate->GetCurrentContext();
                auto bufferData = v8::ArrayBuffer::New(isolate, length);
                memcpy(bufferData->Data(), buffer, length);
                auto uint8Array = v8::Uint8Array::New(bufferData, 0, length);
                v8::Local<v8::Value> args[] = { uint8Array };
                // NOTE: not local scope here
                // assume only called within outer function scope
                auto fn = (v8::Function *) pFn;
                fn->Call(context, Null(isolate), 1, args).ToLocalChecked();
            }, *fn);
    info.GetReturnValue().Set(v8::Integer::New(isolate, success ? 1 : 0));
}
