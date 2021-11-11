# This file is part of Noggit3, licensed under GNU General Public License (version 3).
cmake_minimum_required(VERSION 3.3)
cmake_policy (SET CMP0057 NEW) # "Support new IN_LIST if() operator."
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_C_STANDARD  11)
set (CMAKE_C_STANDARD_REQUIRED  ON)

#including dependencies
include(CheckCXXCompilerFlag)
include(cmake/cmake_lib.cmake)
include(cmake/cmake_macro.cmake)

# Project name
project(Noggit)

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

# better exception handling for visual studio, particularly for the asynchronous stuff
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS /EHa)
# multi core building for visual studio
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /MP)

if(WIN32)
  OPTION(NAME_REUSE_AS_ERROR "Make name reuse warnings errors ?" OFF)
  if(NAME_REUSE_AS_ERROR)
    # declaration of 'identifier' hides previous: 
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4456) # local declaration
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4457) # function parameter
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4458) # class members
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4459) # global declaration
  endif()

  OPTION(ADDITIONAL_OPTIMIZATION_FLAGS "Enable OpenGL error check ?" OFF)
  IF(ADDITIONAL_OPTIMIZATION_FLAGS)
    MESSAGE( STATUS "Enabled additional optimization flags for msvc.")
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Ob2) # inline any suitable functions
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Oi)  # enable intrasic functions
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Ot)  # favor fast code
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /GL)  # whole program optimization
  ENDIF()
endif()

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBARY_OUTPUT_PATH bin)

OPTION(USE_SQL "Enable sql uid save ? (require mysql installed)" OFF)
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
find_package (Qt5 COMPONENTS Widgets OpenGL OpenGLExtensions Network Xml REQUIRED)

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
add_subdirectory (src/external/libnoise)

# Add the found include directories to our include list.
include_directories (SYSTEM "${CMAKE_SOURCE_DIR}/include/")

OPTION(NOGGIT_BUILD_NODE_DATAMODELS "Build node data models? (turn of for faster compiling)" ON)
IF(NOT NOGGIT_BUILD_NODE_DATAMODELS)
  MESSAGE( STATUS "Not building Node data models. Do not attempt to use Node Editor. Do not enable this for deployment!" )
  ADD_DEFINITIONS( -DDO_NOT_BUILD_NODES )
ENDIF(NOT NOGGIT_BUILD_NODE_DATAMODELS)

OPTION(NOGGIT_ENABLE_TRACY_PROFILER "Enable tracy profiler" ON)
IF(NOGGIT_ENABLE_TRACY_PROFILER)
  MESSAGE( STATUS "Tracy profiler enabled." )
  ADD_DEFINITIONS( -DTRACY_ENABLE )
ENDIF(NOGGIT_ENABLE_TRACY_PROFILER)

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
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/external/libnoise/src" )
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/external/qtgradienteditor" )
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/external/tracy" )
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})


collect_files(noggit_root_sources src/noggit false "*.cpp" "")
collect_files(noggit_ui_sources src/noggit/ui true "*.cpp" "")
collect_files(noggit_scripting_sources src/noggit/scripting true "*.cpp" "")
collect_files(math_sources src/math false "*.cpp" "")
collect_files(opengl_sources src/opengl false "*.cpp" "")

IF(NOT NOGGIT_BUILD_NODE_DATAMODELS)
  collect_files(red_sources src/noggit/Red true "*.cpp" "NodeEditor/Nodes/Containers;NodeEditor/Nodes/Data;NodeEditor/Nodes/Functions;NodeEditor/Nodes/Math;NodeEditor/Nodes/World")
ELSE()
  collect_files(red_sources src/noggit/Red true "*.cpp" "")
ENDIF()

collect_files(png_blp_sources src/external/PNG2BLP true "*.c;*.cpp;" "")
collect_files(imguizmo_sources src/external/imguizmo false "*.c;*.cpp;" "")
collect_files(imguipiemenu_sources src/external/imguipiemenu false "*.c;*.cpp;" "")
collect_files(gradienteditor_sources src/external/qtgradienteditor false "*.c;*.cpp;" "")
collect_files(tracy_sources src/external/tracy false "*.c;*.cpp;" "")

