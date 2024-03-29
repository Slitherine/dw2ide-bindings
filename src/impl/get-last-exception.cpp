#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void GetLastExceptionImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG GetLastExceptionImpl entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    if (info.Length() != 0) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 0 arguments", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    //auto *isolate = info.GetIsolate();
    auto handle = DistantWorlds::Ide::GetLastException();
    info.GetReturnValue().Set(v8::External::New(isolate, reinterpret_cast<void *>(handle)));
}