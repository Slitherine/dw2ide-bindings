#pragma once
#include <node.h>

namespace dw2ide {
    using namespace v8;

    class Dw2IdeContext {
    public:

        explicit Dw2IdeContext(Isolate* isolate) {
            node::AddEnvironmentCleanupHook(isolate, Cleanup, this);
        }

        static void Cleanup(void* data) {
            auto *module = static_cast<Dw2IdeContext *>(data);
            delete module;
        }

        static Dw2IdeContext* FromFunctionCallbackInfo(const v8::FunctionCallbackInfo<v8::Value>& info) {
            return reinterpret_cast<Dw2IdeContext*>(info.Data().As<External>()->Value());
        }
    };

    extern "C" NODE_MODULE_EXPORT void
    NODE_MODULE_INITIALIZER(Local<Object> exports,
                            Local<Value> module,
                            Local<Context> context) {
        Isolate* isolate = context->GetIsolate();
        auto* ctx = new Dw2IdeContext(isolate);
        Local<External> external = External::New(isolate, ctx);


    }
}