set ( util_sources
      src/util/exception_to_string.cpp
    )

collect_files(noggit_root_headers src/noggit false "*.h;*.hpp;*.inl" "")
collect_files(noggit_ui_headers src/noggit/ui true "*.h;*.hpp" "")
collect_files(noggit_scripting_headers src/noggit/scripting true "*.h;*.hpp" "")
collect_files(math_headers src/math false "*.h;*.hpp" "")
collect_files(opengl_headers src/opengl false "*.h;*.hpp" "")
collect_files(shaders src/glsl false "*.glsl" "")

IF(NOT NOGGIT_BUILD_NODE_DATAMODELS)
  collect_files(red_headers src/noggit/Red true "*.h;*.hpp" "NodeEditor/Nodes/Containers;NodeEditor/Nodes/Data;NodeEditor/Nodes/Functions;NodeEditor/Nodes/Math;NodeEditor/Nodes/World")
ELSE()
  collect_files(red_headers src/noggit/Red true "*.h;*.hpp" "")
ENDIF()

collect_files(png_blp_headers src/external/PNG2BLP true "*.h;*.hpp" "")
collect_files(imguizmo_headers src/external/imguizmo false "*.h;*.hpp" "")
collect_files(imguipiemenu_headers src/external/imguipiemenu false "*.h;*.hpp" "")
collect_files(gradienteditor_headers src/external/qtgradienteditor false "*.h;*.hpp" "")
collect_files(tracy_headers src/external/tracy false "*.h;*.hpp" "")


IF(WIN32)
  set ( os_sources
        include/win/StackWalker.cpp)
  set ( os_headers
        include/win/StackWalker.h)
ENDIF(WIN32)

#collect_files(headers_to_moc src/noggit true "*.h;*.hpp" "")
# here we need only those which contain qt meta identifiers
#contains_filter(result "${headers_to_moc}" "[\n][ \t]*Q_OBJECT[^a-zA-Z0-9_]")

#qt5_wrap_cpp (moced ${result})
source_group("noggit"  FILES ${noggit_root_sources} ${noggit_root_headers})
source_group("noggit\\ui"  FILES ${noggit_ui_sources} ${noggit_ui_headers})
source_group("noggit\\scripting"  FILES ${noggit_scripting_sources} ${noggit_scripting_headers})
source_group("opengl"  FILES ${opengl_sources} ${opengl_headers})
source_group("math"  FILES ${math_sources} ${math_headers})
source_group("external"  FILES ${external_sources} ${external_headers})
source_group("os"  FILES ${os_sources} ${os_headers})
source_group("util"  FILES ${util_sources})
source_group("glsl"  FILES ${shaders})
source_group("noggit/Red" FILES ${red_headers} ${red_sources})
source_group("png_blp" FILES ${png_blp_headers} ${png_blp_sources})
source_group("imguizmo" FILES ${imguizmo_headers} ${imguizmo_sources})
source_group("gradient_editor" FILES ${gradienteditor_headers} ${gradienteditor_sources})

collect_files(resource_files resources false "*.qrc" "")
qt5_add_resources (compiled_resource_files ${resource_files})
collect_files(ui_files src true "*.ui" "")
qt5_wrap_ui(compiled_ui_files ${ui_files})

ADD_EXECUTABLE ( noggit
                  WIN32
                  MACOSX_BUNDLE
                  ${noggit_root_sources}
                  ${noggit_ui_sources}
                  ${noggit_scripting_sources}
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
                  ${gradienteditor_sources}
                  ${tracy_sources}
                  ${noggit_root_headers}
                  ${noggit_ui_headers}
                  ${noggit_scripting_headers}
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
                  ${gradienteditor_headers}
                  ${tracy_headers}
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
  Qt5::Xml
  ColorWidgets-qt5
  FramelessHelper
  qt_imgui_widgets
  qtadvanceddocking
  nodes
  noise-static
  noiseutils-static
)

set_property(TARGET noggit PROPERTY AUTOMOC ON)

