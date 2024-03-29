#pragma once
#include "pch.h"

using namespace dw2ide;
using namespace v8;

void ReleaseHandleImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG ReleaseHandleImpl entered" << std::endl;
#endif
    if (info.Length() != 1) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    if (!info[0]->IsExternal()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(info[0].As<v8::External>()->Value());
    //auto handle = dw2ide::ExtractHandle(info[0]);

    DistantWorlds::Ide::ReleaseHandle(handle);
}