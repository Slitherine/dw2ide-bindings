#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void InstantiateBundleItemImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG InstantiateBundleItemImpl entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    std::wcerr << L"DEBUG InstantiateBundleItem argCount: " << argCount << std::endl;
    if (argCount != 1) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto isoCtxNum = 0;
    auto thisArg = info.This();
    if (thisArg->IsObject()) {
        auto ctx = Dw2IdeContext::FromFunctionCallbackInfo(info);
        auto sym = ctx->IsolationContextIdSymbol.Get(isolate);
        auto ctxId = thisArg->Get(context,sym).ToLocalChecked();
        if (ctxId->IsInt32())
            isoCtxNum = ctxId->Int32Value(context).ToChecked();
    }
    auto arg0 = info[0];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg0->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    std::wcerr << L"DEBUG InstantiateBundleItem arg0 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be a string", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto maybeUrl = arg0->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybeUrl, url);
    if (url.pBuffer == nullptr || url.nBuffer == 0) {
        std::wcerr << L"DEBUG InstantiateBundleItem exited with undef due to no path" << std::endl;
        return;
    }
    std::wcerr << L"DEBUG InstantiateBundleItem invoking CLR method" << std::endl;
    auto handle = DistantWorlds::Ide::InstantiateBundleItem(url.pBuffer, CheckedNarrowToInt(url.nBuffer), isoCtxNum);
    std::wcerr << L"DEBUG InstantiateBundleItem CLR returned" << std::endl;
    if (handle == 0) {
        std::wcerr << L"DEBUG InstantiateBundleItem exited with undef" << std::endl;
        return;
    }
    std::wcerr << L"DEBUG InstantiateBundleItem instance handle: 0x" << std::hex << handle << std::dec
            << std::endl;
    auto extHandle = v8::External::New(isolate, reinterpret_cast<void *>(handle));
    info.GetReturnValue().Set(extHandle);
    std::wcerr << L"DEBUG InstantiateBundleItem exited with handle" << std::endl;
}