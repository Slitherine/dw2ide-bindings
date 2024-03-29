#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void GetImageDimensionsImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG GetImageDimensionsImpl entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 1) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1 arguments", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    if (!arg0->IsExternal()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(arg0.As<v8::External>()->Value());
    auto dims = DistantWorlds::Ide::GetImageDimensions(handle);
    if (dims < 0) {
        std::wcerr << L"DEBUG GetImageDimensions exited with undef" << std::endl;
        return;
    }
    info.GetReturnValue().Set(v8::Integer::New(isolate, dims));
    std::wcerr << L"DEBUG GetImageDimensions exited with " << dims << std::endl;
}
