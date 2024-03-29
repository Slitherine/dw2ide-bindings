#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;


void TryExportObjectImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG TryExportObjectImpl entered" << std::endl;
#endif
    // arg0 is object id, arg1 is path
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 2) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 2 arguments", v8::NewStringType::kInternalized);
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
    if (!arg0->IsArrayBufferView() && !arg0->IsArrayBuffer()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be an ArrayBuffer or an ArrayBufferView", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto objectIdData = arg0->IsArrayBufferView()
            ? arg0.As<v8::ArrayBufferView>()->Buffer()
            : arg0.As<v8::ArrayBuffer>();

    auto objectIdLength = objectIdData->ByteLength();
    if (objectIdLength != DistantWorlds::Ide::ObjectIdByteSize) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be ObjectIdByteSize bytes in length", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto arg1 = info[1];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg1->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG TryExportObject arg1 is a " << typeNameBuffer << std::endl;

    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 2nd argument to be a string", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto maybePath = arg1->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybePath, path);
    if (path.pBuffer == nullptr || path.nBuffer == 0) {
        //std::wcerr << L"DEBUG TryExportObject exited with undef due to no path" << std::endl;
        return;
    }

    auto success = DistantWorlds::Ide::TryExportObject(
            reinterpret_cast<uint8_t *>(objectIdData->Data()),
            path.pBuffer, CheckedNarrowToInt(path.nBuffer), isoCtxNum);

    info.GetReturnValue().Set(v8::Integer::New(isolate, success ? 1 : 0));


}