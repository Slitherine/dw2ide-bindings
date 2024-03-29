#pragma once

#ifndef WIDEN
#define WIDEN_(x) L ## x
#define WIDEN(x) WIDEN_(x)
#endif
#ifndef STRINGIZE
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)
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

#if TRACE_ENTER
#define DW2IDE_EXPORT_FUNCTION_GET_STRING(name) \
    DW2IDE_EXPORT_FUNCTION(name, [](const FunctionCallbackInfo<Value> &info) { \
        std::wcerr << L"DEBUG " WIDEN(STRINGIZE(name)) " entered" << std::endl; \
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
#else
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
#endif


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
