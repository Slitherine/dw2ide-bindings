#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void HandleToStringImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG HandleToStringImpl entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    if (info.Length() != 1) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    //std::wcerr << L"DEBUG HandleToString getting arg0" << std::endl;
    if (!info[0]->IsExternal()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    //std::wcerr << L"DEBUG HandleToString extracting handle" << std::endl;
    auto handle = reinterpret_cast<intptr_t>(info[0].As<v8::External>()->Value());
    //auto handle = dw2ide::ExtractHandle(info[0]);
    wchar_t *buffer = nullptr;
    size_t size = 0;
    //std::wcerr << L"DEBUG HandleToString invoking CLR method to get buf size" << std::endl;
    auto needed = DistantWorlds::Ide::HandleToString(handle, buffer, size);
    if (needed == 0) {
        //std::wcerr << L"DEBUG HandleToString CLR returned 0" << std::endl;
        //std::wcerr << L"DEBUG HandleToString exited with undef" << std::endl;
        return;
    }
    size = 1 + -needed;
    //std::wcerr << L"DEBUG HandleToString CLR returned " << needed << L", so size " << size << std::endl;
    buffer = (wchar_t *) _alloca(size * sizeof(wchar_t));
    //std::wcerr << L"DEBUG HandleToString invoking CLR method to get string" << std::endl;
    auto wrote = DistantWorlds::Ide::HandleToString(handle, buffer, size);
    //std::wcerr << L"DEBUG HandleToString CLR returned" << std::endl;
    while (buffer[wrote - 1] == 0) wrote--;
    std::wstring_view svBuffer(buffer, wrote);
    //std::wcerr << L"DEBUG HandleToString value: " << svBuffer << std::endl;
    const v8::Local<v8::String> &str = v8::String::NewFromTwoByte(info.GetIsolate(),
            (const uint16_t *) buffer,
            v8::NewStringType::kNormal, wrote).ToLocalChecked();
    info.GetReturnValue().Set(str);
}
