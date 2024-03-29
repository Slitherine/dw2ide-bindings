#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void ShowMessageBoxImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG ShowMessageBoxImpl entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 4) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 4 arguments", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg0->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG ShowMessageBox arg0 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be a string", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto maybeMessage = arg0->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybeMessage, message);
    if (message.pBuffer == nullptr || message.nBuffer == 0) {
        return;
    }
    auto arg1 = info[1];
    memset(typeNameBuffer, 0, 16);
    arg1->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG ShowMessageBox arg1 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 2nd argument to be a string", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto maybeTitle = arg1->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybeTitle, title);
    if (title.pBuffer == nullptr || title.nBuffer == 0) {
        return;
    }
    auto arg2 = info[2];
    if (!arg2->IsInt32()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 3rd argument to be an integer", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto messageBoxButtons = static_cast<DistantWorlds::Ide::MessageBoxButtons_t>(arg2->Int32Value(
            context).ToChecked());
    auto arg3 = info[3];
    if (!arg3->IsInt32()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 4th argument to be an integer", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto messageBoxType = static_cast<DistantWorlds::Ide::MessageBoxType_t>(arg3->Int32Value(context).ToChecked());
    auto result = DistantWorlds::Ide::ShowMessageBox(message.pBuffer, CheckedNarrowToInt(message.nBuffer),
            title.pBuffer, CheckedNarrowToInt(title.nBuffer), messageBoxButtons, messageBoxType);
    info.GetReturnValue().Set(v8::Integer::New(isolate, result));
}