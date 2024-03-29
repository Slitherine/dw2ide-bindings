#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void TryGetObjectIdImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
    try {
#if TRACE_ENTER
        std::wcerr << L"DEBUG TryGetObjectIdImpl entered" << std::endl;
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
        auto isoCtxNum = 0;
        auto thisArg = info.This();
        if (thisArg->IsObject()) {
            auto ctx = Dw2IdeContext::FromFunctionCallbackInfo(info);
            auto sym = ctx->IsolationContextIdSymbol.Get(isolate);
            auto ctxId = thisArg->Get(context, sym).ToLocalChecked();
            if (ctxId->IsInt32())
                isoCtxNum = ctxId->Int32Value(context).ToChecked();
        }
        auto arg0 = info[0];
        char typeNameBuffer[16];
        memset(typeNameBuffer, 0, 16);
        arg0->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
        //std::wcerr << L"DEBUG TryGetObjectId arg0 is a " << typeNameBuffer << std::endl;
        if (strncmp(typeNameBuffer, "string", 16) != 0) {
            const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                    "Expected 1st argument to be a string", v8::NewStringType::kInternalized);
            isolate->ThrowException(kErr);
            return;
        }
        auto arg1 = info[1];
        if (!arg1->IsArrayBufferView() && !arg1->IsArrayBuffer()) {
            const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                    "Expected 1st argument to be an ArrayBuffer or an ArrayBufferView",
                    v8::NewStringType::kInternalized);
            isolate->ThrowException(kErr);
            return;
        }
        auto maybePath = arg0->ToString(context);
        DW2IDE_EXTRACT_UTF16STRING(maybePath, path);
        auto objectIdData = arg1->IsArrayBufferView()
                ? arg1.As<v8::ArrayBufferView>()->Buffer()
                : arg1.As<v8::ArrayBuffer>();
        auto objectIdLength = objectIdData->ByteLength();
        if (objectIdLength != DistantWorlds::Ide::ObjectIdByteSize) {
            const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                    "Expected 1st argument to be ObjectIdByteSize bytes in length", v8::NewStringType::kInternalized);
            isolate->ThrowException(kErr);
            return;
        }
        auto objectId = reinterpret_cast<uint8_t *>(objectIdData->Data());
        bool success = DistantWorlds::Ide::TryGetObjectId(path.pBuffer, CheckedNarrowToInt(path.nBuffer), objectId,
                isoCtxNum);
        info.GetReturnValue().Set(v8::Integer::New(isolate, success ? 1 : 0));
    } catch (const std::exception &e) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = v8::String::NewFromUtf8(isolate, e.what(),
                v8::NewStringType::kInternalized).ToLocalChecked();
        isolate->ThrowException(kErr);
    }
}
