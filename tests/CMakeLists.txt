cmake_minimum_required(VERSION 3.26)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0135 NEW)

set(MOCHA_JS ${CMAKE_SOURCE_DIR}/node_modules/mocha/bin/mocha.js)

set(ENV{NODE_OPTIONS} "--experimental-modules --experimental-vm-modules --experimental-specifier-resolution=node")
add_test(NAME mocha
        COMMAND ${NODE} ${MOCHA_JS} --recursive ${CMAKE_SOURCE_DIR}/tests
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
