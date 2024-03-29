#include "../pch.h"

using namespace dw2ide;
using namespace v8;

void DebugBreakImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG DebugBreakImpl entered" << std::endl;
#endif
    DEBUG_BREAK();
}

void ClrDebugBreakImpl(const v8::FunctionCallbackInfo<v8::Value> &info) {
#if TRACE_ENTER
    std::wcerr << L"DEBUG ClrDebugBreakImpl entered" << std::endl;
#endif
    DistantWorlds::Ide::DebugBreak();
}
