#include "pch.h"
#include "helpers.h"

using namespace dw2ide;
using namespace v8;

void DeisolateImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG DeisolateImpl entered" << std::endl;
#endif
    DistantWorlds::Ide::Deisolate();
}