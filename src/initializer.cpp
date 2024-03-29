#include "pch.h"
#include "initializer.h"
#include "impl.h"

using namespace dw2ide;
using namespace v8;


void initializer(Local<Object> exports, Local<Value> module, Local<Context> context, int nodeApiVersion) {

    Isolate *isolate = context->GetIsolate();
    auto *ctx = new Dw2IdeContext(isolate, exports);
    auto sym = ctx->IsolationContextIdSymbol.Get(isolate);
    auto isoCtxId = DistantWorlds::Ide::GetIsolationContextId();
    exports->Set(context, sym, Integer::New(isolate, isoCtxId)).FromJust();

    Local<External> ctxExternal = External::New(isolate, ctx);

    exports->Set(context, String::NewFromUtf8Literal(isolate, "ObjectIdByteSize", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::ObjectIdByteSize)).FromJust();

    exports->Set(context, String::NewFromUtf8Literal(isolate, "ObjectIdStringLength", NewStringType::kInternalized),
            Integer::New(isolate, DistantWorlds::Ide::ObjectIdStringLength)).FromJust();


    const auto &kInitialize = String::NewFromUtf8Literal(isolate, "Initialize", NewStringType::kInternalized);
    exports->Set(context, kInitialize, FunctionTemplate::New(isolate, [](const FunctionCallbackInfo<Value> &info) {
        DistantWorlds::Ide::Initialize();
    }, ctxExternal)->GetFunction(context).ToLocalChecked()).FromJust();


#ifdef TRACE_EXPORTS
    std::wcerr << L"DEBUG dw2ide registering exports" << std::endl;
#endif

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

    DW2IDE_EXPORT_FUNCTION_IMPL(NewIsolationContext);

    DW2IDE_EXPORT_FUNCTION_IMPL(DebugBreak);

    DW2IDE_EXPORT_FUNCTION_IMPL(ClrDebugBreak);

    // void DNNE_CALLTYPE ReleaseHandle(intptr_t handle);
    DW2IDE_EXPORT_FUNCTION_IMPL(ReleaseHandle);

    // bool DNNE_CALLTYPE HandleToString(intptr_t handle, DNNE_WCHAR* buffer, int32_t bufferLength);
    DW2IDE_EXPORT_FUNCTION_IMPL(HandleToString);

    // create object template for bundle handles

    //const Local<ObjectTemplate> managedHandleTemplate = ObjectTemplate::New(isolate);
    //managedHandleTemplate->SetInternalFieldCount(2);
    //managedHandleTemplate->MarkAsUndetectable();

    //ctx->ManagedHandleTemplate = Global<ObjectTemplate>(isolate, managedHandleTemplate);

    DW2IDE_EXPORT_FUNCTION_IMPL(GetLastException);

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

#ifdef TRACE_EXPORTS
    std::wcerr << L"DEBUG dw2ide registering MessageBoxButtons" << std::endl;
#endif

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

#ifdef TRACE_EXPORTS
    std::wcerr << L"DEBUG dw2ide registering MessageBoxType" << std::endl;
#endif

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

#ifdef TRACE_EXPORTS
    std::wcerr << L"DEBUG dw2ide registering DialogResult" << std::endl;
#endif

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

#ifdef TRACE_EXPORTS
    std::wcerr << L"DEBUG dw2ide export registration complete" << std::endl;
#endif

}

