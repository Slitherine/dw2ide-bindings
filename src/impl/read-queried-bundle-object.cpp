#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;


void ReadQueriedBundleObjectImpl(const FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG ReadQueriedBundleObjectImpl entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    //std::wcerr << L"DEBUG ReadQueriedBundleObject argCount: " << argCount << std::endl;
    if (argCount != 1) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    //std::wcerr << L"DEBUG ReadQueriedBundleObject checking arg0" << std::endl;
    if (!arg0->IsExternal()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be a handle", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    //std::wcerr << L"DEBUG ReadQueriedBundleObject extracting handle" << std::endl;
    auto bundleQueryHandle = reinterpret_cast<intptr_t>(arg0.As<v8::External>()->Value());
    //std::wcerr << L"DEBUG ReadQueriedBundleObject handle: 0x" << std::hex << bundleQueryHandle << std::dec
    //        << std::endl;
    wchar_t *buffer = nullptr;
    size_t size = 0;
    //std::wcerr << L"DEBUG ReadQueriedBundleObject invoking CLR method to get buf size" << std::endl;
    auto needed = DistantWorlds::Ide::ReadQueriedBundleObject(bundleQueryHandle, buffer, size);
    if (needed == 0) {
        //std::wcerr << L"DEBUG ReadQueriedBundleObject exited with undef" << std::endl;
        return;
    }
    size = 1 + -needed;
    //std::wcerr << L"DEBUG ReadQueriedBundleObject CLR returned " << needed << L", so size " << size << std::endl;
    buffer = (wchar_t *) _alloca(size * sizeof(wchar_t));
    auto wrote = DistantWorlds::Ide::ReadQueriedBundleObject(bundleQueryHandle, buffer, size);
    while (buffer[wrote - 1] == 0) wrote--;
    const v8::Local<v8::String> &str = v8::String::NewFromTwoByte(info.GetIsolate(),
            (const uint16_t *) buffer,
            v8::NewStringType::kNormal, wrote).ToLocalChecked();
    std::wstring_view svBuffer(buffer, wrote);
    //std::wcerr << L"DEBUG ReadQueriedBundleObject value: " << svBuffer << std::endl;
    info.GetReturnValue().Set(str);
    //std::wcerr << L"DEBUG ReadQueriedBundleObject exited with string" << std::endl;
}

