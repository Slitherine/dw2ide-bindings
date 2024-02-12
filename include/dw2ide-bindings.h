#pragma once

#include <node.h>

namespace DistantWorlds::Ide {

#include <DistantWorlds.IDE.Interop.h>

}


namespace dw2ide {
    using namespace v8;

    class Dw2IdeContext {
    public:

        explicit Dw2IdeContext(Isolate *isolate) {
            node::AddEnvironmentCleanupHook(isolate, Cleanup, this);
        }

        static void Cleanup(void *data) {
            auto *module = static_cast<Dw2IdeContext *>(data);
            delete module;
        }

        static Dw2IdeContext *FromFunctionCallbackInfo(const v8::FunctionCallbackInfo<v8::Value> &info) {
            return reinterpret_cast<Dw2IdeContext *>(info.Data().As<External>()->Value());
        }


        Global<ObjectTemplate> ManagedHandleTemplate;
    };

    extern "C" NODE_MODULE_EXPORT void
    NODE_MODULE_INITIALIZER_X(NODE_MODULE_INITIALIZER_BASE, 118)
            (Local<Object> exports,
             Local<Value> module,
             Local<Context> context);

    // redirect compatible versions to the same export

    extern "C" NODE_MODULE_EXPORT void
    NODE_MODULE_INITIALIZER_X(NODE_MODULE_INITIALIZER_BASE, 119)
            (Local<Object> exports,
             Local<Value> module,
             Local<Context> context) {
        NODE_MODULE_INITIALIZER_X(NODE_MODULE_INITIALIZER_BASE, 118)(exports, module, context);
    }

    extern "C" NODE_MODULE_EXPORT void
    NODE_MODULE_INITIALIZER_X(NODE_MODULE_INITIALIZER_BASE, 120)
            (Local<Object> exports,
             Local<Value> module,
             Local<Context> context) {
        NODE_MODULE_INITIALIZER_X(NODE_MODULE_INITIALIZER_BASE, 118)(exports, module, context);
    }
}