# This file is part of Noggit3, licensed under GNU General Public License (version 3).

cmake_minimum_required(VERSION 3.3)
cmake_policy (SET CMP0057 NEW) # "Support new IN_LIST if() operator."
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_C_STANDARD  11)
set (CMAKE_C_STANDARD_REQUIRED  ON)

# Project name
project(Noggit)


include (CheckCXXCompilerFlag)
macro (add_compiler_flag_if_supported _VAR _FLAG)
  string (MAKE_C_IDENTIFIER "CXX_COMPILER_SUPPORTS_${_FLAG}" _test_variable)
  check_cxx_compiler_flag ("${_FLAG}" ${_test_variable})
  if (${_test_variable})
    if ("${${_VAR}}" STREQUAL "")
      set (${_VAR} "${_FLAG}")
    else()
      set (${_VAR} "${${_VAR}} ${_FLAG}")
    endif()
  endif()
endmacro()

MACRO(COLLECT_FILES recursive result dir_source ext_list)
  LIST(APPEND globbing ${ext_list})
  SET(directory ${PROJECT_SOURCE_DIR}/${dir_source})
  LIST(TRANSFORM globbing PREPEND ${directory}/*)

  IF(${recursive})
    FILE(GLOB_RECURSE files RELATIVE ${directory} CONFIGURE_DEPENDS ${globbing})
  ELSE()
    FILE(GLOB files RELATIVE ${directory} CONFIGURE_DEPENDS ${globbing})
  ENDIF()

  LIST(TRANSFORM files PREPEND ${dir_source}/)
  SET(${result} ${files})
  UNSET(files)
  UNSET(directory)
  UNSET(globbing)
ENDMACRO()

add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -fcolor-diagnostics)

# covered by CMAKE_CXX_STANDARD
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-c++98-compat)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-c++98-compat-pedantic)

# covered by compilers used
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-gnu-anonymous-struct)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-variadic-macros)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-vla)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-vla-extension)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-zero-length-array)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-gnu-zero-variadic-macro-arguments)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-nested-anon-types)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-four-char-constants)

# we assume that our constructors and destructors do not access other global state
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-exit-time-destructors)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-global-constructors)

# is fine with GNU, required due to our libstdc
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-disabled-macro-expansion)

# we can live with the compilation unit containing the vtable not being fixed
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-weak-vtables)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-weak-template-vtables)

# __DATE__ and __TIME__ not being reproducible is exactly why they exist.
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-date-time)

# we don't care for a few bytes
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-padded)

# msvc++ mangles struct/class into name, thus symbols may be called differently
# with a bad forward-decl. we want compilation to fail, not linking.
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4099)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Werror=mismatched-tags)

# yes, we intend to use multi-character character constants
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-multichar)

add_compiler_flag_if_supported (CMAKE_C_FLAGS -Wno-implicit-function-declaration)

if(WIN32)
  OPTION(NAME_REUSE_AS_ERROR "Make name reuse warnings errors ?" OFF)
  if(NAME_REUSE_AS_ERROR)
    # declaration of 'identifier' hides previous: 
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4456) # local declaration
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4457) # function parameter
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4458) # class members
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4459) # global declaration
  endif()
endif()

# better exception handling for visual studio, particularly for the asynchronous stuff
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS /EHa)
# multi core building for visual studio
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /MP)


if(WIN32)
  # Disable opengl error log
  OPTION(ADDITIONAL_OPTIMIZATION_FLAGS "Enable OpenGL error check ?" OFF)
  IF(ADDITIONAL_OPTIMIZATION_FLAGS)
    MESSAGE( STATUS "Enabled additional optimization flags for msvc.")
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Ob2) # inline any suitable functions
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Oi)  # enable intrasic functions
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Ot)  # favor fast code
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /GL)  # whole program optimization
  ENDIF()
endif(WIN32)


SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBARY_OUTPUT_PATH bin)

OPTION(USE_SQL "Enable sql uid save ? (require mysql installed)" OFF)

macro(includePlattform SUFFIX)
  if(UNIX)
    if(APPLE)
      include("${CMAKE_SOURCE_DIR}/cmake/apple_${SUFFIX}.cmake")
    else(APPLE)
      include("${CMAKE_SOURCE_DIR}/cmake/linux_${SUFFIX}.cmake")
    endif(APPLE)
  else(UNIX)
    if(WIN32)
      include("${CMAKE_SOURCE_DIR}/cmake/win32_${SUFFIX}.cmake")
      # adds for library repo
      set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/lib/")
      #storm lib
      set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/include/")
      #boost
      include_directories (SYSTEM "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/")
      set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/libs/")
      set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/")
	  endif(WIN32)
  endif(UNIX)
endmacro(includePlattform)

OPTION(VALIDATE_OPENGL_PROGRAMS "Validate Opengl programs" OFF)


if(VALIDATE_OPENGL_PROGRAMS)
  add_definitions ( -DVALIDATE_OPENGL_PROGRAMS)
  
  if(APPLE)
    message(WARNING "Noggit will most likely not work on a mac with this option enabled.")
  endif()
endif()

includePlattform("prefind")

FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( Boost 1.60 COMPONENTS thread filesystem system unit_test_framework REQUIRED )
FIND_PACKAGE( StormLib REQUIRED )
find_package (Qt5 COMPONENTS Widgets OpenGL OpenGLExtensions)


if (USE_SQL)
  find_library(MYSQL_LIBRARY
    NAMES libmysql
    HINTS "${CMAKE_SOURCE_DIR}/../Noggit3libs/mysql")
  find_library(MYSQLCPPCONN_LIBRARY
    NAMES mysqlcppconn
    HINTS "${CMAKE_SOURCE_DIR}/../Noggit3libs/mysql/connector")
  find_path (MYSQLCPPCONN_INCLUDE
    NAMES cppconn/driver.h
    HINTS "${CMAKE_SOURCE_DIR}/../Noggit3libs/mysql/connector")

  if (MYSQL_LIBRARY AND MYSQLCPPCONN_LIBRARY AND MYSQLCPPCONN_INCLUDE)
    add_definitions ( -DUSE_MYSQL_UID_STORAGE )

    set ( mysql_sources
          src/mysql/mysql.cpp
        )

    set ( mysql_headers
          src/mysql/mysql.h
        )
    source_group("mysql"  FILES ${mysql_sources} ${mysql_headers})
  else()
     message (FATAL_ERROR "MySQL lib or connector not found")
  endif()
endif()

add_subdirectory (src/external/qt-color-widgets)
add_subdirectory (src/external/framelesshelper)
add_subdirectory (src/external/qtimgui)
add_subdirectory (src/external/QtAdvancedDockingSystem)
add_subdirectory (src/external/NodeEditor)

# Add the found include directories to our include list.
include_directories (SYSTEM "${CMAKE_SOURCE_DIR}/include/")

OPTION(NOGGIT_ALL_WARNINGS "Enable all warnings?" OFF)

# Log to console for easier debugging.
OPTION( NOGGIT_LOGTOCONSOLE "Log to console instead of log.txt?" OFF )
IF( NOGGIT_LOGTOCONSOLE )
  MESSAGE( STATUS "And writing log to console instead of log.txt" )
  ADD_DEFINITIONS( -DDEBUG__LOGGINGTOCONSOLE )
ENDIF( NOGGIT_LOGTOCONSOLE )

# Disable opengl error log
OPTION(NOGGIT_OPENGL_ERROR_CHECK "Enable OpenGL error check ?" ON)
IF(NOT NOGGIT_OPENGL_ERROR_CHECK )
  MESSAGE( STATUS "OpenGL error check disabled." )
  ADD_DEFINITIONS( -DNOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS )
ENDIF()

includePlattform("postfind")

include_directories ("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/tmp")
include_directories(PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# And do the job.
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src" )
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/external/PNG2BLP" )
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/external/PNG2BLP/libimagequant" )
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/external/PNG2BLP/libpng" )
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/external/PNG2BLP/libtxc_dxtn" )
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/external/PNG2BLP/pngpp" )
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/external/PNG2BLP/zlib" )
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/external/imguizmo" )
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/external/imguipiemenu" )
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

COLLECT_FILES(false noggit_root_sources src/noggit .cpp)
COLLECT_FILES(true noggit_ui_sources src/noggit/ui .cpp)
COLLECT_FILES(false math_sources src/math .cpp)
COLLECT_FILES(false opengl_sources src/opengl .cpp)
COLLECT_FILES(true red_sources src/noggit/Red .cpp)
COLLECT_FILES(true png_blp_sources src/external/PNG2BLP ".c;.cpp;")
COLLECT_FILES(false imguizmo_sources src/external/imguizmo ".c;.cpp;")
COLLECT_FILES(false imguipiemenu_sources src/external/imguipiemenu ".c;.cpp;")
set ( util_sources
      src/util/exception_to_string.cpp
    )
COLLECT_FILES(false noggit_root_headers src/noggit ".h;.hpp")
COLLECT_FILES(true noggit_ui_headers src/noggit/ui ".h;.hpp")
COLLECT_FILES(false math_headers src/math ".h;.hpp")
COLLECT_FILES(false opengl_headers src/opengl ".h;.hpp")
COLLECT_FILES(false shaders src/glsl .glsl)
COLLECT_FILES(true red_headers src/noggit/Red ".hpp;.inl")
COLLECT_FILES(true png_blp_headers src/external/PNG2BLP ".h;.hpp;")
COLLECT_FILES(false imguizmo_headers src/external/imguizmo ".h;.hpp;")
COLLECT_FILES(false imguipiemenu_headers src/external/imguipiemenu ".h;.hpp;")

IF(WIN32)
  set ( os_sources
        include/win/StackWalker.cpp)
  set ( os_headers
        include/win/StackWalker.h)
ENDIF(WIN32)

COLLECT_FILES(true headers_to_moc src/noggit ".h;.hpp")
# here we need only those which contain qt meta identifiers
FOREACH(file IN LISTS headers_to_moc)
  SET(file_full ${file})
  STRING(PREPEND file_full ${PROJECT_SOURCE_DIR}/)
  FILE(STRINGS ${file_full} contents NEWLINE_CONSUME REGEX "Q_OBJECT[\t ;]*\n")

  IF("${contents}" STREQUAL "")
    LIST(REMOVE_ITEM headers_to_moc ${file})
  ENDIF()
ENDFOREACH()

UNSET(file_full)
qt5_wrap_cpp (moced ${headers_to_moc})
source_group("noggit"  FILES ${noggit_root_sources} ${noggit_root_headers})
source_group("noggit\\ui"  FILES ${noggit_ui_sources} ${noggit_ui_headers})
source_group("opengl"  FILES ${opengl_sources} ${opengl_headers})
source_group("math"  FILES ${math_sources} ${math_headers})
source_group("external"  FILES ${external_sources} ${external_headers})
source_group("os"  FILES ${os_sources} ${os_headers})
source_group("util"  FILES ${util_sources})
source_group("glsl"  FILES ${shaders})
source_group("noggit/Red" FILES ${red_headers} ${red_sources})
source_group("png_blp" FILES ${png_blp_headers} ${png_blp_sources})
source_group("imguizmo" FILES ${imguizmo_headers} ${imguizmo_sources})

COLLECT_FILES(false resource_files resources .qrc)
qt5_add_resources (compiled_resource_files ${resource_files})
COLLECT_FILES(true ui_files src .ui)
qt5_wrap_ui(compiled_ui_files ${ui_files})

ADD_EXECUTABLE ( noggit
                  WIN32
                  MACOSX_BUNDLE
                  ${noggit_root_sources}
                  ${noggit_ui_sources}
                  ${opengl_sources}
                  ${math_sources}
                  ${external_sources}
                  ${mysql_sources}
                  ${os_sources}
                  ${util_sources}
                  ${red_sources}
                  ${png_blp_sources}
                  ${imguizmo_sources}
                  ${imguipiemenu_sources}
                  ${noggit_root_headers}
                  ${noggit_ui_headers}
                  ${opengl_headers}
                  ${math_headers}
                  ${external_headers}
                  ${mysql_headers}
                  ${os_headers}
                  ${png_blp_headers}
                  ${ResFiles}
                  ${moced}
                  ${red_headers}
                  ${imguizmo_headers}
                  ${imguipiemenu_headers}
                  ${compiled_resource_files}
                  ${compiled_ui_files}
                  ${shaders}
                  ${force_update_file}
                )
                
TARGET_LINK_LIBRARIES (noggit
  ${OPENGL_LIBRARIES}
  StormLib
  Boost::thread
  Boost::filesystem
  Boost::system
  Qt5::Widgets
  Qt5::OpenGL
  Qt5::OpenGLExtensions
  ColorWidgets-qt5
  FramelessHelper
  qt_imgui_widgets
  qtadvanceddocking
  nodes
)

set (_noggit_revision_output_dir "${CMAKE_BINARY_DIR}/revision_output")
set (_noggit_revision_template_file "${CMAKE_SOURCE_DIR}/cmake/revision.h.in")
set (_noggit_revision_output_file "${_noggit_revision_output_dir}/revision.h")
set (_noggit_revision_state_file "${CMAKE_BINARY_DIR}/revision.state")
set (_noggit_revision_script_file "${CMAKE_SOURCE_DIR}/cmake/GenerateRevision.cmake")

include_directories ("${_noggit_revision_output_dir}")

find_package (Git)
if (GIT_FOUND)
  add_custom_target (update_git_revision
    ALL
    DEPENDS "${_noggit_revision_template_file}"
            "${_noggit_revision_script_file}"
    BYPRODUCTS "${_noggit_revision_output_file}"
               "${_noggit_revision_state_file}"
    COMMAND ${CMAKE_COMMAND}
              -D_noggit_revision_template_file="${_noggit_revision_template_file}"
              -D_noggit_revision_output_file="${_noggit_revision_output_file}"
              -D_noggit_revision_state_file="${_noggit_revision_state_file}"
              -DGIT_EXECUTABLE="${GIT_EXECUTABLE}"
              -P "${_noggit_revision_script_file}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

  add_dependencies (noggit update_git_revision)
else()
  message (WARNING "Failed to find a Git executable, will NOT produce a "
    "useful version string. Crash logs will be useless. Do NOT distribute.")

  set (NOGGIT_GIT_VERSION_STRING "UNKNOWN")
  configure_file ("${_noggit_revision_template_file}"
                  "${_noggit_revision_output_file}" @ONLY)
endif()

if(APPLE)
  TARGET_LINK_LIBRARIES (noggit
    "-framework Cocoa"
    "-framework AppKit"
    "-framework Foundation"
  )
endif()

if (MYSQL_LIBRARY AND MYSQLCPPCONN_LIBRARY AND MYSQLCPPCONN_INCLUDE)
  target_link_libraries (noggit ${MYSQL_LIBRARY} ${MYSQLCPPCONN_LIBRARY})
  target_include_directories (noggit SYSTEM PRIVATE ${MYSQLCPPCONN_INCLUDE})
endif()

if (NOGGIT_LOGTOCONSOLE AND WIN32)
  set_property (TARGET noggit APPEND PROPERTY LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
  set_property (TARGET noggit APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:"_CONSOLE">)
endif()

includePlattform("pack")

add_library (noggit-math STATIC
  "src/math/matrix_4x4.cpp"
  "src/math/vector_2d.cpp"
)
add_library (noggit::math ALIAS noggit-math)

include (CTest)
enable_testing()

add_executable (math-vector_2d.test test/math/vector_2d.cpp)
target_compile_definitions (math-vector_2d.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_link_libraries (math-vector_2d.test Boost::unit_test_framework noggit::math)
add_test (NAME math-vector_2d COMMAND $<TARGET_FILE:math-vector_2d.test>)

add_executable (math-trig.test test/math/trig.cpp)
target_compile_definitions (math-trig.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_link_libraries (math-trig.test Boost::unit_test_framework noggit::math)
add_test (NAME math-trig COMMAND $<TARGET_FILE:math-trig.test>)

add_executable (math-matrix_4x4.test test/math/matrix_4x4.cpp)
target_compile_definitions (math-matrix_4x4.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_link_libraries (math-matrix_4x4.test Boost::unit_test_framework noggit::math)
add_test (NAME math-matrix_4x4 COMMAND $<TARGET_FILE:math-matrix_4x4.test>)
