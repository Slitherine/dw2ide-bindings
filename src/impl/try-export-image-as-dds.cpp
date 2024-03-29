#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;


void TryExportImageAsDdsImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG TryExportImageAsDdsImpl entered" << std::endl;
#endif
    // arg0 is handle, arg1 is path
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

    auto handle = reinterpret_cast<intptr_t>(arg0.As<v8::External>()->Value());
    auto arg1 = info[1];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg1->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG TryExportImageAsDds arg1 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 2nd argument to be a string", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto maybePath = arg1->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybePath, path);
    if (path.pBuffer == nullptr || path.nBuffer == 0) {
        //std::wcerr << L"DEBUG TryExportImageAsDds exited with undef due to no path" << std::endl;
        return;
    }

    auto success = DistantWorlds::Ide::TryExportImageAsDds(handle, path.pBuffer,
            CheckedNarrowToInt(path.nBuffer));

    info.GetReturnValue().Set(v8::Integer::New(isolate, success ? 1 : 0));
}