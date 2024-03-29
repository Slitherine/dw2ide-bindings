#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void GetImageWidthImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG GetImageWidthImpl entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 2) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 2 arguments", v8::NewStringType::kInternalized);
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
    auto arg1 = info[1];
    if (!arg1->IsInt32()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 2nd to be an integer", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(arg0.As<v8::External>()->Value());
    auto mipLevel = arg1->Int32Value(context).ToChecked();
    auto width = DistantWorlds::Ide::GetImageWidth(handle, mipLevel);
    if (width < 0) {
        //std::wcerr << L"DEBUG GetImageWidth exited with undef" << std::endl;
        return;
    }
    info.GetReturnValue().Set(v8::Integer::New(isolate, width));
    //std::wcerr << L"DEBUG GetImageWidth exited with " << width << std::endl;
}