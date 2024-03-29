#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;


void IsImageImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG IsImageImpl entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    if (info.Length() != 1) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    if (!info[0]->IsExternal()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    //auto *isolate = info.GetIsolate();
    auto handle = reinterpret_cast<intptr_t>(info[0].As<v8::External>()->Value());
    bool success = DistantWorlds::Ide::IsImage(handle);
    info.GetReturnValue().Set(v8::Integer::New(isolate, success ? 1 : 0));
}