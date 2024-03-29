#pragma once

#include <node.h>

namespace DistantWorlds::Ide {

#include <DistantWorlds.IDE.Interop.h>

}

#ifndef __DNNE_GENERATED_HEADER_DISTANTWORLDS_IDE__
#error Failed to include DistantWorlds.IDE.Interop.h
#endif


namespace dw2ide {
    using namespace v8;

    class Dw2IdeContext {
    public:

        explicit Dw2IdeContext(Isolate *isolate, Local<Object> & exports) {
            node::AddEnvironmentCleanupHook(isolate, Cleanup, this);
            Exports.Reset(isolate, exports);
            const auto &ctxSymbol = Symbol::New(isolate, String::NewFromUtf8Literal(isolate, "IsoCtxId", NewStringType::kInternalized));
            IsolationContextIdSymbol.Reset(isolate, ctxSymbol);
        }

        static void Cleanup(void *data) {
            auto *module = static_cast<Dw2IdeContext *>(data);
            module->IsolationContextIdSymbol.Reset();
            delete module;
        }

        static Dw2IdeContext *FromFunctionCallbackInfo(const v8::FunctionCallbackInfo<v8::Value> &info) {
            return reinterpret_cast<Dw2IdeContext *>(info.Data().As<External>()->Value());
        }

        Global<Object> Exports;
        Global<Symbol> IsolationContextIdSymbol;
    };

    /*
    extern "C" NODE_MODULE_EXPORT void
    NODE_MODULE_INITIALIZER_X(NODE_MODULE_INITIALIZER_BASE, 118)
            (Local<Object> exports,
                    Local<Value> module,
                    Local<Context> context);

    extern "C" NODE_MODULE_EXPORT void
    NODE_MODULE_INITIALIZER_X(NODE_MODULE_INITIALIZER_BASE, 119)
            (Local<Object> exports,
                    Local<Value> module,
                    Local<Context> context);

    extern "C" NODE_MODULE_EXPORT void
    NODE_MODULE_INITIALIZER_X(NODE_MODULE_INITIALIZER_BASE, 120)
            (Local<Object> exports,
                    Local<Value> module,
                    Local<Context> context);
*/

    extern "C" NODE_MODULE_EXPORT void
    NODE_MODULE_INITIALIZER_X(NODE_MODULE_INITIALIZER_BASE, 121)
            (Local<Object> exports,
             Local<Value> module,
             Local<Context> context);

}