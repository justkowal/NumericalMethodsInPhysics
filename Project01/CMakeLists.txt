cmake_minimum_required(VERSION 3.16)

# name project after directory
get_filename_component(CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
project(${CURRENT_DIR} LANGUAGES CXX)

# compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
add_compile_options(-Wall -Wextra -Wpedantic -O2)

# check python environment
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_FOUND)
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "import numpy, scipy, matplotlib"
        RESULT_VARIABLE _py_res OUTPUT_QUIET ERROR_QUIET
    )
    if(NOT _py_res EQUAL 0)
        message(STATUS "some python libs missing (numpy/scipy/matplotlib)")
    endif()
endif()

# collect source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
set(ALL_TEXT "")
set(EXES "")
set(LIB_SRCS "")

foreach(SRC IN LISTS SOURCES)
    file(READ "${SRC}" CONTENT)
    string(APPEND ALL_TEXT "${CONTENT}\n")
    if(CONTENT MATCHES "(int|auto)[ \t\r\n]+main[ \t\r\n]*\\(")
        list(APPEND EXES "${SRC}")
    else()
        list(APPEND LIB_SRCS "${SRC}")
    endif()
endforeach()

# collect headers for dependency detection
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "include/*.hpp" "include/*.h")
foreach(HDR IN LISTS HEADERS)
    file(READ "${HDR}" CONTENT)
    string(APPEND ALL_TEXT "${CONTENT}\n")
endforeach()

# detect libraries from includes
set(LIBS m)
set(DEFS "")
if(ALL_TEXT MATCHES "cblas\\.h|openblas")
    list(APPEND LIBS openblas)
endif()
if(ALL_TEXT MATCHES "lapacke\\.h")
    list(APPEND LIBS lapacke openblas)
endif()
if(ALL_TEXT MATCHES "fftw3\\.h")
    list(APPEND LIBS fftw3)
endif()
if(ALL_TEXT MATCHES "omp\\.h|#pragma[ \t]+omp")
    find_package(OpenMP QUIET)
    if(OpenMP_CXX_FOUND)
        list(APPEND LIBS OpenMP::OpenMP_CXX)
    endif()
endif()

# core library
set(CORE "${PROJECT_NAME}_core")
if(LIB_SRCS)
    add_library(${CORE} STATIC ${LIB_SRCS})
else()
    add_library(${CORE} INTERFACE)
endif()
target_include_directories(${CORE} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
target_link_libraries(${CORE} PUBLIC ${LIBS})
target_compile_definitions(${CORE} PUBLIC ${DEFS})

# build targets and run_all
list(SORT EXES)
set(RUN_CMDS "")
set(RAN_SCRIPTS "")

foreach(EXEC_SRC IN LISTS EXES)
    get_filename_component(NAME "${EXEC_SRC}" NAME_WE)
    add_executable(${NAME} "${EXEC_SRC}")
    target_link_libraries(${NAME} PRIVATE ${CORE})

    list(APPEND RUN_CMDS
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "Output of ${NAME}:"
        COMMAND ${NAME}
    )

    # matching plot script
    set(PLOT_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/plot_${NAME}.py")
    if(EXISTS "${PLOT_SCRIPT}" AND Python3_FOUND)
        list(APPEND RUN_CMDS COMMAND "${Python3_EXECUTABLE}" "${PLOT_SCRIPT}")
        list(APPEND RAN_SCRIPTS "${PLOT_SCRIPT}")
    endif()
endforeach()

# remaining plot scripts
file(GLOB ALL_PLOT_SCRIPTS CONFIGURE_DEPENDS "scripts/plot_*.py")
foreach(SCRIPT IN LISTS ALL_PLOT_SCRIPTS)
    if(NOT "${SCRIPT}" IN_LIST RAN_SCRIPTS AND Python3_FOUND)
        list(APPEND RUN_CMDS COMMAND "${Python3_EXECUTABLE}" "${SCRIPT}")
    endif()
endforeach()

# run_all target
if(EXES)
    add_custom_target(run_all ${RUN_CMDS} WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    add_custom_target(run-all DEPENDS run_all)
endif()
