#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void NewIsolationContextImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
    try {
#if TRACE_ENTER
        std::wcerr << L"DEBUG NewIsolationContextImpl entered" << std::endl;
#endif
        // get exports from context class
        auto *ctx = Dw2IdeContext::FromFunctionCallbackInfo(info);
        if (!ctx) {
            // undefined
            return;
        }
        // new object with internal field set to new context id
        auto *isolate = info.GetIsolate();
        auto context = isolate->GetCurrentContext();
        auto exports = ctx->Exports.Get(isolate);
        // copy the exports to a new object
        auto keys = exports->GetPropertyNames(context).ToLocalChecked();
        auto obj = Object::New(isolate);
        auto length = keys->Length();
        for (uint32_t i = 0; i < length; i++) {
            auto key = keys->Get(context, i).ToLocalChecked();
            auto value = exports->Get(context, key).ToLocalChecked();
            obj->Set(context, key, value).FromJust();
        }
        // set the field to the new context id
        auto ctxId = DistantWorlds::Ide::StartIsolationContext();
        auto sym = ctx->IsolationContextIdSymbol.Get(isolate);
        obj->Set(context, sym, Integer::New(isolate, ctxId)).FromJust();
        // validate object
        if (obj->Get(context, sym).ToLocalChecked()->IntegerValue(context).ToChecked()
                != ctxId)
            throw std::runtime_error("Failed to validate setting of isolation context Id");
        info.GetReturnValue().Set(obj);
    } catch (const std::exception &e) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = v8::String::NewFromUtf8(isolate, e.what(),
                v8::NewStringType::kInternalized).ToLocalChecked();
        isolate->ThrowException(kErr);
    }
}