if(APPLE)
  target_include_directories(noggit PRIVATE /usr/local/Cellar/llvm/12.0.1/include/c++/v1/)
  target_link_libraries(noggit /usr/local/Cellar/llvm/12.0.1/lib/libc++.1.0.dylib)
endif()

set (_noggit_revision_output_dir "${CMAKE_BINARY_DIR}/revision_output")
set (_noggit_revision_template_file "${CMAKE_SOURCE_DIR}/cmake/revision.h.in")
set (_noggit_revision_output_file "${_noggit_revision_output_dir}/revision.h")
set (_noggit_revision_state_file "${CMAKE_BINARY_DIR}/revision.state")
set (_noggit_revision_script_file "${CMAKE_SOURCE_DIR}/cmake/GenerateRevision.cmake")

include_directories ("${_noggit_revision_output_dir}")

find_package (Git)
if (FALSE) # GIT_FOUND
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

include (FetchContent)

# Dependency: json.hpp
FetchContent_Declare (json
        GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
        GIT_TAG v3.9.1
        )
FetchContent_GetProperties (json)
if (NOT json_POPULATED)
  message (STATUS "Installing json.hpp...")
  FetchContent_Populate (json)
endif()
add_subdirectory (${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)

# Dependency: lodepng
FetchContent_Declare (lodepng
        GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
        GIT_TAG 7fdcc96a5e5864eee72911c3ca79b1d9f0d12292
        )
FetchContent_GetProperties (lodepng)
if (NOT lodepng_POPULATED)
  message (STATUS "Installing lodepng...")
  FetchContent_Populate (lodepng)
endif()
add_library (lodepng "${lodepng_SOURCE_DIR}/lodepng.cpp")
target_include_directories (lodepng SYSTEM PUBLIC ${lodepng_SOURCE_DIR})

# Dependency: FastNoise2
FetchContent_Declare (fastnoise2
        GIT_REPOSITORY https://github.com/tswow/FastNoise2.git
        GIT_TAG v0.0.1-heightmap
        PATCH_COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_SOURCE_DIR}/cmake/deps/patch_fastnoise2.cmake"
        UPDATE_DISCONNECTED true
        )
FetchContent_GetProperties (fastnoise2)
if (NOT fastnoise2_POPULATED)
  message (STATUS "Installing FastNoise2... (big repo, large download)")
  FetchContent_Populate (fastnoise2)
endif()

set(FASTNOISE2_NOISETOOL OFF)
set(FASTNOISE2_TESTS:BOOL OFF)

if (FASTNOISE2_NOISETOOL)
  add_subdirectory (${fastnoise2_SOURCE_DIR} ${fastnoise2_BINARY_DIR})
else()
  add_subdirectory (${fastnoise2_SOURCE_DIR} ${fastnoise2_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Dependency: Lua
find_package (Lua REQUIRED)
add_library (Lua-Lua INTERFACE)
add_library (Lua::Lua ALIAS Lua-Lua)
target_link_libraries (Lua-Lua INTERFACE ${LUA_LIBRARIES})
target_include_directories (Lua-Lua INTERFACE ${LUA_INCLUDE_DIR})

#include blizzardDatabase
#add_subdirectory(src/dbcd-cpp)

# Dependency: sol2
FetchContent_Declare (sol2
        GIT_REPOSITORY https://github.com/tswow/sol2
        GIT_TAG b9c83d5ecf6bc9503dc66779f2395dc32dffb1e5
        )
FetchContent_MakeAvailable (sol2)
# sol2::sol2 neither links lua nor sets include directories as system so will clobber us with
# loads of warnings, sadly. It also wants to be install(EXPORT)ed which is not what we want.
add_library (sane-sol2 INTERFACE)
add_library (sol2::sane ALIAS sane-sol2)
target_link_libraries (sane-sol2 INTERFACE Lua::Lua)
target_include_directories (sane-sol2 SYSTEM INTERFACE "${sol2_SOURCE_DIR}/include")

target_link_libraries (noggit
		    #BlizzardDatabaseLib
        lodepng
        FastNoise
        nlohmann_json::nlohmann_json
        sol2::sane
        dl
        )
