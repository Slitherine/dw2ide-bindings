#pragma once

#include <iostream>
#include <excpt.h>
#include <sstream>
#include <iomanip>
#include <node.h>

#define TRACE_ENTER 1
#define TRACE_EXPORTS 1

#include <dw2ide-bindings.h>

#include "helper-macros.h"

static_assert(121 == NODE_MODULE_VERSION, "NODE_MODULE_VERSION mismatch: " STRINGIZE(NODE_MODULE_VERSION));
