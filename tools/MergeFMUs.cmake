# Merges two or more FMUs that contain the same model, but compiled for
# different platforms.
#
# The modelDescription.xml files of the different FMUs are compared to
# verify that the FMUs contain the same model. All other files are assumed
# to be the same if they have the same paths.
#
# Configuration variables:
#
#   INPUT_FMUS
#       A list of the FMUs that will be merged.
#
#   OUTPUT_FMU
#       The name of the output FMU.
#
cmake_minimum_required(VERSION 3.0)

function(unzip zipFile targetDir)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" "-E" "tar" "xf" "${zipFile}" "--format=zip"
        WORKING_DIRECTORY "${targetDir}"
        RESULT_VARIABLE exitCode
    )
    if(NOT exitCode EQUAL 0)
        message(FATAL_ERROR "Unzip command failed")
    endif()
endfunction()

function(zip zipFile sourceDir)
    file(GLOB inputFiles RELATIVE "${sourceDir}" "${sourceDir}/*")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" "-E" "tar" "cf" "${zipFile}" "--format=zip" ${inputFiles}
        WORKING_DIRECTORY "${sourceDir}"
        RESULT_VARIABLE exitCode
    )
    if(NOT exitCode EQUAL 0)
        message(FATAL_ERROR "Zip command failed")
    endif()
endfunction()

function(read_text_file filePath outputVar)
    file(READ "${filePath}" content)
    string(REGEX REPLACE "\r\n" "\n" unixContent "${content}")
    set("${outputVar}" "${unixContent}" PARENT_SCOPE)
endfunction()

function(make_absolute path outputVar)
    if(IS_ABSOLUTE "${path}")
        set(${outputVar} "${path}" PARENT_SCOPE)
    else()
        set(${outputVar} "${CMAKE_BINARY_DIR}/${path}" PARENT_SCOPE)
    endif()
endfunction()


# --- Script starts here ---

if(NOT INPUT_FMUS)
    message(FATAL_ERROR "No input FMUs specified")
endif()
if(NOT OUTPUT_FMU)
    message(FATAL_ERROR "No output FMU specified")
endif()

get_filename_component(fmuName "${OUTPUT_FMU}" NAME)
string(RANDOM random)
set(mergeDir "${CMAKE_BINARY_DIR}/${fmuName}.${random}")
file(MAKE_DIRECTORY "${mergeDir}")

set(oldFmu)
set(oldModelDescription)
foreach(fmu IN LISTS INPUT_FMUS)
    make_absolute("${fmu}" absPath)
    unzip("${absPath}" "${mergeDir}")
    read_text_file("${mergeDir}/modelDescription.xml" modelDescription)
    if(oldFmu AND NOT modelDescription STREQUAL oldModelDescription)
        message(FATAL_ERROR "Model descriptions of '${oldFmu}' and '${fmu}' do not match")
    endif()
    set(oldFmu "${fmu}")
    set(oldModelDescription "${modelDescription}")
endforeach()

make_absolute("${OUTPUT_FMU}" absOutputFmu)
zip("${absOutputFmu}" "${mergeDir}")
file(REMOVE_RECURSE "${mergeDir}")
