#include "dw2ide-bindings.h"
#include <iostream>
#include <excpt.h>
#include <sstream>
#include <iomanip>

using namespace dw2ide;
using namespace v8;

#ifndef DEBUG_BREAK
#if defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#if __has_builtin(__builtin_debugtrap)
#define DEBUG_BREAK() __builtin_debugtrap()
#else
#define DEBUG_BREAK() __builtin_trap()
#endif
#endif
#endif

#define DW2IDE_EXTRACT_UTF16STRING(maybeStr, str) \
DistantWorlds::Ide::Utf16String str = {nullptr, 0}; \
{ \
    if (!maybeStr.IsEmpty()) { \
        auto str##Arg_ = maybeStr.ToLocalChecked(); \
        auto str##Len_ = str##Arg_->Length(); \
        str = { \
                reinterpret_cast<DNNE_WCHAR *>(_alloca(str##Len_ * sizeof(DNNE_WCHAR))), \
                static_cast<size_t>(str##Len_) \
        }; \
        str##Arg_->Write(isolate, \
                reinterpret_cast<uint16_t *>(str.pBuffer), \
                0, str##Len_, \
                String::WriteOptions::NO_NULL_TERMINATION); \
    } \
} (void)(0)

#define DW2IDE_EXPORT_FUNCTION(name, ...) \
    const auto &k##name = String::NewFromUtf8Literal(isolate, #name, NewStringType::kInternalized); \
    exports->Set(context, k##name, FunctionTemplate::New(isolate, __VA_ARGS__, \
    ctxExternal)->GetFunction(context).ToLocalChecked()).FromJust()
#define DW2IDE_EXPORT_FUNCTION_IMPL(name) \
    const auto &k##name = String::NewFromUtf8Literal(isolate, #name, NewStringType::kInternalized); \
    exports->Set(context, k##name, FunctionTemplate::New(isolate, name##Impl, \
    ctxExternal)->GetFunction(context).ToLocalChecked()).FromJust()

#define DW2IDE_EXPORT_FUNCTION_GET_STRING(name) \
    DW2IDE_EXPORT_FUNCTION(name, [](const FunctionCallbackInfo<Value> &info) { \
        auto *isolate = info.GetIsolate(); \
        auto needed = DistantWorlds::Ide::name(nullptr, 0); \
        if (needed == 0) { \
            return; \
        } \
        auto size = -needed; \
        auto *buffer = (wchar_t *) _alloca(size * sizeof(wchar_t)); \
        auto wrote = DistantWorlds::Ide::name(buffer, size);\
        while (buffer[wrote-1] == 0) wrote--; \
        const Local<String> &str = String::NewFromTwoByte(info.GetIsolate(), \
                (const uint16_t *) buffer, \
                NewStringType::kNormal, wrote).ToLocalChecked(); \
        info.GetReturnValue().Set(str); \
    })


int32_t CheckedNarrowToInt(size_t v);

void DeisolateImpl(const FunctionCallbackInfo<Value> &info) {
    DistantWorlds::Ide::Deisolate();
};

void DebugBreakImpl(const FunctionCallbackInfo<Value> &info) {
    DEBUG_BREAK();
};

void ClrDebugBreakImpl(const FunctionCallbackInfo<Value> &info) {
    DistantWorlds::Ide::DebugBreak();
};

