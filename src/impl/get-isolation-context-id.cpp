#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void GetIsolationContextIdImpl(const FunctionCallbackInfo<v8::Value> &info) {
    try {
#if TRACE_ENTER
        std::wcerr << L"DEBUG GetIsolationContextIdImpl entered" << std::endl;
#endif
        auto *isolate = info.GetIsolate();
        auto context = isolate->GetCurrentContext();
        auto argCount = info.Length();
        if (argCount != 0) {
            const auto &kErr = v8::String::NewFromUtf8Literal(isolate, "Expected no argument(s)",
                    v8::NewStringType::kInternalized);
            isolate->ThrowException(kErr);
            return;
        }
        auto isoCtxNum = -1;
        auto thisArg = info.This();
        if (thisArg->IsObject()) {
            auto ctx = Dw2IdeContext::FromFunctionCallbackInfo(info);
            auto sym = ctx->IsolationContextIdSymbol.Get(isolate);
            auto ctxId = thisArg->Get(context, sym).ToLocalChecked();
            if (ctxId->IsInt32())
                isoCtxNum = ctxId->Int32Value(context).ToChecked();
        }

        info.GetReturnValue().Set(v8::Integer::New(isolate, isoCtxNum));
    } catch (const std::exception &e) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = v8::String::NewFromUtf8(isolate, e.what(),
                v8::NewStringType::kInternalized).ToLocalChecked();
        isolate->ThrowException(kErr);
    }
}
