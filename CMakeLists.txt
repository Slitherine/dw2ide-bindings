cmake_minimum_required(VERSION 3.26)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0135 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# force build directory to be in a known location for cmake.js
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}/build>)

include(InstallRequiredSystemLibraries)

project(dw2ide_runtime)

include(CMakeJS.cmake)
setup_cmake_js()
if (NOT CMAKE_JS_VERSION)
    message(FATAL_ERROR "CMake.js is missing or not set up.")
endif ()

include_directories(${CMAKE_JS_INC})

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS LIST_DIRECTORIES false
        "src/*.cxx"
        "src/*.cpp"
        "src/*.c"
        "src/*.hpp"
        "src/*.h"
        "include/*.hpp"
        "include/*.h")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC include)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "include/dw2ide_runtime.h")

# NODE_MODULE_VERSION might need to be set manually
set_target_properties(${PROJECT_NAME} PROPERTIES
        NODE_MODULE_VERSION 120
        COMPILE_WARNING_AS_ERROR ON)


if (MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
    # touch these files to make sure they exist
    file(TOUCH ${CMAKE_JS_NODELIB_DEF})
    file(TOUCH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/node-lib.exp)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS}
            COMMENT "Generating node.lib")
endif ()

# path to the DistantWorlds.IDE c# project
set(DW_IDE_CS_PATH "" CACHE PATH "Path to the DistantWorlds.IDE project.")
if (NOT DW_IDE_CS_PATH)
    # check for dir for it in parent dir
    get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
    set(DW_IDE_CS_PATH "${PARENT_DIR}/DistantWorlds.IDE")
    if (NOT EXISTS ${DW_IDE_CS_PATH}/DistantWorlds.IDE.sln)
        set(DW_IDE_CS_PATH "")
    endif ()
endif ()
if (NOT DW_IDE_CS_PATH)
    message(FATAL_ERROR "Path to the DistantWorlds.IDE project is not set.")
endif ()

set(DW_IDE_CS_FWTARGET "net8.0" CACHE PATH "Target framework for DistantWorlds.IDE.")
set(DW_IDE_CS_PLATFORM "win-x64" CACHE PATH "Target platform for DistantWorlds.IDE.")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(DW_IDE_CS_CONFIG "Release" CACHE PATH "Configuration for DistantWorlds.IDE.")
else ()
    set(DW_IDE_CS_CONFIG "Debug" CACHE PATH "Configuration for DistantWorlds.IDE.")
endif ()

set(DW_IDE_CS_OUTDIR "${PARENT_DIR}/DistantWorlds.IDE/bin/${DW_IDE_CS_CONFIG}/${DW_IDE_CS_FWTARGET}/${DW_IDE_CS_PLATFORM}")

if (NOT EXISTS "${DW_IDE_CS_OUTDIR}/DistantWorlds.IDE.Interop.dll")
    # build DistantWorlds.IDE
    message(STATUS "Building DistantWorlds.IDE...")
    execute_process(COMMAND dotnet build
            -c ${DW_IDE_CS_CONFIG} -f ${DW_IDE_CS_FWTARGET} -r ${DW_IDE_CS_PLATFORM} # configuration
            -p:SelfContained=true # include runtime
            -p:CopyLocalLockFileAssemblies=true # all dependencies
            #"-p:OutDir=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            ${DW_IDE_CS_PATH}/DistantWorlds.IDE.csproj
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE DW_IDE_CS_BUILD_RESULT
    )
    if (NOT DW_IDE_CS_BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to build DistantWorlds.IDE!")
    else ()
        message(STATUS "DistantWorlds.IDE build output: ${DW_IDE_CS_BUILD_OUTPUT}")
    endif ()
endif ()

# DNNE will put DistantWorlds.IDE.Interop.dll, DistantWorlds.IDE.dll and DistantWorlds.IDE.runtimeconfig.json
# DistantWorlds.IDE.Interop.h and DistantWorlds.IDE.Interop.lib into the output dir, add to the target
add_library(DistantWorlds.IDE SHARED IMPORTED)
set_target_properties(DistantWorlds.IDE PROPERTIES
        IMPORTED_IMPLIB "${DW_IDE_CS_OUTDIR}/DistantWorlds.IDE.Interop.lib"
        IMPORTED_LOCATION "${DW_IDE_CS_OUTDIR}/DistantWorlds.IDE.Interop.dll"
        PUBLIC_HEADER "${DW_IDE_CS_OUTDIR}/DistantWorlds.IDE.Interop.h"
        INTERFACE_INCLUDE_DIRECTORIES "${DW_IDE_CS_OUTDIR}"
)
# add all extra files in the output dir
file(GLOB_RECURSE DW_IDE_CS_EXTRA_FILES CONFIGURE_DEPENDS LIST_DIRECTORIES false
        "${DW_IDE_CS_OUTDIR}/**")
# remove the generated headers
list(FILTER DW_IDE_CS_EXTRA_FILES EXCLUDE REGEX ".*\.h$")


# copy all extra files to the output dir
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DW_IDE_CS_EXTRA_FILES}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying additional dependencies from DistantWorlds.IDE..."
)


