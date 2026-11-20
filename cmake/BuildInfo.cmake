set(commit "")
set(commit_short "")
set(commit_date "")
set(commit_time 0)
set(dirty 0)

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" log -1 "--format=%H%n%h%n%cI%n%ct"
        WORKING_DIRECTORY "${SOURCE_DIR}"
        RESULT_VARIABLE log_result
        OUTPUT_VARIABLE log_output
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(log_result EQUAL 0 AND log_output)
        string(REPLACE "\n" ";" log_lines "${log_output}")
        list(GET log_lines 0 commit)
        list(GET log_lines 1 commit_short)
        list(GET log_lines 2 commit_date)
        list(GET log_lines 3 commit_time)

        execute_process(
            COMMAND "${GIT_EXECUTABLE}" status --porcelain --untracked-files=no -- src cmake CMakeLists.txt
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE status_output
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(status_output)
            set(dirty 1)
        endif()
    endif()
endif()

if(commit)
    set(product_version "${VERSION} (${commit_short}")
    if(dirty)
        string(APPEND product_version "-dirty")
    endif()
    string(APPEND product_version ", ${commit_date})")
else()
    set(product_version "${VERSION}")
endif()

string(REPLACE "." ";" version_parts "${VERSION}")
list(GET version_parts 0 version_major)
list(GET version_parts 1 version_minor)
list(GET version_parts 2 version_patch)

set(content "#pragma once

#define SDK_VERSION \"${VERSION}\"
#define SDK_VERSION_MAJOR ${version_major}
#define SDK_VERSION_MINOR ${version_minor}
#define SDK_VERSION_PATCH ${version_patch}
#define SDK_COMMIT \"${commit}\"
#define SDK_COMMIT_SHORT \"${commit_short}\"
#define SDK_COMMIT_DATE \"${commit_date}\"
#define SDK_COMMIT_TIME ${commit_time}
#define SDK_DIRTY ${dirty}
#define SDK_PRODUCT_VERSION \"${product_version}\"
")

file(WRITE "${OUTPUT}.tmp" "${content}")
configure_file("${OUTPUT}.tmp" "${OUTPUT}" COPYONLY)
file(REMOVE "${OUTPUT}.tmp")