void ReleaseHandleImpl(const FunctionCallbackInfo<Value> &info) {
    if (info.Length() != 1) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    if (!info[0]->IsExternal()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(info[0].As<External>()->Value());
    //auto handle = dw2ide::ExtractHandle(info[0]);

    DistantWorlds::Ide::ReleaseHandle(handle);
}

void HandleToStringImpl(const FunctionCallbackInfo<Value> &info) {
    //std::wcerr << L"DEBUG HandleToString entered" << std::endl;
    auto *isolate = info.GetIsolate();
    if (info.Length() != 1) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    //std::wcerr << L"DEBUG HandleToString getting arg0" << std::endl;
    if (!info[0]->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    //std::wcerr << L"DEBUG HandleToString extracting handle" << std::endl;
    auto handle = reinterpret_cast<intptr_t>(info[0].As<External>()->Value());
    //auto handle = dw2ide::ExtractHandle(info[0]);
    wchar_t *buffer = nullptr;
    size_t size = 0;
    //std::wcerr << L"DEBUG HandleToString invoking CLR method to get buf size" << std::endl;
    auto needed = DistantWorlds::Ide::HandleToString(handle, buffer, size);
    if (needed == 0) {
        //std::wcerr << L"DEBUG HandleToString CLR returned 0" << std::endl;
        //std::wcerr << L"DEBUG HandleToString exited with undef" << std::endl;
        return;
    }
    size = 1 + -needed;
    //std::wcerr << L"DEBUG HandleToString CLR returned " << needed << L", so size " << size << std::endl;
    buffer = (wchar_t *) _alloca(size * sizeof(wchar_t));
    //std::wcerr << L"DEBUG HandleToString invoking CLR method to get string" << std::endl;
    auto wrote = DistantWorlds::Ide::HandleToString(handle, buffer, size);
    //std::wcerr << L"DEBUG HandleToString CLR returned" << std::endl;
    while (buffer[wrote - 1] == 0) wrote--;
    std::wstring_view svBuffer(buffer, wrote);
    //std::wcerr << L"DEBUG HandleToString value: " << svBuffer << std::endl;
    const Local<String> &str = String::NewFromTwoByte(info.GetIsolate(),
            (const uint16_t *) buffer,
            NewStringType::kNormal, wrote).ToLocalChecked();
    info.GetReturnValue().Set(str);
}

void LoadBundleImpl(const FunctionCallbackInfo<Value> &info) {
    try {
        //std::wcerr << L"DEBUG LoadBundle entered" << std::endl;
        auto *isolate = info.GetIsolate();
        auto context = isolate->GetCurrentContext();
        auto argCount = info.Length();
        //std::wcerr << L"DEBUG LoadBundle argCount: " << argCount << std::endl;
        if (argCount != 1) {
            const auto &kErr = String::NewFromUtf8Literal(isolate, "Expected 1 argument",
                    NewStringType::kInternalized);
            isolate->ThrowException(kErr);
            return;
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
            const auto &kErr = String::NewFromUtf8Literal(isolate,
                    "Expected 1st argument to be a string",
                    NewStringType::kNormal);
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
                        String::WriteOptions::NO_NULL_TERMINATION);
                //std::wcerr << L"DEBUG LoadBundle wrote path" << std::endl;
            }
        }
        std::wstring_view svPath(path.pBuffer, path.nBuffer);
        //std::wcerr << L"path: 0x" << std::hex << (size_t) path.pBuffer << std::dec << L" "
        //        << svPath << std::endl;
        //std::wcerr << L"DEBUG LoadBundle invoking CLR method" << std::endl;
        auto handle = DistantWorlds::Ide::LoadBundle(path.pBuffer, CheckedNarrowToInt(path.nBuffer));
        //std::wcerr << L"DEBUG LoadBundle CLR returned" << std::endl;
        if (handle == 0) {
            //std::wcerr << L"DEBUG LoadBundle exited with undef" << std::endl;
        } else {
            ReturnValue<Value> value = info.GetReturnValue();
            //std::wcerr << L"DEBUG LoadBundle getting Dw2IdeContext" << std::endl;
            auto *ctx = Dw2IdeContext::FromFunctionCallbackInfo(info);
            //std::wcerr << L"DEBUG LoadBundle creating managed handle object" << std::endl;
            //const auto obj = ctx->ManagedHandleTemplate.Get(isolate)->NewInstance(context).ToLocalChecked();
            //if (obj->InternalFieldCount() != 2) DEBUG_BREAK();
            //std::wcerr << L"DEBUG LoadBundle creating external handle value" << std::endl;
            const auto extHandle = External::New(isolate, reinterpret_cast<void *>(handle));
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
        const auto &kErr = String::NewFromUtf8(isolate, e.what(),
                NewStringType::kInternalized).ToLocalChecked();
        isolate->ThrowException(kErr);
    }
}

void QueryBundleObjectsImpl(const FunctionCallbackInfo<Value> &info) {
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 2) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg0->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG QueryBundleObject arg0 is a " << typeNameBuffer << std::endl;
    if (!arg0->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg1 = info[1];
    memset(typeNameBuffer, 0, 16);
    arg1->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG QueryBundleObject arg1 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2nd argument to be a string", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto maybeGlob = arg1->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybeGlob, glob);
    intptr_t handle;
    //std::wcerr << L"DEBUG QueryBundleObject invoking CLR method" << std::endl;
    handle = DistantWorlds::Ide::QueryBundleObjects(
            reinterpret_cast<intptr_t>(arg0.As<External>()->Value()),
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
    auto extHandle = External::New(isolate, reinterpret_cast<void *>(handle));
    //std::wcerr << L"DEBUG QueryBundleObject returning managed handle object" << std::endl;
    //info.GetReturnValue().Set(obj);
    info.GetReturnValue().Set(extHandle);
    //std::wcerr << L"DEBUG QueryBundleObject exited with handle" << std::endl;
}

void ReadQueriedBundleObjectImpl(const FunctionCallbackInfo<Value> &info) {
    //std::wcerr << L"DEBUG ReadQueriedBundleObject entered" << std::endl;
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    //std::wcerr << L"DEBUG ReadQueriedBundleObject argCount: " << argCount << std::endl;
    if (argCount != 1) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto arg0 = info[0];
    //std::wcerr << L"DEBUG ReadQueriedBundleObject checking arg0" << std::endl;
    if (!arg0->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    //std::wcerr << L"DEBUG ReadQueriedBundleObject extracting handle" << std::endl;
    auto bundleQueryHandle = reinterpret_cast<intptr_t>(arg0.As<External>()->Value());
    //std::wcerr << L"DEBUG ReadQueriedBundleObject handle: 0x" << std::hex << bundleQueryHandle << std::dec
    //        << std::endl;
    wchar_t *buffer = nullptr;
    size_t size = 0;
    //std::wcerr << L"DEBUG ReadQueriedBundleObject invoking CLR method to get buf size" << std::endl;
    auto needed = DistantWorlds::Ide::ReadQueriedBundleObject(bundleQueryHandle, buffer, size);
    if (needed == 0) {
        //std::wcerr << L"DEBUG ReadQueriedBundleObject exited with undef" << std::endl;
        return;
    }
    size = 1 + -needed;
    //std::wcerr << L"DEBUG ReadQueriedBundleObject CLR returned " << needed << L", so size " << size << std::endl;
    buffer = (wchar_t *) _alloca(size * sizeof(wchar_t));
    auto wrote = DistantWorlds::Ide::ReadQueriedBundleObject(bundleQueryHandle, buffer, size);
    while (buffer[wrote - 1] == 0) wrote--;
    const Local<String> &str = String::NewFromTwoByte(info.GetIsolate(),
            (const uint16_t *) buffer,
            NewStringType::kNormal, wrote).ToLocalChecked();
    std::wstring_view svBuffer(buffer, wrote);
    //std::wcerr << L"DEBUG ReadQueriedBundleObject value: " << svBuffer << std::endl;
    info.GetReturnValue().Set(str);
    //std::wcerr << L"DEBUG ReadQueriedBundleObject exited with string" << std::endl;
}

void TryGetObjectIdImpl(const FunctionCallbackInfo<Value> &info) {
    auto *isolate = info.GetIsolate();
    if (info.Length() != 2) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg0->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG TryGetObjectId arg0 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be a string", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg1 = info[1];
    if (!arg1->IsArrayBufferView() && !arg1->IsArrayBuffer()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be an ArrayBuffer or an ArrayBufferView", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto context = isolate->GetCurrentContext();
    auto maybePath = arg0->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybePath, path);
    auto objectIdData = arg1->IsArrayBufferView()
            ? arg1.As<ArrayBufferView>()->Buffer()
            : arg1.As<ArrayBuffer>();
    auto objectIdLength = objectIdData->ByteLength();
    if (objectIdLength != DistantWorlds::Ide::ObjectIdByteSize) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be ObjectIdByteSize bytes in length", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto objectId = reinterpret_cast<uint8_t *>(objectIdData->Data());
    bool success = DistantWorlds::Ide::TryGetObjectId(path.pBuffer, CheckedNarrowToInt(path.nBuffer), objectId);
    info.GetReturnValue().Set(success);
}

void GetObjectOffsetImpl(const FunctionCallbackInfo<Value> &info) {

    int argCount = info.Length();
    if (argCount != 2 && argCount != 3) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2 or 3 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto arg0 = info[0];
    if (!arg0->IsArrayBufferView() && !arg0->IsArrayBuffer()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be an ArrayBuffer or an ArrayBufferView", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto objectIdData = arg0->IsArrayBufferView()
            ? arg0.As<ArrayBufferView>()->Buffer()
            : arg0.As<ArrayBuffer>();
    auto objectIdLength = objectIdData->ByteLength();

    if (objectIdLength != DistantWorlds::Ide::ObjectIdByteSize) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be ObjectIdByteSize bytes in length", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto arg1 = info[1];
    if (!arg1->IsBoolean()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2nd argument to be a boolean", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto *isolate = info.GetIsolate();
    auto wantsSourceDirOrBundlePath = arg1->BooleanValue(isolate);

    auto context = isolate->GetCurrentContext();
    int64_t *pOffset = nullptr;
    int64_t *pOffsetEnd = nullptr;

    bool success;
    wchar_t *pSourceDirOrBundlePath = nullptr;
    if (wantsSourceDirOrBundlePath) {
        success = DistantWorlds::Ide::TryGetObjectOffset(
                reinterpret_cast<uint8_t *>(objectIdData->Data()),
                pOffset, pOffsetEnd,
                &pSourceDirOrBundlePath);

    } else {
        success = DistantWorlds::Ide::TryGetObjectOffset(
                reinterpret_cast<uint8_t *>(objectIdData->Data()),
                pOffset, pOffsetEnd, nullptr);
    }
    if (!success) {
        return;
    }

    // return an object: { offset, offsetEnd, [sourceDirOrBundlePath] }
    auto obj = Object::New(isolate);
    obj->Set(context, String::NewFromUtf8Literal(isolate, "offset", NewStringType::kInternalized),
            Number::New(isolate, *pOffset)).FromJust();
    obj->Set(context, String::NewFromUtf8Literal(isolate, "offsetEnd", NewStringType::kInternalized),
            Number::New(isolate, *pOffsetEnd)).FromJust();


    if (pSourceDirOrBundlePath != nullptr) {
        // pSourceDirOrBundlePath is optional output, not input
        // it will be a null terminated UTF-16 string allocated on a native heap
        // not subject to garbage collection or memory motion
        // it is mapped by an interned pool, don't have to free it
        // once written, it will not be freed until the heap is destroyed

        const Local<String> &str = String::NewFromTwoByte(info.GetIsolate(),
                (const uint16_t *) pSourceDirOrBundlePath,
                NewStringType::kNormal).ToLocalChecked();
        obj->Set(context,
                String::NewFromUtf8Literal(isolate, "sourceDirOrBundlePath", NewStringType::kInternalized),
                str).FromJust();
    }

    info.GetReturnValue().Set(obj);
}

void GetObjectSizeImpl(const FunctionCallbackInfo<Value> &info) {

    int argCount = info.Length();
    if (argCount != 1) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto arg0 = info[0];
    if (!arg0->IsArrayBufferView() && !arg0->IsArrayBuffer()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be an ArrayBuffer or an ArrayBufferView", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto objectIdData = arg0->IsArrayBufferView()
            ? arg0.As<ArrayBufferView>()->Buffer()
            : arg0.As<ArrayBuffer>();

    auto objectIdLength = objectIdData->ByteLength();

    if (objectIdLength != DistantWorlds::Ide::ObjectIdByteSize) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be ObjectIdByteSize bytes in length", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    int64_t *pSize = nullptr;

    bool success = DistantWorlds::Ide::TryGetObjectSize(
            reinterpret_cast<uint8_t *>(objectIdData->Data()),
            pSize);

    if (!success) {
        return;
    }

    int64_t sizeValue = *pSize;

    if (sizeValue < 0) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Object size is negative, this is probably an error in the bundle data"
                " or the data type has no valid size",
                NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    if (sizeValue > 9007199254740991 /*MAX_SAFE_INTEGER*/) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Object size is too large to be represented as a JavaScript integer,"
                " this is probably an error in the bundle data",
                NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    info.GetReturnValue().Set(Number::New(isolate, (double) sizeValue));
}

void GetObjectTypeImpl(const FunctionCallbackInfo<Value> &info) {
    //std::wcerr << L"DEBUG GetObjectType entered" << std::endl;
    int argCount = info.Length();
    if (argCount != 1) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    //std::wcerr << L"DEBUG GetObjectType getting arg0" << std::endl;
    auto arg0 = info[0];
    if (!arg0->IsArrayBufferView() && !arg0->IsArrayBuffer()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be an ArrayBuffer or an ArrayBufferView", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    //std::wcerr << L"DEBUG GetObjectType checking arg0" << std::endl;
    auto objectIdData = arg0->IsArrayBufferView()
            ? arg0.As<ArrayBufferView>()->Buffer()
            : arg0.As<ArrayBuffer>();

    auto objectIdLength = objectIdData->ByteLength();
    //std::wcerr << L"DEBUG GetObjectType checking arg0 length" << std::endl;
    if (objectIdLength != DistantWorlds::Ide::ObjectIdByteSize) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be ObjectIdByteSize bytes in length", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    wchar_t *pObjectType = nullptr;
    int32_t *pObjectTypeLength = nullptr;

    auto *objectId = reinterpret_cast<uint8_t *>(objectIdData->Data());
    //std::wcerr << L"DEBUG GetObjectType objectId: 0x" << std::hex
    //        << std::setw(2) << std::setfill(L'0');
    //for (int i = 0; i < 16; ++i)
    //    std::wcerr << (unsigned char) objectId[i];
    //std::wcerr << std::dec << std::endl;

    //std::wcerr << L"DEBUG GetObjectType invoking CLR method to get buf size" << std::endl;
    auto needed = DistantWorlds::Ide::GetObjectType
            (objectId, pObjectType, 0);
    if (needed == 0) {
        //std::wcerr << L"DEBUG GetObjectType exited with undef" << std::endl;
        return;
    }
    auto size = 1 + -needed;
    //std::wcerr << L"DEBUG GetObjectType CLR returned " << needed << L", so size " << size << std::endl;
    pObjectType = (wchar_t *) _alloca(size * sizeof(wchar_t));
    auto wrote = DistantWorlds::Ide::GetObjectType(
            reinterpret_cast<uint8_t *>(objectIdData->Data()),
            pObjectType, size);
    //std::wcerr << L"DEBUG GetObjectType CLR returned " << wrote << std::endl;
    if (wrote < 0) DEBUG_BREAK();
    while (pObjectType[wrote - 1] == 0) wrote--;
    const Local<String> &str = String::NewFromTwoByte(info.GetIsolate(),
            (const uint16_t *) pObjectType,
            NewStringType::kNormal, wrote).ToLocalChecked();
    info.GetReturnValue().Set(str);
    std::wstring_view svBuffer(pObjectType, wrote);
    //std::wcerr << L"DEBUG GetObjectType exited with exited with string: "
    //        << svBuffer << std::endl;
}

void GetObjectSimplifiedTypeImpl(const FunctionCallbackInfo<Value> &info) {
    //std::wcerr << L"DEBUG GetObjectSimplifiedType entered" << std::endl;
    int argCount = info.Length();
    if (argCount != 1) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    //std::wcerr << L"DEBUG GetObjectSimplifiedType getting arg0" << std::endl;
    auto arg0 = info[0];
    if (!arg0->IsArrayBufferView() && !arg0->IsArrayBuffer()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be an ArrayBuffer or an ArrayBufferView", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto objectIdData = arg0->IsArrayBufferView()
            ? arg0.As<ArrayBufferView>()->Buffer()
            : arg0.As<ArrayBuffer>();

    auto objectIdLength = objectIdData->ByteLength();
    //std::wcerr << L"DEBUG GetObjectSimplifiedType checking arg0 length" << std::endl;
    if (objectIdLength != DistantWorlds::Ide::ObjectIdByteSize) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be ObjectIdByteSize bytes in length", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
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
            pObjectType, 0);
    if (needed == 0) {
        //std::wcerr << L"DEBUG GetObjectSimplifiedType exited with undef" << std::endl;
        return;
    }
    auto size = 1 + -needed;
    //std::wcerr << L"DEBUG GetObjectSimplifiedType CLR returned " << needed << L", so size " << size << std::endl;
    pObjectType = (wchar_t *) _alloca(size * sizeof(wchar_t));
    auto wrote = DistantWorlds::Ide::GetObjectSimplifiedType(
            reinterpret_cast<uint8_t *>(objectIdData->Data()),
            pObjectType, size);
    while (pObjectType[wrote - 1] == 0) wrote--;
    const Local<String> &str = String::NewFromTwoByte(info.GetIsolate(),
            (const uint16_t *) pObjectType,
            NewStringType::kNormal, wrote).ToLocalChecked();
    //std::wcerr << L"DEBUG GetObjectSimplifiedType CLR returned " << wrote << std::endl;
    if (wrote < 0) DEBUG_BREAK();
    info.GetReturnValue().Set(str);
    std::wstring_view svBuffer(pObjectType, wrote);
    //std::wcerr << L"DEBUG GetObjectSimplifiedType exited with string: "
    //        << svBuffer << std::endl;
}

void InstantiateBundleItemImpl(const FunctionCallbackInfo<Value> &info) {
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    std::wcerr << L"DEBUG InstantiateBundleItem argCount: " << argCount << std::endl;
    if (argCount != 1) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg0->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    std::wcerr << L"DEBUG InstantiateBundleItem arg0 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be a string", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto maybeUrl = arg0->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybeUrl, url);
    if (url.pBuffer == nullptr || url.nBuffer == 0) {
        std::wcerr << L"DEBUG InstantiateBundleItem exited with undef due to no path" << std::endl;
        return;
    }
    std::wcerr << L"DEBUG InstantiateBundleItem invoking CLR method" << std::endl;
    auto handle = DistantWorlds::Ide::InstantiateBundleItem(url.pBuffer, CheckedNarrowToInt(url.nBuffer));
    std::wcerr << L"DEBUG InstantiateBundleItem CLR returned" << std::endl;
    if (handle == 0) {
        std::wcerr << L"DEBUG InstantiateBundleItem exited with undef" << std::endl;
        return;
    }
    std::wcerr << L"DEBUG InstantiateBundleItem instance handle: 0x" << std::hex << handle << std::dec
            << std::endl;
    auto extHandle = External::New(isolate, reinterpret_cast<void *>(handle));
    info.GetReturnValue().Set(extHandle);
    std::wcerr << L"DEBUG InstantiateBundleItem exited with handle" << std::endl;
}

void IsImageImpl(const FunctionCallbackInfo<Value> &info) {
    if (info.Length() != 1) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 argument", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    if (!info[0]->IsExternal()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(info[0].As<External>()->Value());
    bool success = DistantWorlds::Ide::IsImage(handle);
    info.GetReturnValue().Set(success);
}

void TryConvertImageToBufferWebpImpl(const FunctionCallbackInfo<Value> &info) {
    if (info.Length() != 3) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 3 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    if (!info[0]->IsExternal()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(info[0].As<External>()->Value());
    //auto handle = dw2ide::ExtractHandle(info[0]);
    auto mipLevel = info[1]->Int32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
    auto arg2 = info[2];
    if (!arg2->IsArrayBufferView() && !arg2->IsArrayBuffer()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 3rd argument to be an ArrayBuffer or an ArrayBufferView", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto bufferData = arg2->IsArrayBufferView()
            ? arg2.As<ArrayBufferView>()->Buffer()
            : arg2.As<ArrayBuffer>();
    auto bufferLength = bufferData->ByteLength();
    bool success = DistantWorlds::Ide::TryConvertImageToBufferWebp(handle, mipLevel,
            reinterpret_cast<uint8_t *>(bufferData->Data()), bufferLength);
    info.GetReturnValue().Set(success);
}

void TryConvertImageToStreamWebpImpl(const FunctionCallbackInfo<Value> &info) {
    if (info.Length() != 3) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 3 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    if (!info[0]->IsExternal()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(info[0].As<External>()->Value());
    auto mipLevel = info[1]->Int32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
    auto arg2 = info[2];
    if (!arg2->IsFunction()) {
        auto *isolate = info.GetIsolate();
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 3rd argument to be a function", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto *isolate = info.GetIsolate();
    auto fn = arg2.As<Function>();
    auto success = DistantWorlds::Ide::TryConvertImageToStreamWebp(handle, mipLevel,
            [](void *pFn, unsigned char *buffer, size_t length) {
                auto *isolate = Isolate::GetCurrent();
                auto context = isolate->GetCurrentContext();
                auto bufferData = ArrayBuffer::New(isolate, length);
                memcpy(bufferData->Data(), buffer, length);
                auto uint8Array = Uint8Array::New(bufferData, 0, length);
                Local<Value> args[] = {
                        uint8Array,
                        Integer::New(isolate, length)
                };
                // NOTE: not local scope here
                // assume only called within outer function scope
                auto fn = (Function *) pFn;
                fn->Call(context, Null(isolate), 2, args).ToLocalChecked();
            }, *fn);
    info.GetReturnValue().Set(Integer::New(isolate, success));
}

void GetImageMipLevelsImpl(const FunctionCallbackInfo<Value> &info) {
    auto *isolate = info.GetIsolate();
    auto argCount = info.Length();
    if (argCount != 1) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    if (!arg0->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(arg0.As<External>()->Value());
    auto mipLevels = DistantWorlds::Ide::GetImageMipLevels(handle);
    if (mipLevels < 0) {
        std::wcerr << L"DEBUG GetImageMipLevels exited with undef" << std::endl;
        return;
    }
    info.GetReturnValue().Set(Integer::New(isolate, mipLevels));
    std::wcerr << L"DEBUG GetImageMipLevels exited with " << mipLevels << std::endl;
}

void GetImageDimensionsImpl(const FunctionCallbackInfo<Value> &info) {
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 1) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    if (!arg0->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(arg0.As<External>()->Value());
    auto dims = DistantWorlds::Ide::GetImageDimensions(handle);
    if (dims < 0) {
        std::wcerr << L"DEBUG GetImageDimensions exited with undef" << std::endl;
        return;
    }
    info.GetReturnValue().Set(Integer::New(isolate, dims));
    std::wcerr << L"DEBUG GetImageDimensions exited with " << dims << std::endl;
}

void GetImageWidthImpl(const FunctionCallbackInfo<Value> &info) {
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 2) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    if (!arg0->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg1 = info[1];
    if (!arg1->IsInt32()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2nd to be an integer", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(arg0.As<External>()->Value());
    auto mipLevel = arg1->Int32Value(context).ToChecked();
    auto width = DistantWorlds::Ide::GetImageWidth(handle, mipLevel);
    if (width < 0) {
        //std::wcerr << L"DEBUG GetImageWidth exited with undef" << std::endl;
        return;
    }
    info.GetReturnValue().Set(Integer::New(isolate, width));
    //std::wcerr << L"DEBUG GetImageWidth exited with " << width << std::endl;
}

void GetImageHeightImpl(const FunctionCallbackInfo<Value> &info) {
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 2) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    if (!arg0->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg1 = info[1];
    if (!arg1->IsInt32()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2nd to be an integer", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(arg0.As<External>()->Value());
    auto mipLevel = arg1->Int32Value(context).ToChecked();
    auto height = DistantWorlds::Ide::GetImageHeight(handle, mipLevel);
    if (height < 0) {
        //std::wcerr << L"DEBUG GetImageHeight exited with undef" << std::endl;
        return;
    }
    info.GetReturnValue().Set(Integer::New(isolate, height));
    //std::wcerr << L"DEBUG GetImageHeight exited with " << height << std::endl;
}

void GetImageDepthImpl(const FunctionCallbackInfo<Value> &info) {
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 2) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    if (!arg0->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg1 = info[1];
    if (!arg1->IsInt32()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2nd to be an integer", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(arg0.As<External>()->Value());
    auto mipLevel = arg1->Int32Value(context).ToChecked();
    auto depth = DistantWorlds::Ide::GetImageDepth(handle, mipLevel);
    if (depth < 0) {
        //std::wcerr << L"DEBUG GetImageDepth exited with undef" << std::endl;
        return;
    }
    info.GetReturnValue().Set(Integer::New(isolate, depth));
    //std::wcerr << L"DEBUG GetImageDepth exited with " << depth << std::endl;
}

void ShowMessageBoxImpl(const FunctionCallbackInfo<Value> &info) {
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 4) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 4 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg0->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG ShowMessageBox arg0 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be a string", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto maybeMessage = arg0->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybeMessage, message);
    if (message.pBuffer == nullptr || message.nBuffer == 0) {
        return;
    }
    auto arg1 = info[1];
    memset(typeNameBuffer, 0, 16);
    arg1->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG ShowMessageBox arg1 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2nd argument to be a string", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto maybeTitle = arg1->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybeTitle, title);
    if (title.pBuffer == nullptr || title.nBuffer == 0) {
        return;
    }
    auto arg2 = info[2];
    if (!arg2->IsInt32()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 3rd argument to be an integer", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto messageBoxButtons = static_cast<DistantWorlds::Ide::MessageBoxButtons_t>(arg2->Int32Value(
            context).ToChecked());
    auto arg3 = info[3];
    if (!arg3->IsInt32()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 4th argument to be an integer", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto messageBoxType = static_cast<DistantWorlds::Ide::MessageBoxType_t>(arg3->Int32Value(context).ToChecked());
    auto result = DistantWorlds::Ide::ShowMessageBox(message.pBuffer, CheckedNarrowToInt(message.nBuffer),
            title.pBuffer, CheckedNarrowToInt(title.nBuffer), messageBoxButtons, messageBoxType);
    info.GetReturnValue().Set(Integer::New(isolate, result));
}

void GetImageFormatImpl(const FunctionCallbackInfo<Value> &info) {
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 1) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    if (!arg0->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(arg0.As<External>()->Value());
    auto needed = DistantWorlds::Ide::GetImageFormat(handle, nullptr, 0);
    if (needed == 0) {
        //std::wcerr << L"DEBUG GetImageFormat exited with undef" << std::endl;
        return;
    }
    auto size = 1 + -needed;
    auto *pFormat = (wchar_t *) _alloca(size * sizeof(wchar_t));
    auto wrote = DistantWorlds::Ide::GetImageFormat(handle, pFormat, size);
    if (wrote < 0) DEBUG_BREAK();
    while (pFormat[wrote - 1] == 0) wrote--;
    const Local<String> &str = String::NewFromTwoByte(info.GetIsolate(),
            (const uint16_t *) pFormat,
            NewStringType::kNormal, wrote).ToLocalChecked();
    info.GetReturnValue().Set(str);
    //std::wcerr << L"DEBUG GetImageFormat exited with string: "
    //        << std::wstring_view(pFormat, wrote) << std::endl;
}

void GetImageTextureTypeImpl(const FunctionCallbackInfo<Value> &info) {
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 1) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    if (!arg0->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto handle = reinterpret_cast<intptr_t>(arg0.As<External>()->Value());
    auto needed = DistantWorlds::Ide::GetImageTextureType(handle, nullptr, 0);
    if (needed == 0) {
        //std::wcerr << L"DEBUG GetImageTextureType exited with undef" << std::endl;
        return;
    }
    auto size = 1 + -needed;
    auto *pTextureType = (wchar_t *) _alloca(size * sizeof(wchar_t));
    auto wrote = DistantWorlds::Ide::GetImageTextureType(handle, pTextureType, size);
    if (wrote < 0) DEBUG_BREAK();
    while (pTextureType[wrote - 1] == 0) wrote--;
    const Local<String> &str = String::NewFromTwoByte(info.GetIsolate(),
            (const uint16_t *) pTextureType,
            NewStringType::kNormal, wrote).ToLocalChecked();
    info.GetReturnValue().Set(str);
    //std::wcerr << L"DEBUG GetImageTextureType exited with string: "
    //        << std::wstring_view(pTextureType, wrote) << std::endl;
}

void TryExportObjectImpl(const FunctionCallbackInfo<Value> &info) {
    // arg0 is object id, arg1 is path
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 2) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    if (!arg0->IsArrayBufferView() && !arg0->IsArrayBuffer()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be an ArrayBuffer or an ArrayBufferView", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto objectIdData = arg0->IsArrayBufferView()
            ? arg0.As<ArrayBufferView>()->Buffer()
            : arg0.As<ArrayBuffer>();

    auto objectIdLength = objectIdData->ByteLength();
    if (objectIdLength != DistantWorlds::Ide::ObjectIdByteSize) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st argument to be ObjectIdByteSize bytes in length", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto arg1 = info[1];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg1->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG TryExportObject arg1 is a " << typeNameBuffer << std::endl;

    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2nd argument to be a string", NewStringType::kInternalized);
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
            path.pBuffer, CheckedNarrowToInt(path.nBuffer));

    info.GetReturnValue().Set(Boolean::New(isolate, success));


}

int CheckedNarrowToInt(size_t v) {
    if (v > INT32_MAX) {
        DEBUG_BREAK();
        return INT32_MAX;
    }
    return static_cast<int>(v);
}

void TryExportImageAsWebpImpl(const FunctionCallbackInfo<Value> &info) {
    // arg0 is handle, arg1 is path
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 2) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    if (!arg0->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto handle = reinterpret_cast<intptr_t>(arg0.As<External>()->Value());
    auto arg1 = info[1];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg1->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG TryExportImageAsWebp arg1 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2nd argument to be a string", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto maybePath = arg1->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybePath, path);
    if (path.pBuffer == nullptr || path.nBuffer == 0) {
        //std::wcerr << L"DEBUG TryExportImageAsWebp exited with undef due to no path" << std::endl;
        return;
    }

    auto success = DistantWorlds::Ide::TryExportImageAsWebp(handle, path.pBuffer,
            CheckedNarrowToInt(path.nBuffer));

    info.GetReturnValue().Set(Boolean::New(isolate, success));
}

void TryExportImageAsDdsImpl(const FunctionCallbackInfo<Value> &info) {
    // arg0 is handle, arg1 is path
    auto *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto argCount = info.Length();
    if (argCount != 2) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2 arguments", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }
    auto arg0 = info[0];
    if (!arg0->IsExternal()) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 1st to be a handle", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto handle = reinterpret_cast<intptr_t>(arg0.As<External>()->Value());
    auto arg1 = info[1];
    char typeNameBuffer[16];
    memset(typeNameBuffer, 0, 16);
    arg1->TypeOf(isolate)->WriteUtf8(isolate, typeNameBuffer, 32);
    //std::wcerr << L"DEBUG TryExportImageAsDds arg1 is a " << typeNameBuffer << std::endl;
    if (strncmp(typeNameBuffer, "string", 16) != 0) {
        const auto &kErr = String::NewFromUtf8Literal(isolate,
                "Expected 2nd argument to be a string", NewStringType::kInternalized);
        isolate->ThrowException(kErr);
        return;
    }

    auto maybePath = arg1->ToString(context);
    DW2IDE_EXTRACT_UTF16STRING(maybePath, path);
    if (path.pBuffer == nullptr || path.nBuffer == 0) {
        //std::wcerr << L"DEBUG TryExportImageAsDds exited with undef due to no path" << std::endl;
        return;
    }

    auto success = DistantWorlds::Ide::TryExportImageAsDds(handle, path.pBuffer,
            CheckedNarrowToInt(path.nBuffer));

    info.GetReturnValue().Set(Boolean::New(isolate, success));
}

void dw2ide::NODE_MODULE_INITIALIZER_X(NODE_MODULE_INITIALIZER_BASE, 118)
        (Local<Object> exports, Local<Value> module,
                Local<Context> context) {
    Isolate *isolate = context->GetIsolate();
    auto *ctx = new Dw2IdeContext(isolate);
    Local<External> ctxExternal = External::New(isolate, ctx);

    exports->Set(context, String::NewFromUtf8Literal(isolate, "ObjectIdByteSize", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::ObjectIdByteSize)).FromJust();

    exports->Set(context, String::NewFromUtf8Literal(isolate, "ObjectIdStringLength", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::ObjectIdStringLength)).FromJust();

    const auto &kInitialize = String::NewFromUtf8Literal(isolate, "Initialize", NewStringType::kInternalized);
    exports->Set(context, kInitialize, FunctionTemplate::New(isolate, [](const FunctionCallbackInfo<Value> &info) {
        auto *ctx = Dw2IdeContext::FromFunctionCallbackInfo(info);
        DistantWorlds::Ide::Initialize();
    }, ctxExternal)->GetFunction(context).ToLocalChecked()).FromJust();

    // int32_t DNNE_CALLTYPE GetVersion(DNNE_WCHAR* pBuffer, int32_t bufferSize);
    DW2IDE_EXPORT_FUNCTION_GET_STRING(GetVersion);

    // int32_t DNNE_CALLTYPE GetNetVersion(DNNE_WCHAR* pBuffer, int32_t bufferSize);
    DW2IDE_EXPORT_FUNCTION_GET_STRING(GetNetVersion);

    // int32_t DNNE_CALLTYPE GetGameDirectory(DNNE_WCHAR* pBuffer, int32_t bufferSize);
    DW2IDE_EXPORT_FUNCTION_GET_STRING(GetGameDirectory);

    // int32_t DNNE_CALLTYPE GetUserChosenGameDirectory(DNNE_WCHAR* pBuffer, int32_t bufferSize);
    DW2IDE_EXPORT_FUNCTION_GET_STRING(GetUserChosenGameDirectory);

    // void DNNE_CALLTYPE Deisolate(void);
    DW2IDE_EXPORT_FUNCTION_IMPL(Deisolate);

    DW2IDE_EXPORT_FUNCTION_IMPL(DebugBreak);

    DW2IDE_EXPORT_FUNCTION_IMPL(ClrDebugBreak);

    // void DNNE_CALLTYPE ReleaseHandle(intptr_t handle);
    DW2IDE_EXPORT_FUNCTION_IMPL(ReleaseHandle);

    // bool DNNE_CALLTYPE HandleToString(intptr_t handle, DNNE_WCHAR* buffer, int32_t bufferLength);
    DW2IDE_EXPORT_FUNCTION_IMPL(HandleToString);

    // create object template for bundle handles

    const Local<ObjectTemplate> managedHandleTemplate = ObjectTemplate::New(isolate);
    managedHandleTemplate->SetInternalFieldCount(2);
    //managedHandleTemplate->MarkAsUndetectable();

    ctx->ManagedHandleTemplate = Global<ObjectTemplate>(isolate, managedHandleTemplate);

    // intptr_t DNNE_CALLTYPE LoadBundle(DNNE_WCHAR* path, int32_t pathLength);
    DW2IDE_EXPORT_FUNCTION_IMPL(LoadBundle);

//intptr_t DNNE_CALLTYPE QueryBundleObjects(intptr_t bundleHandle, DNNE_WCHAR* glob, int32_t globLength);
    DW2IDE_EXPORT_FUNCTION_IMPL(QueryBundleObjects);

// intptr_t DNNE_CALLTYPE ReadQueriedBundleObject(intptr_t bundleQueryHandle, DNNE_WCHAR* buffer, int32_t bufferLength);
    DW2IDE_EXPORT_FUNCTION_IMPL(ReadQueriedBundleObject);

// bool DNNE_CALLTYPE TryGetObjectId(DNNE_WCHAR* path, int32_t pathLength, uint8_t* pObjectId);
    DW2IDE_EXPORT_FUNCTION_IMPL(TryGetObjectId);

// NOTE: pSourceDirOrBundlePath is optional output, not input
// bool DNNE_CALLTYPE TryGetObjectOffset(uint8_t* pObjectId, int64_t* pOffset, int64_t* pOffsetEnd, DNNE_WCHAR* pSourceDirOrBundlePath, int32_t sourceDirOrBundlePathLength);
    DW2IDE_EXPORT_FUNCTION_IMPL(GetObjectOffset);

// bool DNNE_CALLTYPE TryGetObjectSize(uint8_t* pObjectId, int64_t* pSize);
    DW2IDE_EXPORT_FUNCTION_IMPL(GetObjectSize);

// int32_t DNNE_CALLTYPE GetObjectType(uint8_t* pObjectId, DNNE_WCHAR* pObjectType, int32_t* pObjectTypeLength);
    DW2IDE_EXPORT_FUNCTION_IMPL(GetObjectType);

// int32_t DNNE_CALLTYPE GetObjectSimplifiedType(uint8_t* pObjectId, DNNE_WCHAR* pSimpleType, int32_t* pSimpleTypeLength);
    DW2IDE_EXPORT_FUNCTION_IMPL(GetObjectSimplifiedType);

// intptr_t DNNE_CALLTYPE InstantiateBundleItem(DNNE_WCHAR* url, int32_t urlLength);
    DW2IDE_EXPORT_FUNCTION_IMPL(InstantiateBundleItem);

// bool DNNE_CALLTYPE IsImage(intptr_t handle);
    DW2IDE_EXPORT_FUNCTION_IMPL(IsImage);

// bool DNNE_CALLTYPE TryConvertImageToBufferWebp(intptr_t handle, int32_t mipLevel, uint8_t* pBuffer, int32_t bufferSize);
    DW2IDE_EXPORT_FUNCTION_IMPL(TryConvertImageToBufferWebp);

// typedef void(* StreamWrite_t)(void* state, unsigned char* buffer, size_t length);
// bool DNNE_CALLTYPE TryConvertImageToStreamWebp(intptr_t handle, int32_t mipLevel, StreamWrite_t pWriteFn, void* state);
    DW2IDE_EXPORT_FUNCTION_IMPL(TryConvertImageToStreamWebp);

    DW2IDE_EXPORT_FUNCTION_IMPL(GetImageMipLevels);
    DW2IDE_EXPORT_FUNCTION_IMPL(GetImageWidth);
    DW2IDE_EXPORT_FUNCTION_IMPL(GetImageHeight);
    DW2IDE_EXPORT_FUNCTION_IMPL(GetImageDepth);
    DW2IDE_EXPORT_FUNCTION_IMPL(GetImageDimensions);
    DW2IDE_EXPORT_FUNCTION_IMPL(GetImageFormat);
    DW2IDE_EXPORT_FUNCTION_IMPL(GetImageTextureType);

    // DialogResult_t DNNE_CALLTYPE ShowMessageBox(DNNE_WCHAR* message, int32_t messageLength, DNNE_WCHAR* title, int32_t titleLength, MessageBoxButtons_t messageBoxButtons, MessageBoxType_t messageBoxType)
    DW2IDE_EXPORT_FUNCTION_IMPL(ShowMessageBox);

    DW2IDE_EXPORT_FUNCTION_IMPL(TryExportObject);
    DW2IDE_EXPORT_FUNCTION_IMPL(TryExportImageAsWebp);
    DW2IDE_EXPORT_FUNCTION_IMPL(TryExportImageAsDds);

    const auto &kMessageBoxButtons = String::NewFromUtf8Literal(isolate, "MessageBoxButtons",
            NewStringType::kInternalized); \
    const auto &oMessageBoxButtons = Object::New(isolate);
    oMessageBoxButtons->Set(context, String::NewFromUtf8Literal(isolate, "OK", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::MessageBoxButtons::OK)).FromJust();
    oMessageBoxButtons->Set(context, String::NewFromUtf8Literal(isolate, "OKCancel", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::MessageBoxButtons::OKCancel)).FromJust();
    oMessageBoxButtons->Set(context, String::NewFromUtf8Literal(isolate, "YesNo", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::MessageBoxButtons::YesNo)).FromJust();
    oMessageBoxButtons->Set(context, String::NewFromUtf8Literal(isolate, "YesNoCancel", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::MessageBoxButtons::YesNoCancel)).FromJust();
    exports->Set(context, kMessageBoxButtons, oMessageBoxButtons).FromJust();

    const auto &kMessageBoxType = String::NewFromUtf8Literal(isolate, "MessageBoxType", NewStringType::kInternalized); \
    const auto &oMessageBoxType = Object::New(isolate);
    oMessageBoxType->Set(context, String::NewFromUtf8Literal(isolate, "Information", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::MessageBoxType::Information)).FromJust();
    oMessageBoxType->Set(context, String::NewFromUtf8Literal(isolate, "Warning", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::MessageBoxType::Warning)).FromJust();
    oMessageBoxType->Set(context, String::NewFromUtf8Literal(isolate, "Error", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::MessageBoxType::Error)).FromJust();
    oMessageBoxType->Set(context, String::NewFromUtf8Literal(isolate, "Question", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::MessageBoxType::Question)).FromJust();
    exports->Set(context, kMessageBoxType, oMessageBoxType).FromJust();

    const auto &kDialogResult = String::NewFromUtf8Literal(isolate, "DialogResult", NewStringType::kInternalized); \
    const auto &oDialogResult = Object::New(isolate);
    oDialogResult->Set(context, String::NewFromUtf8Literal(isolate, "None", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::DialogResult::None)).FromJust();
    oDialogResult->Set(context, String::NewFromUtf8Literal(isolate, "Ok", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::DialogResult::Ok)).FromJust();
    oDialogResult->Set(context, String::NewFromUtf8Literal(isolate, "Cancel", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::DialogResult::Cancel)).FromJust();
    oDialogResult->Set(context, String::NewFromUtf8Literal(isolate, "Yes", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::DialogResult::Yes)).FromJust();
    oDialogResult->Set(context, String::NewFromUtf8Literal(isolate, "No", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::DialogResult::No)).FromJust();
    oDialogResult->Set(context, String::NewFromUtf8Literal(isolate, "Abort", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::DialogResult::Abort)).FromJust();
    oDialogResult->Set(context, String::NewFromUtf8Literal(isolate, "Ignore", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::DialogResult::Ignore)).FromJust();
    oDialogResult->Set(context, String::NewFromUtf8Literal(isolate, "Retry", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::DialogResult::Retry)).FromJust();
    exports->Set(context, kDialogResult, oDialogResult).FromJust();


}

