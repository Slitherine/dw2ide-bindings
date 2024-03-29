#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void LoadBundleImpl(const FunctionCallbackInfo<v8::Value> &info) {
    try {
#if TRACE_ENTER
        std::wcerr << L"DEBUG LoadBundle entered" << std::endl;
#endif
        auto *isolate = info.GetIsolate();
        auto context = isolate->GetCurrentContext();
        auto argCount = info.Length();
        //std::wcerr << L"DEBUG LoadBundle argCount: " << argCount << std::endl;
        if (argCount != 1) {
            const auto &kErr = v8::String::NewFromUtf8Literal(isolate, "Expected 1 argument",
                    v8::NewStringType::kInternalized);
            isolate->ThrowException(kErr);
            return;
        }
        //std::wcerr << L"DEBUG LoadBundle identifying isolation context" << std::endl;
        auto isoCtxNum = 0;
        auto thisArg = info.This();
        //std::wcerr << L"DEBUG LoadBundle checking if called on an object (this)" << std::endl;
        if (thisArg->IsObject()) {
            //std::wcerr << L"DEBUG LoadBundle retrieving IDE context" << std::endl;
            auto ctx = Dw2IdeContext::FromFunctionCallbackInfo(info);
            //std::wcerr << L"DEBUG LoadBundle identifying if 'this' has an isolation context" << std::endl;
            auto sym = ctx->IsolationContextIdSymbol.Get(isolate);
            //std::wcerr << L"DEBUG LoadBundle checking isolation context symbol property" << std::endl;
            auto ctxId = thisArg->Get(context,sym).ToLocalChecked();
            if (ctxId->IsInt32()) {
                //std::wcerr << L"DEBUG LoadBundle getting isolation context from property" << std::endl;
                isoCtxNum = ctxId->Int32Value(context).ToChecked();
                //std::wcerr << L"DEBUG LoadBundle got isolation context "<< isoCtxNum << std::endl;
            }
        }
        //std::wcerr << L"DEBUG LoadBundle getting arg0" << std::endl;
        auto arg0 = info[0];
        //std::wcerr << L"DEBUG LoadBundle checking arg0" << std::endl;
        char typeNameBuffer[16];
        memset(typeNameBuffer, 0, 16);
        arg0->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
        //std::wcerr << L"DEBUG LoadBundle arg0 is a " << typeNameBuffer << std::endl;
        if (strncmp(typeNameBuffer, "string", 16) != 0) {
            //std::wcerr << L"DEBUG LoadBundle arg0 is not a string" << std::endl;
            const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                    "Expected 1st argument to be a string",
                    v8::NewStringType::kNormal);
            isolate->ThrowException(kErr);
            return;
        }
        auto maybePath = arg0->ToString(context);
        //std::wcerr << L"DEBUG LoadBundle extracting arg0 string content" << std::endl;
        DistantWorlds::Ide::Utf16String path = {nullptr, 0};
        {
            if (!maybePath.IsEmpty()) {
                auto pathArg_ = maybePath.ToLocalChecked();
                auto pathLen_ = pathArg_->Length();
                //std::wcerr << L"DEBUG LoadBundle extracting arg0 with length " << pathLen_ << std::endl;
                path = {reinterpret_cast<wchar_t *>(_alloca(pathLen_ * sizeof(wchar_t))),
                        static_cast<size_t>(pathLen_)};

                //std::wcerr << L"DEBUG LoadBundle allocated path" << std::endl;
                pathArg_->Write(isolate, reinterpret_cast<uint16_t *>(path.pBuffer), 0, pathLen_,
                        v8::String::NO_NULL_TERMINATION);
                //std::wcerr << L"DEBUG LoadBundle wrote path" << std::endl;
            }
        }
        /*std::wstring_view svPath(path.pBuffer, path.nBuffer);
        std::wcerr << L"path: 0x" << std::hex << (size_t) path.pBuffer << std::dec << L" "
                << svPath << std::endl;*/
        //std::wcerr << L"DEBUG LoadBundle invoking CLR method" << std::endl;
        auto handle = DistantWorlds::Ide::LoadBundle(path.pBuffer, CheckedNarrowToInt(path.nBuffer), isoCtxNum);
        //std::wcerr << L"DEBUG LoadBundle CLR returned" << std::endl;
        if (handle == 0) {
            //std::wcerr << L"DEBUG LoadBundle exited with undef" << std::endl;
        } else {
            v8::ReturnValue<v8::Value> value = info.GetReturnValue();
            //std::wcerr << L"DEBUG LoadBundle getting Dw2IdeContext" << std::endl;
            auto *ctx = dw2ide::Dw2IdeContext::FromFunctionCallbackInfo(info);
            //std::wcerr << L"DEBUG LoadBundle creating managed handle object" << std::endl;
            //const auto obj = ctx->ManagedHandleTemplate.Get(isolate)->NewInstance(context).ToLocalChecked();
            //if (obj->InternalFieldCount() != 2) DEBUG_BREAK();
            //std::wcerr << L"DEBUG LoadBundle creating external handle value" << std::endl;
            const auto extHandle = v8::External::New(isolate, reinterpret_cast<void *>(handle));
            //std::wcerr << L"DEBUG LoadBundle setting managed handle object internal field" << std::endl;
            //obj->SetAlignedPointerInInternalField(1, reinterpret_cast<void *>(handle));
            //std::wcerr << L"DEBUG LoadBundle returning managed handle object" << std::endl;
            value.Set(extHandle);
            //std::wcerr << L"DEBUG LoadBundle exited with handle" << std::endl;
        }
        //std::wcerr << L"DEBUG LoadBundle exited successfully: 0x" << std::hex << handle << std::dec
        //        << std::endl;
    } catch (const std::exception &e) {
        //std::wcerr << L"DEBUG LoadBundle raised exception: " << e.what() << std::endl;
        auto *isolate = info.GetIsolate();
        const auto &kErr = v8::String::NewFromUtf8(isolate, e.what(),
                v8::NewStringType::kInternalized).ToLocalChecked();
        isolate->ThrowException(kErr);
    }
}
