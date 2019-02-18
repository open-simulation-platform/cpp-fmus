cmake_minimum_required(VERSION 3.0)
project(cpp-fmus)

option(FMU_USE_STATIC_RUNTIME
    "Whether to link the model code against a static C(++) runtime library"
    ON)
set(FMU_BUILD_DIR "${CMAKE_BINARY_DIR}/fmu-build"
    CACHE PATH "Where to put the generated FMU contents before zipping")
set(FMU_UUID_DIR "${CMAKE_BINARY_DIR}/fmu-uuids"
    CACHE PATH "Where to put the headers containing FMU UUIDs")

# The names of the FMUs built in this project
set(fmus
    "cpu_grinder"
    "null"
)

# ==============================================================================
# Compiler/platform specific settings
# ==============================================================================

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options("-Wall" "-Wextra" "-Wpedantic" "-Werror")
    add_compile_options("-Wno-parentheses")
    if(FMU_USE_STATIC_RUNTIME)
        add_compile_options("-static")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/W4" "/WX")
    add_compile_options("/wd4996")
    add_definitions("-D_SCL_SECURE_NO_WARNINGS" "-D_CRT_SECURE_NO_WARNINGS")
    if(FMU_USE_STATIC_RUNTIME)
        foreach(lang "C" "CXX")
            string(REPLACE "/MD" "/MT" "CMAKE_${lang}_FLAGS" "${${CMAKE_${lang}_FLAGS}}")
            foreach(config ${CMAKE_CONFIGURATION_TYPES})
                string(TOUPPER "${config}" configUpper)
                set(flagVar "CMAKE_${lang}_FLAGS_${configUpper}")
                string(REPLACE "/MD" "/MT" ${flagVar} ${${flagVar}})
            endforeach()
        endforeach()
    endif()
endif()

if(UNIX)
    # Remove the "lib" prefix on the generated .so files
    set(CMAKE_SHARED_MODULE_PREFIX)
endif()

# ==============================================================================
# Build
# ==============================================================================

# List CPPFMU sources
file(GLOB cppfmuSources "${CMAKE_SOURCE_DIR}/cppfmu/*.?pp")

# Detect platform
if("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
    set(os "darwin")
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(os "linux")
elseif(WIN32)
    set(os "win")
else()
    message(FATAL_ERROR "Unknown or unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
math(EXPR wordSize 8*${CMAKE_SIZEOF_VOID_P})
set(fmuPlatform "${os}${wordSize}")

# Prepare names for FMU output directories.
if(CMAKE_CONFIGURATION_TYPES)
    set(fmuFullBuildDir "${FMU_BUILD_DIR}/$<CONFIGURATION>")
else()
    set(fmuFullBuildDir "${FMU_BUILD_DIR}")
endif()

# Create targets
set(fmuIncludeDirs
    "${CMAKE_SOURCE_DIR}/fmi"
    "${CMAKE_SOURCE_DIR}/cppfmu"
)
foreach(fmuName IN LISTS fmus)
    set(dlTarget "${fmuName}")
    set(uuidTarget "${fmuName}_uuid")
    set(fmuTarget "${fmuName}_fmu")
    set(outputFmu "${CMAKE_BINARY_DIR}/${fmuName}.fmu")
    set(uuidDir "${FMU_UUID_DIR}/${fmuName}")
    set(sourceDir "${CMAKE_SOURCE_DIR}/src/${fmuName}")
    set(outputDir "${fmuFullBuildDir}/${fmuName}")
    set(binariesDir "${outputDir}/binaries/${fmuPlatform}")
    set(sourceModelDescription "${sourceDir}/modelDescription.xml")
    set(sourceFiles "${sourceDir}/sources/fmu.cpp")

    # Target to generate UUID
    set(outputModelDescription "${uuidDir}/modelDescription.xml")
    set(outputUuidHeader "${uuidDir}/fmu-uuid.h")
    add_custom_command(
        OUTPUT "${outputModelDescription}" "${outputUuidHeader}"
        COMMAND
        "${CMAKE_COMMAND}" "-E" "make_directory" "${uuidDir}"
        COMMAND
            "${CMAKE_COMMAND}"
            "-DINPUT_MODEL_DESCRIPTION=${sourceModelDescription}"
            "-DINPUT_HEADER=${CMAKE_SOURCE_DIR}/cmake/fmu-uuid.h.in"
            "-DOUTPUT_MODEL_DESCRIPTION=${outputModelDescription}"
            "-DOUTPUT_HEADER=${outputUuidHeader}"
            "-DADDITIONAL_INPUT=${sourceFiles}"
            "-P" "${CMAKE_SOURCE_DIR}/cmake/GenerateFmuGuid.cmake"
        DEPENDS "${sourceModelDescription}"
        VERBATIM
    )
    add_custom_target(${uuidTarget}
        DEPENDS "${outputModelDescription}" "${outputUuidHeader}"
    )

    # Target to build dynamic library
    add_library(${dlTarget} MODULE
        ${sourceFiles}
        ${cppfmuSources}
        "${sourceModelDescription}"
    )
    add_dependencies(${dlTarget} ${uuidTarget})
    target_compile_definitions(${dlTarget} PRIVATE
        "MODEL_IDENTIFIER=${fmuName}")
    target_include_directories(${dlTarget} PRIVATE "${uuidDir}" ${fmuIncludeDirs})

    # Target to generate FMU contents
    set(ruleName "${fmuName}-contents")
    add_custom_command(OUTPUT "${outputFmu}"
        COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory" "${sourceDir}" "${outputDir}"
        COMMAND "${CMAKE_COMMAND}" "-E" "make_directory" "${binariesDir}"
        COMMAND "${CMAKE_COMMAND}" "-E" "copy" "$<TARGET_FILE:${dlTarget}>" "${binariesDir}/"
        COMMAND "${CMAKE_COMMAND}" "-E" "copy" "${outputModelDescription}" "${outputDir}/modelDescription.xml"
        COMMAND "${CMAKE_COMMAND}" "-E" "copy" "${outputUuidHeader}" "${outputDir}/sources/"
        COMMAND "${CMAKE_COMMAND}" "-DBASE_DIR=${outputDir}" "-DOUTPUT_FILE=${outputFmu}" "-P" "${CMAKE_SOURCE_DIR}/cmake/ZipAll.cmake"
        DEPENDS ${dlTarget} ${uuidTarget}
        VERBATIM)
    add_custom_target(${fmuTarget} ALL DEPENDS "${outputFmu}")
endforeach()

source_group("Metadata" REGULAR_EXPRESSION "modelDescription.xml")
