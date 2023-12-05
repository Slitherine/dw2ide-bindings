function(setup_cmake_js)
    if (WIN32)
        # add delayimp.lib to link libraries
        link_libraries(delayimp)
    endif ()

    if (CMAKE_JS_VERSION)
        message(STATUS "[CMakeJS] CMake.js v${CMAKE_JS_VERSION} already set up.")
        return()
    endif ()

    find_program(NODE "node")
    if (NOT NODE)
        message(FATAL_ERROR "[CMakeJS] Node.js not found. This project requires Node.js and NPM.")
    endif ()

    find_program(NPM "npm")
    if (NPM)
        message(VERBOSE "[CMakeJS] NPM found.")
    else ()
        message(FATAL_ERROR "[CMakeJS] NPM not found. This project requires Node.js and NPM.")
    endif ()

    if (WIN32)
        set(NPM_COMMAND ${NPM}.cmd)
    else ()
        set(NPM_COMMAND ${NPM})
    endif ()

    find_program(CMAKEJS "cmake-js")
    if (CMAKEJS)
        message(VERBOSE "[CMakeJS] CMake.js found.")
    else ()
        message(ERROR "[CMakeJS] CMake.js not found, installing globally...")
        execute_process(COMMAND "${NPM_COMMAND}" install -g cmake-js
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE NPM_OUTPUT
                RESULT_VARIABLE NPM_RESULT
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
        if (NOT NPM_RESULT EQUAL 0 OR "${NPM_OUTPUT}" MATCHES "ERR!")
            message(FATAL_ERROR "[CMakeJS] Failed to install CMake.js")
        endif ()
        message(STATUS "CMake.js should now be installed.")
        message(VERBOSE ${NPM_OUTPUT})
    endif ()

    if (WIN32)
        set(CMAKEJS_CMD ${CMAKEJS}.cmd)
    else ()
        set(CMAKEJS_CMD ${CMAKEJS})
    endif ()
    #message(STATUS "[CMakeJS] CMake.js command: ${CMAKEJS_CMD}")

    execute_process(COMMAND "${CMAKEJS_CMD}" install
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            ERROR_QUIET)

    if (CMAKE_BUILD_TYPE MATCHES "[Dd]ebug")
        #message(STATUS "[CMakeJS] Getting debug configuration...")
        execute_process(COMMAND "${CMAKEJS_CMD}" --debug -o "${CMAKE_CURRENT_BINARY_DIR}" print-configure
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE CMAKE_JS_OUTPUT
                RESULT_VARIABLE CMAKE_JS_RESULT
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
    else ()
        #message(STATUS "[CMakeJS] Getting release configuration...")
        execute_process(COMMAND "${CMAKEJS_CMD}" -o "${CMAKE_CURRENT_BINARY_DIR}" print-configure
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE CMAKE_JS_OUTPUT
                RESULT_VARIABLE CMAKE_JS_RESULT
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
    endif ()

    if (NOT CMAKE_JS_RESULT EQUAL 0)
        message(FATAL_ERROR "[CMakeJS] Failed to get CMake.js configuration.")
    endif ()

    # trim any potential prefix to the json array
    string(REGEX MATCHALL "\\[.*\\]$" CMAKE_JS_OUTPUT "${CMAKE_JS_OUTPUT}")

    # reparse strings from single quotes to double quotes
    execute_process(COMMAND "${NODE}" -e "console.log(JSON.stringify([${CMAKE_JS_OUTPUT}]))"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE CMAKE_JS_OUTPUT
            RESULT_VARIABLE CMAKE_JS_RESULT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET)

    #message(STATUS "[CMakeJS] CMake.js output: ${CMAKE_JS_OUTPUT}")

    if (NOT CMAKE_JS_RESULT EQUAL 0)
        message(FATAL_ERROR "[CMakeJS] Failed to parse CMake.js configuration into JSON.")
    endif ()

    #message(STATUS "[CMakeJS] CMake.js JSON: ${CMAKE_JS_OUTPUT}")
    string(JSON CMAKE_JS_JSON_ARRAY_LENGTH LENGTH "${CMAKE_JS_OUTPUT}" 0)

    # blacklist some properties
    set(CMAKE_JS_VAR_BLACKLIST
            "CMAKE_BUILD_TYPE"
            "CMAKE_RUNTIME_OUTPUT_DIRECTORY"
            "CMAKE_JS_NODELIB_DEF"
            "CMAKE_JS_NODELIB_TARGET"
    )
    #message(STATUS "[CMakeJS] Blacklisted: ${CMAKE_JS_VAR_BLACKLIST}")


    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "" FORCE)
    set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" CACHE INTERNAL "" FORCE)

    if (CMAKE_JS_OUTPUT MATCHES "-DCMAKE_JS_NODELIB_DEF=")
        # node_addon_api is used
        list(APPEND CMAKE_JS_VAR_BLACKLIST "CMAKE_JS_LIB")
        set(CMAKE_JS_LIB "${CMAKE_CURRENT_BINARY_DIR}/node.lib" CACHE INTERNAL "CMAKE_JS_LIB" FORCE)
        cmake_path(SET CMAKE_JS_LIB NORMALIZE "${CMAKE_JS_LIB}")
        set(CMAKE_JS_NODELIB_DEF "${CMAKE_CURRENT_BINARY_DIR}/node-lib.def" CACHE INTERNAL "CMAKE_JS_NODELIB_DEF" FORCE)
        cmake_path(SET CMAKE_JS_NODELIB_DEF NORMALIZE "${CMAKE_JS_NODELIB_DEF}")
        set(CMAKE_JS_NODELIB_TARGET "${CMAKE_CURRENT_BINARY_DIR}/node.lib" CACHE INTERNAL "CMAKE_JS_NODELIB_TARGET" FORCE)
        cmake_path(SET CMAKE_JS_NODELIB_TARGET NORMALIZE "${CMAKE_JS_NODELIB_TARGET}")
    endif ()

    set(CMAKE_JS_JSON_ARRAY_INDEX 0)
    while (CMAKE_JS_JSON_ARRAY_INDEX LESS CMAKE_JS_JSON_ARRAY_LENGTH)
        string(JSON CMAKE_JS_JSON_ARRAY_ITEM GET "${CMAKE_JS_OUTPUT}" 0 ${CMAKE_JS_JSON_ARRAY_INDEX})
        if (CMAKE_JS_JSON_ARRAY_ITEM MATCHES "^-D([^=]+)=(.*)")
            #string(REGEX MATCH "^-D([^=]+)=(.*)" CMAKE_JS_JSON_ARRAY_ITEM_MATCH "${CMAKE_JS_JSON_ARRAY_ITEM}")
            #if (NOT CMAKE_JS_JSON_ARRAY_ITEM_MATCH)
            #    message(FATAL_ERROR "[CMakeJS] Failed to parse CMake.js output: ${CMAKE_JS_JSON_ARRAY_ITEM}")
            #endif ()
            if ("${CMAKE_MATCH_1}" IN_LIST CMAKE_JS_VAR_BLACKLIST)
                message(STATUS "[CMakeJS] ${CMAKE_MATCH_1}=${CMAKE_MATCH_2} (skipped)")
                message(STATUS "[CMakeJS] ${CMAKE_MATCH_1}=${${CMAKE_MATCH_1}}")
            else ()
                cmake_path(IS_ABSOLUTE CMAKE_MATCH_2 VAR_IS_ABSOLUTE_PATH)
                if (VAR_IS_ABSOLUTE_PATH)
                    cmake_path(SET "${CMAKE_MATCH_1}" NORMALIZE "${CMAKE_MATCH_2}")
                endif ()
                set("${CMAKE_MATCH_1}" "${CMAKE_MATCH_2}" CACHE INTERNAL "${CMAKE_MATCH_1}" FORCE)
                message(STATUS "[CMakeJS] ${CMAKE_MATCH_1}=${${CMAKE_MATCH_1}}")
            endif ()
        endif ()
        math(EXPR CMAKE_JS_JSON_ARRAY_INDEX "${CMAKE_JS_JSON_ARRAY_INDEX} + 1")
    endwhile ()

    # verify CMAKE_JS_VERSION, CMAKE_JS_INC, CMAKE_JS_INC, CMAKE_JS_LIB are set
    if (NOT CMAKE_JS_VERSION OR NOT CMAKE_JS_INC OR NOT CMAKE_JS_SRC OR NOT CMAKE_JS_LIB)
        message(FATAL_ERROR "[CMakeJS] Failed to validate parsed CMake.js output.")
    endif ()

    message(STATUS "[CMakeJS] CMake.js v${CMAKE_JS_VERSION} set up complete.")
endfunction(setup_cmake_js)