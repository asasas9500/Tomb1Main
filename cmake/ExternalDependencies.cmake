cmake_minimum_required(VERSION 3.11)

include(FindUnixCommands)

if(BASH)
    file(TO_NATIVE_PATH "${BASH}" NATIVE_BASH)
    get_filename_component(SCRIPT_MODE_DIR "${CMAKE_SCRIPT_MODE_FILE}" DIRECTORY)
    file(TO_NATIVE_PATH "${SCRIPT_MODE_DIR}" NATIVE_SCRIPT_MODE_DIR)

    execute_process(
        COMMAND "${NATIVE_BASH}" -l -- "${NATIVE_SCRIPT_MODE_DIR}\\build.sh"
    )
endif()
