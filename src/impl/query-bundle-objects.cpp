#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void QueryBundleObjectsImpl(const FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG QueryBundleObjectsImpl entered" << std::endl;
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
    auto arg0 = info[0];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg0->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG QueryBundleObject arg0 is a " << typeNameBuffer << std::endl;
    if (!arg0->IsExternal()) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be a handle", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg1 = info[1];
    memset(typeNameBuffer, 0, 16);
    arg1->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG QueryBundleObject arg1 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = v8::String::NewFromUtf8Literal(isolate,
                "Expected 2nd argument to be a string", v8::NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto maybeGlob = arg1->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybeGlob, glob);
    intptr_t handle;
    //std::wcerr << L"DEBUG QueryBundleObject invoking CLR method" << std::endl;
    handle = DistantWorlds::Ide::QueryBundleObjects(
            reinterpret_cast<intptr_t>(arg0.As<v8::External>()->Value()),
            glob.pBuffer, CheckedNarrowToInt(glob.nBuffer));
    if (handle == 0) {
        //std::wcerr << L"DEBUG QueryBundleObject exited with undef" << std::endl;
        return;
    }
    //std::wcerr << L"DEBUG QueryBundleObject CLR returned" << std::endl;
    //auto *ctx = Dw2IdeContext::FromFunctionCallbackInfo(info);
    //std::wcerr << L"DEBUG QueryBundleObject creating managed handle object" << std::endl;
    //auto obj = ctx->ManagedHandleTemplate.Get(isolate)->NewInstance(context).ToLocalChecked();
    //obj->SetAlignedPointerInInternalField(1, reinterpret_cast<void *>(handle));
    //std::wcerr << L"DEBUG QueryBundleObject creating external handle value" << std::endl;
    auto extHandle = v8::External::New(isolate, reinterpret_cast<void *>(handle));
    //std::wcerr << L"DEBUG QueryBundleObject returning managed handle object" << std::endl;
    //info.GetReturnValue().Set(obj);
    info.GetReturnValue().Set(extHandle);
    //std::wcerr << L"DEBUG QueryBundleObject exited with handle" << std::endl;
}
