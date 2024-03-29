#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void GetObjectSimplifiedTypeImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG GetObjectSimplifiedType entered" << std::endl;
#endif
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    int argCount = info.Length();
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
    //std::wcerr << L"DEBUG GetObjectSimplifiedType getting arg0" << std::endl;
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
    //std::wcerr << L"DEBUG GetObjectSimplifiedType checking arg0 length" << std::endl;
    if (objectIdLength != DistantWorlds::Ide::ObjectIdByteSize) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be ObjectIdByteSize bytes in length", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    wchar_t *pObjectType = nullptr;
    int32_t *pObjectTypeLength = nullptr;

    auto *objectId = reinterpret_cast<uint8_t *>(objectIdData->Data());
    //std::wcerr << L"DEBUG GetObjectSimplifiedType objectId: 0x" << std::hex
    //        << std::setw(2) << std::setfill(L'0');
    //for (int i = 0; i < 16; ++i)
    //    std::wcerr << (unsigned char) objectId[i];
    //std::wcerr << std::dec << std::endl;
    //std::wcerr << L"DEBUG GetObjectSimplifiedType invoking CLR method to get buf size" << std::endl;
    auto needed = DistantWorlds::Ide::GetObjectSimplifiedType(
            objectId,
            pObjectType, 0, isoCtxNum);
    if (needed == 0) {
        //std::wcerr << L"DEBUG GetObjectSimplifiedType exited with undef" << std::endl;
        return;
    }
    auto size = 1 + -needed;
    //std::wcerr << L"DEBUG GetObjectSimplifiedType CLR returned " << needed << L", so size " << size << std::endl;
    pObjectType = (wchar_t *) _alloca(size * sizeof(wchar_t));
    auto wrote = DistantWorlds::Ide::GetObjectSimplifiedType(
            reinterpret_cast<uint8_t *>(objectIdData->Data()),
            pObjectType, size, isoCtxNum);
    while (pObjectType[wrote - 1] == 0) wrote--;
    const v8::Local<v8::String> &str = v8::String::NewFromTwoByte(info.GetIsolate(),
            (const uint16_t *) pObjectType,
            v8::NewStringType::kNormal, wrote).ToLocalChecked();
    //std::wcerr << L"DEBUG GetObjectSimplifiedType CLR returned " << wrote << std::endl;
    if (wrote < 0) DEBUG_BREAK();
    info.GetReturnValue().Set(str);
    std::wstring_view svBuffer(pObjectType, wrote);
    //std::wcerr << L"DEBUG GetObjectSimplifiedType exited with string: "
    //        << svBuffer << std::endl;
}
