#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;


void GetImageFormatImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG GetImageFormatImpl entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    //auto context = isolate->GetCurrentContext();
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
    auto needed = DistantWorlds::Ide::GetImageFormat(handle, nullptr, 0);
    if (needed == 0) {
        //std::wcerr << L"DEBUG GetImageFormat exited with undef" << std::endl;
        return;
    }
    auto size = 1 + -needed;
    auto *pFormat = (wchar_t *) _alloca(size * sizeof(wchar_t));
    auto wrote = DistantWorlds::Ide::GetImageFormat(handle, pFormat, size);
    if (wrote < 0) DEBUG_BREAK();
    while (pFormat[wrote - 1] == 0) wrote--;
    const v8::Local<v8::String> &str = v8::String::NewFromTwoByte(info.GetIsolate(),
            (const uint16_t *) pFormat,
            v8::NewStringType::kNormal, wrote).ToLocalChecked();
    info.GetReturnValue().Set(str);
    //std::wcerr << L"DEBUG GetImageFormat exited with string: "
    //        << std::wstring_view(pFormat, wrote) << std::endl;
}