# strip leading path from absolute to make relative and then add to ADDITIONAL_CLEAN_FILES
set(DW_IDE_CS_EXTRA_FILES_RELATIVE "")
foreach (file ${DW_IDE_CS_EXTRA_FILES})
    file(RELATIVE_PATH rel_file ${DW_IDE_CS_OUTDIR} ${file})
    list(APPEND DW_IDE_CS_EXTRA_FILES_RELATIVE ${rel_file})
endforeach ()
#message(STATUS "DistantWorlds.IDE extra files: ${DW_IDE_CS_EXTRA_FILES_RELATIVE}")
# DistantWorlds.IDE extra files: AtkSharp.dll;BCnEncoder.dll;BCnEncoder.xml;CairoSharp.dll;DistantWorlds.IDE.deps.json;DistantWorlds.IDE.dll;DistantWorlds.IDE.pdb;DistantWorlds.IDE.runtimeconfig.json;Eto.Gtk.dll;Eto.dll;Eto.xml;FluentResults.dll;FluentResults.xml;GLibSharp.dll;GameFinder.Common.dll;GameFinder.Common.xml;GameFinder.RegistryUtils.dll;GameFinder.RegistryUtils.xml;GameFinder.StoreHandlers.EADesktop.dll;GameFinder.StoreHandlers.EADesktop.xml;GameFinder.StoreHandlers.EGS.dll;GameFinder.StoreHandlers.EGS.xml;GameFinder.StoreHandlers.GOG.dll;GameFinder.StoreHandlers.GOG.xml;GameFinder.StoreHandlers.Origin.dll;GameFinder.StoreHandlers.Origin.xml;GameFinder.StoreHandlers.Steam.dll;GameFinder.StoreHandlers.Steam.xml;GameFinder.StoreHandlers.Xbox.dll;GameFinder.StoreHandlers.Xbox.xml;GameFinder.Wine.dll;GameFinder.Wine.xml;GdkSharp.dll;GioSharp.dll;GtkSharp.dll;JetBrains.Annotations.dll;JetBrains.Annotations.xml;K4os.Compression.LZ4.Legacy.dll;K4os.Compression.LZ4.Legacy.xml;K4os.Compression.LZ4.dll;K4os.Compression.LZ4.xml;MediatR.dll;MediatR.xml;Microsoft.Bcl.AsyncInterfaces.dll;Microsoft.Bcl.AsyncInterfaces.xml;Microsoft.DotNet.PlatformAbstractions.dll;Microsoft.DotNet.PlatformAbstractions.xml;Microsoft.Extensions.Configuration.Abstractions.dll;Microsoft.Extensions.Configuration.Abstractions.xml;Microsoft.Extensions.Configuration.Binder.dll;Microsoft.Extensions.Configuration.Binder.xml;Microsoft.Extensions.Configuration.dll;Microsoft.Extensions.Configuration.xml;Microsoft.Extensions.DependencyInjection.Abstractions.dll;Microsoft.Extensions.DependencyInjection.Abstractions.xml;Microsoft.Extensions.DependencyInjection.dll;Microsoft.Extensions.DependencyInjection.xml;Microsoft.Extensions.DependencyModel.dll;Microsoft.Extensions.DependencyModel.xml;Microsoft.Extensions.Logging.Abstractions.dll;Microsoft.Extensions.Logging.Abstractions.xml;Microsoft.Extensions.Logging.Configuration.dll;Microsoft.Extensions.Logging.Configuration.xml;Microsoft.Extensions.Logging.Console.dll;Microsoft.Extensions.Logging.Console.xml;Microsoft.Extensions.Logging.dll;Microsoft.Extensions.Logging.xml;Microsoft.Extensions.Options.ConfigurationExtensions.dll;Microsoft.Extensions.Options.ConfigurationExtensions.xml;Microsoft.Extensions.Options.dll;Microsoft.Extensions.Options.xml;Microsoft.Extensions.Primitives.dll;Microsoft.Extensions.Primitives.xml;Microsoft.Toolkit.HighPerformance.dll;Microsoft.Toolkit.HighPerformance.pdb;Microsoft.Toolkit.HighPerformance.xml;Microsoft.VisualStudio.Threading.dll;Microsoft.VisualStudio.Threading.xml;Microsoft.VisualStudio.Validation.dll;Microsoft.VisualStudio.Validation.xml;Microsoft.Win32.SystemEvents.dll;Microsoft.Win32.SystemEvents.xml;Nerdbank.Streams.dll;Nerdbank.Streams.xml;Newtonsoft.Json.dll;Newtonsoft.Json.xml;NexusMods.Paths.dll;NexusMods.Paths.xml;OmniSharp.Extensions.JsonRpc.dll;OmniSharp.Extensions.JsonRpc.pdb;OmniSharp.Extensions.LanguageClient.dll;OmniSharp.Extensions.LanguageClient.pdb;OmniSharp.Extensions.LanguageProtocol.dll;OmniSharp.Extensions.LanguageProtocol.pdb;OmniSharp.Extensions.LanguageServer.Shared.dll;OmniSharp.Extensions.LanguageServer.Shared.pdb;OmniSharp.Extensions.LanguageServer.dll;OmniSharp.Extensions.LanguageServer.pdb;OneOf.dll;PangoSharp.dll;SDL2.dll;SHA3.Net.dll;ServiceWire.dll;SharpDX.D3DCompiler.dll;SharpDX.D3DCompiler.pdb;SharpDX.D3DCompiler.xml;SharpDX.DXGI.dll;SharpDX.DXGI.pdb;SharpDX.DXGI.xml;SharpDX.Direct3D11.dll;SharpDX.Direct3D11.pdb;SharpDX.Direct3D11.xml;SharpDX.Direct3D12.dll;SharpDX.Direct3D12.pdb;SharpDX.Direct3D12.xml;SharpDX.dll;SharpDX.pdb;SharpFont.dll;Silk.NET.Core.dll;Silk.NET.Core.xml;Silk.NET.Maths.dll;Silk.NET.Maths.xml;Silk.NET.OpenGL.dll;Silk.NET.OpenGL.xml;Silk.NET.OpenGLES.Extensions.EXT.dll;Silk.NET.OpenGLES.Extensions.EXT.xml;Silk.NET.OpenGLES.dll;Silk.NET.OpenGLES.xml;Silk.NET.SDL.dll;Silk.NET.SDL.xml;SixLabors.ImageSharp.dll;SixLabors.ImageSharp.xml;Stride.Core.IO.dll;Stride.Core.IO.pdb;Stride.Core.IO.xml;Stride.Core.Mathematics.dll;Stride.Core.Mathematics.pdb;Stride.Core.Mathematics.xml;Stride.Core.MicroThreading.dll;Stride.Core.MicroThreading.pdb;Stride.Core.MicroThreading.xml;Stride.Core.Serialization.dll;Stride.Core.Serialization.pdb;Stride.Core.Serialization.xml;Stride.Core.dll;Stride.Core.pdb;Stride.Core.xml;Stride.Graphics.dll;Stride.Graphics.xml;Stride.Native.dll;Stride.Native.pdb;Stride.Native.xml;Stride.Shaders.dll;Stride.Shaders.pdb;Stride.Shaders.xml;Stride.dll;Stride.pdb;Stride.xml;System.CodeDom.dll;System.CodeDom.xml;System.Drawing.Common.dll;System.Drawing.Common.xml;System.IO.Pipelines.dll;System.IO.Pipelines.xml;System.Management.dll;System.Management.xml;System.Reactive.dll;System.Reactive.xml;ValveKeyValue.dll;Vogen.SharedTypes.dll;Vortice.Mathematics.dll;Vortice.Mathematics.xml;Vortice.Vulkan.dll;Vortice.Vulkan.xml;cs/Microsoft.VisualStudio.Threading.resources.dll;cs/Microsoft.VisualStudio.Validation.resources.dll;de/Microsoft.VisualStudio.Threading.resources.dll;de/Microsoft.VisualStudio.Validation.resources.dll;es/Microsoft.VisualStudio.Threading.resources.dll;es/Microsoft.VisualStudio.Validation.resources.dll;fr/Microsoft.VisualStudio.Threading.resources.dll;fr/Microsoft.VisualStudio.Validation.resources.dll;freetype.dll;it/Microsoft.VisualStudio.Threading.resources.dll;it/Microsoft.VisualStudio.Validation.resources.dll;ja/Microsoft.VisualStudio.Threading.resources.dll;ja/Microsoft.VisualStudio.Validation.resources.dll;ko/Microsoft.VisualStudio.Threading.resources.dll;ko/Microsoft.VisualStudio.Validation.resources.dll;libstride.dll;pl/Microsoft.VisualStudio.Threading.resources.dll;pl/Microsoft.VisualStudio.Validation.resources.dll;pt-BR/Microsoft.VisualStudio.Threading.resources.dll;pt-BR/Microsoft.VisualStudio.Validation.resources.dll;ru/Microsoft.VisualStudio.Threading.resources.dll;ru/Microsoft.VisualStudio.Validation.resources.dll;tr/Microsoft.VisualStudio.Threading.resources.dll;tr/Microsoft.VisualStudio.Validation.resources.dll;zh-Hans/Microsoft.VisualStudio.Threading.resources.dll;zh-Hans/Microsoft.VisualStudio.Validation.resources.dll;zh-Hant/Microsoft.VisualStudio.Threading.resources.dll;zh-Hant/Microsoft.VisualStudio.Validation.resources.dll
set_target_properties(DistantWorlds.IDE PROPERTIES
        ADDITIONAL_CLEAN_FILES "${DW_IDE_CS_EXTRA_FILES_RELATIVE}"
)

# add library to the target
target_link_libraries(${PROJECT_NAME} DistantWorlds.IDE)

option(dw2ide_runtime_SKIP_TESTS "Skip building and running the tests." OFF)
if (dw2ide_runtime_SKIP_TESTS)
    MESSAGE(WARNING "Tests skipped!")
else ()
    enable_testing()
    add_subdirectory(tests)
endif ()