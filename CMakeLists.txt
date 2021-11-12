#██████   █████    ███████      █████████    █████████  █████ ███████████    ███████████   ██████████ ██████████  
#░░██████ ░░███   ███░░░░░███   ███░░░░░███  ███░░░░░███░░███ ░█░░░███░░░█   ░░███░░░░░███ ░░███░░░░░█░░███░░░░███ 
# ░███░███ ░███  ███     ░░███ ███     ░░░  ███     ░░░  ░███ ░   ░███  ░     ░███    ░███  ░███  █ ░  ░███   ░░███
# ░███░░███░███ ░███      ░███░███         ░███          ░███     ░███        ░██████████   ░██████    ░███    ░███
# ░███ ░░██████ ░███      ░███░███    █████░███    █████ ░███     ░███        ░███░░░░░███  ░███░░█    ░███    ░███
# ░███  ░░█████ ░░███     ███ ░░███  ░░███ ░░███  ░░███  ░███     ░███        ░███    ░███  ░███ ░   █ ░███    ███ 
# █████  ░░█████ ░░░███████░   ░░█████████  ░░█████████  █████    █████       █████   █████ ██████████ ██████████  
#░░░░░    ░░░░░    ░░░░░░░      ░░░░░░░░░    ░░░░░░░░░  ░░░░░    ░░░░░       ░░░░░   ░░░░░ ░░░░░░░░░░ ░░░░░░░░░░   
CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
CMAKE_POLICY(SET CMP0057 NEW) # "Support new IN_LIST if() operator."
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_C_STANDARD  11)
SET(CMAKE_C_STANDARD_REQUIRED  ON)

#Including cmake dependencies
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(cmake/cmake_function.cmake)
INCLUDE(cmake/cmake_macro.cmake)

#Project name
PROJECT(Noggit)

#Covered by CMAKE_CXX_STANDARD
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-c++98-compat)
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-c++98-compat-pedantic)
#Covered by compilers used
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-gnu-anonymous-struct)
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-variadic-macros)
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-vla)
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-vla-extension)
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-zero-length-array)
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-gnu-zero-variadic-macro-arguments)
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-nested-anon-types)
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-four-char-constants)
#We assume that our constructors and destructors do not access other global state
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-exit-time-destructors)
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-global-constructors)
#Is fine with GNU, required due to our libstdc
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-disabled-macro-expansion)
#We can live with the compilation unit containing the vtable not being fixed
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-weak-vtables)
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-weak-template-vtables)
# __DATE__ and __TIME__ not being reproducible is exactly why they exist.
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-date-time)
#We don't care for a few bytes
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-padded)
#Msvc++ mangles struct/class into name, thus symbols may be called differently
#With a bad forward-decl. we want compilation to fail, not linking.
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS /we4099)
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Werror=mismatched-tags)
#Yes, we intend to use multi-character character constants
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS -Wno-multichar)
#Better exception handling for visual studio, particularly for the asynchronous stuff
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS /EHa)
#Multi core building for visual studio
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS /MP)

add_compiler_flag_if_supported (CMAKE_C_FLAGS -Wno-implicit-function-declaration)

IF(WIN32)
  OPTION(NAME_REUSE_AS_ERROR "Make name reuse warnings errors ?" OFF)
  IF(NAME_REUSE_AS_ERROR)
    # declaration of 'identifier' hides previous: 
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4456) # local declaration
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4457) # function parameter
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4458) # class members
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4459) # global declaration
  ENDIF()

  OPTION(ADDITIONAL_OPTIMIZATION_FLAGS "Enable OpenGL error check ?" OFF)
  IF(ADDITIONAL_OPTIMIZATION_FLAGS)
    MESSAGE( STATUS "Enabled additional optimization flags for msvc.")
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Ob2) # inline any suitable functions
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Oi)  # enable intrasic functions
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Ot)  # favor fast code
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /GL)  # whole program optimization
  ENDIF()
ENDIF()

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
SET(EXECUTABLE_OUTPUT_PATH bin)
SET(LIBARY_OUTPUT_PATH bin)
SET(EXTERNAL_SOURCE_DIR src/external)

OPTION(USE_SQL "Enable sql uid save ? (require mysql installed)" OFF)
OPTION(VALIDATE_OPENGL_PROGRAMS "Validate Opengl programs" OFF)

IF(VALIDATE_OPENGL_PROGRAMS)
  ADD_DEFINITIONS(-DVALIDATE_OPENGL_PROGRAMS)
  IF(APPLE)
    MESSAGE(WARNING "Noggit will most likely not work on a mac with this option enabled.")
  ENDIF()
ENDIF()

includePlatform("prefind")

FIND_PACKAGE(OpenGL REQUIRED )
FIND_PACKAGE(Boost 1.60 COMPONENTS thread filesystem system unit_test_framework REQUIRED)
FIND_PACKAGE(StormLib REQUIRED )
FIND_PACKAGE(Qt5 COMPONENTS Widgets OpenGL OpenGLExtensions Network Xml REQUIRED)

IF(USE_SQL)
  FIND_LIBRARY(MYSQL_LIBRARY NAMES libmysql 
              HINTS "${CMAKE_SOURCE_DIR}/../Noggit3libs/mysql")
  FIND_LIBRARY(MYSQLCPPCONN_LIBRARY NAMES mysqlcppconn 
              HINTS "${CMAKE_SOURCE_DIR}/../Noggit3libs/mysql/connector")
  FIND_PATH(MYSQLCPPCONN_INCLUDE NAMES cppconn/driver.h 
              HINTS "${CMAKE_SOURCE_DIR}/../Noggit3libs/mysql/connector")

  IF(MYSQL_LIBRARY AND MYSQLCPPCONN_LIBRARY AND MYSQLCPPCONN_INCLUDE)
    ADD_DEFINITIONS(-DUSE_MYSQL_UID_STORAGE)
    SET (mysql_sources src/mysql/mysql.cpp)
    SET (mysql_headers src/mysql/mysql.h)
    SOURCE_GROUP("mysql"  FILES ${mysql_sources} ${mysql_headers})
  ELSE()
    MESSAGE(FATAL_ERROR "MySQL lib or connector not found")
  ENDIF()
ENDIF()

ADD_SUBDIRECTORY("${EXTERNAL_SOURCE_DIR}/qt-color-widgets")
ADD_SUBDIRECTORY("${EXTERNAL_SOURCE_DIR}/framelesshelper")
ADD_SUBDIRECTORY("${EXTERNAL_SOURCE_DIR}/qtimgui")
ADD_SUBDIRECTORY("${EXTERNAL_SOURCE_DIR}/QtAdvancedDockingSystem")
ADD_SUBDIRECTORY("${EXTERNAL_SOURCE_DIR}/NodeEditor")
ADD_SUBDIRECTORY("${EXTERNAL_SOURCE_DIR}/libnoise")
ADD_SUBDIRECTORY("${EXTERNAL_SOURCE_DIR}/dbcd-cpp")
ADD_SUBDIRECTORY("${EXTERNAL_SOURCE_DIR}/glm")

# Add the found include directories to our include list.
INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_SOURCE_DIR}/include/")

OPTION(NOGGIT_BUILD_NODE_DATAMODELS "Build node data models? (turn of for faster compiling)" ON)
IF(NOT NOGGIT_BUILD_NODE_DATAMODELS)
  MESSAGE(STATUS "Not building Node data models. Do not attempt to use Node Editor. Do not enable this for deployment!")
  ADD_DEFINITIONS(-DDO_NOT_BUILD_NODES )
ENDIF(NOT NOGGIT_BUILD_NODE_DATAMODELS)

OPTION(NOGGIT_ENABLE_TRACY_PROFILER "Enable tracy profiler" ON)
IF(NOGGIT_ENABLE_TRACY_PROFILER)
  MESSAGE(STATUS "Tracy profiler enabled." )
  ADD_DEFINITIONS(-DTRACY_ENABLE )
ENDIF(NOGGIT_ENABLE_TRACY_PROFILER)

OPTION(NOGGIT_ALL_WARNINGS "Enable all warnings?" OFF)

# Log to console for easier debugging.
OPTION( NOGGIT_LOGTOCONSOLE "Log to console instead of log.txt?" OFF)
IF(NOGGIT_LOGTOCONSOLE)
  MESSAGE(STATUS "And writing log to console instead of log.txt")
  ADD_DEFINITIONS(-DDEBUG__LOGGINGTOCONSOLE)
ENDIF(NOGGIT_LOGTOCONSOLE)

# Disable opengl error log
OPTION(NOGGIT_OPENGL_ERROR_CHECK "Enable OpenGL error check ?" ON)
IF(NOT NOGGIT_OPENGL_ERROR_CHECK)
  MESSAGE(STATUS "OpenGL error check disabled.")
  ADD_DEFINITIONS(-DNOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS)
ENDIF()

includePlatform("postfind")

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/tmp")
INCLUDE_DIRECTORIES(PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# And do the job.
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/PNG2BLP" )
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/PNG2BLP/libimagequant")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/PNG2BLP/libpng")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/PNG2BLP/libtxc_dxtn")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/PNG2BLP/pngpp")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/PNG2BLP/zlib")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/imguizmo")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/imguipiemenu")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/libnoise/src")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/qtgradienteditor")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/tracy")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/dbcd-cpp/include")
INCLUDE_DIRECTORIES(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

collect_files(noggit_root_sources src/noggit FALSE "*.cpp" "")
collect_files(noggit_ui_sources src/noggit/ui TRUE "*.cpp" "")
collect_files(noggit_scripting_sources src/noggit/scripting TRUE "*.cpp" "")
collect_files(math_sources src/math FALSE "*.cpp" "")
collect_files(opengl_sources src/opengl FALSE "*.cpp" "")

IF(NOT NOGGIT_BUILD_NODE_DATAMODELS)
  collect_files(red_sources src/noggit/Red TRUE "*.cpp" "NodeEditor/Nodes/Containers;NodeEditor/Nodes/Data;NodeEditor/Nodes/Functions;NodeEditor/Nodes/Math;NodeEditor/Nodes/World")
ELSE()
  collect_files(red_sources src/noggit/Red TRUE "*.cpp" "")
ENDIF()

collect_files(png_blp_sources src/external/PNG2BLP TRUE "*.c;*.cpp;" "")
collect_files(imguizmo_sources src/external/imguizmo FALSE "*.c;*.cpp;" "")
collect_files(imguipiemenu_sources src/external/imguipiemenu FALSE "*.c;*.cpp;" "")
collect_files(gradienteditor_sources src/external/qtgradienteditor FALSE "*.c;*.cpp;" "")
collect_files(tracy_sources src/external/tracy FALSE "*.c;*.cpp;" "")

SET(util_sources src/util/exception_to_string.cpp)

collect_files(noggit_root_headers src/noggit FALSE "*.h;*.hpp;*.inl" "")
collect_files(noggit_ui_headers src/noggit/ui TRUE "*.h;*.hpp" "")
collect_files(noggit_scripting_headers src/noggit/scripting TRUE "*.h;*.hpp" "")
collect_files(math_headers src/math FALSE "*.h;*.hpp" "")
collect_files(opengl_headers src/opengl FALSE "*.h;*.hpp" "")
collect_files(shaders src/glsl FALSE "*.glsl" "")

IF(NOT NOGGIT_BUILD_NODE_DATAMODELS)
  collect_files(red_headers src/noggit/Red TRUE "*.h;*.hpp" "NodeEditor/Nodes/Containers;NodeEditor/Nodes/Data;NodeEditor/Nodes/Functions;NodeEditor/Nodes/Math;NodeEditor/Nodes/World")
ELSE()
  collect_files(red_headers src/noggit/Red TRUE "*.h;*.hpp" "")
ENDIF()

collect_files(png_blp_headers src/external/PNG2BLP TRUE "*.h;*.hpp" "")
collect_files(imguizmo_headers src/external/imguizmo FALSE "*.h;*.hpp" "")
collect_files(imguipiemenu_headers src/external/imguipiemenu FALSE "*.h;*.hpp" "")
collect_files(gradienteditor_headers src/external/qtgradienteditor FALSE "*.h;*.hpp" "")
collect_files(tracy_headers src/external/tracy FALSE "*.h;*.hpp" "")
collect_files(blizzard_database_headers src/external/dbcd-cpp/include FALSE "*.h;*.hpp" "")

IF(WIN32)
  SET(os_sources include/win/StackWalker.cpp)
  SET(os_headers include/win/StackWalker.h)
ENDIF(WIN32)

#collect_files(headers_to_moc src/noggit true "*.h;*.hpp" "")
# here we need only those which contain qt meta identifiers
#contains_filter(result "${headers_to_moc}" "[\n][ \t]*Q_OBJECT[^a-zA-Z0-9_]")

#qt5_wrap_cpp (moced ${result})
SOURCE_GROUP("noggit"  FILES ${noggit_root_sources} ${noggit_root_headers})
SOURCE_GROUP("noggit\\ui"  FILES ${noggit_ui_sources} ${noggit_ui_headers})
SOURCE_GROUP("noggit\\scripting"  FILES ${noggit_scripting_sources} ${noggit_scripting_headers})
SOURCE_GROUP("opengl"  FILES ${opengl_sources} ${opengl_headers})
SOURCE_GROUP("math"  FILES ${math_sources} ${math_headers})
SOURCE_GROUP("external"  FILES ${external_sources} ${external_headers})
SOURCE_GROUP("os"  FILES ${os_sources} ${os_headers})
SOURCE_GROUP("util"  FILES ${util_sources})
SOURCE_GROUP("glsl"  FILES ${shaders})
SOURCE_GROUP("noggit/Red" FILES ${red_headers} ${red_sources})
SOURCE_GROUP("png_blp" FILES ${png_blp_headers} ${png_blp_sources})
SOURCE_GROUP("imguizmo" FILES ${imguizmo_headers} ${imguizmo_sources})
SOURCE_GROUP("gradient_editor" FILES ${gradienteditor_headers} ${gradienteditor_sources})

collect_files(resource_files resources FALSE "*.qrc" "")
qt5_add_resources (compiled_resource_files ${resource_files})
collect_files(ui_files src TRUE "*.ui" "")
qt5_wrap_ui(compiled_ui_files ${ui_files})

ADD_EXECUTABLE(noggit
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
  ${blizzard_database_headers}
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
  BlizzardDatabaseLib
  qt_imgui_widgets
  qtadvanceddocking
  nodes
  noise-static
  noiseutils-static
  glm
)

SET_PROPERTY(TARGET noggit PROPERTY AUTOMOC ON)

IF(APPLE)
  TARGET_INCLUDE_DIRECTORIES(noggit PRIVATE /usr/local/Cellar/llvm/12.0.1/include/c++/v1/)
  TARGET_LINK_LIBRARIES(noggit /usr/local/Cellar/llvm/12.0.1/lib/libc++.1.0.dylib)
ENDIF()

SET(_noggit_revision_output_dir "${CMAKE_BINARY_DIR}/revision_output")
SET(_noggit_revision_template_file "${CMAKE_SOURCE_DIR}/cmake/revision.h.in")
SET(_noggit_revision_output_file "${_noggit_revision_output_dir}/revision.h")
SET(_noggit_revision_state_file "${CMAKE_BINARY_DIR}/revision.state")
SET(_noggit_revision_script_file "${CMAKE_SOURCE_DIR}/cmake/GenerateRevision.cmake")

INCLUDE_DIRECTORIES("${_noggit_revision_output_dir}")

FIND_PACKAGE(Git)
IF(FALSE) # GIT_FOUND
  ADD_CUSTOM_TARGET(update_git_revision
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

  ADD_DEPENDENCIES(noggit update_git_revision)
ELSE()
  MESSAGE(WARNING "Failed to find a Git executable, will NOT produce a useful version string. Crash logs will be useless. Do NOT distribute.")

  SET(NOGGIT_GIT_VERSION_STRING "UNKNOWN")
  CONFIGURE_FILE("${_noggit_revision_template_file}" "${_noggit_revision_output_file}" @ONLY)
ENDIF()

IF(APPLE)
  TARGET_LINK_LIBRARIES (noggit "-framework Cocoa" "-framework AppKit" "-framework Foundation")
ENDIF()

IF(MYSQL_LIBRARY AND MYSQLCPPCONN_LIBRARY AND MYSQLCPPCONN_INCLUDE)
  TARGET_LINK_LIBRARIES(noggit ${MYSQL_LIBRARY} ${MYSQLCPPCONN_LIBRARY})
  TARGET_INCLUDE_DIRECTORIES(noggit SYSTEM PRIVATE ${MYSQLCPPCONN_INCLUDE})
ENDIF()

IF(NOGGIT_LOGTOCONSOLE AND WIN32)
  SET_PROPERTY(TARGET noggit APPEND PROPERTY LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
  SET_PROPERTY(TARGET noggit APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:"_CONSOLE">)
ENDIF()

includePlatform("pack")

INCLUDE(FetchContent)

# Dependency: json.hpp
FetchContent_Declare (json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1
)
FetchContent_GetProperties (json)
IF(NOT json_POPULATED)
  MESSAGE(STATUS "Installing json.hpp...")
  FetchContent_Populate(json)
ENDIF()
ADD_SUBDIRECTORY(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)

# Dependency: lodepng
FetchContent_Declare (lodepng
  GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
  GIT_TAG 7fdcc96a5e5864eee72911c3ca79b1d9f0d12292
)
FetchContent_GetProperties (lodepng)
IF(NOT lodepng_POPULATED)
  MESSAGE(STATUS "Installing lodepng...")
  FetchContent_Populate (lodepng)
ENDIF()
ADD_LIBRARY(lodepng "${lodepng_SOURCE_DIR}/lodepng.cpp")
TARGET_INCLUDE_DIRECTORIES(lodepng SYSTEM PUBLIC ${lodepng_SOURCE_DIR})

# Dependency: FastNoise2
FetchContent_Declare (fastnoise2
        GIT_REPOSITORY https://github.com/tswow/FastNoise2.git
        GIT_TAG v0.0.1-heightmap
        PATCH_COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_SOURCE_DIR}/cmake/deps/patch_fastnoise2.cmake"
        UPDATE_DISCONNECTED true
        )
FetchContent_GetProperties (fastnoise2)
IF(NOT fastnoise2_POPULATED)
  MESSAGE(STATUS "Installing FastNoise2... (big repo, large download)")
  FetchContent_Populate (fastnoise2)
ENDIF()

SET(FASTNOISE2_NOISETOOL OFF)
SET(FASTNOISE2_TESTS:BOOL OFF)

IF(FASTNOISE2_NOISETOOL)
  ADD_SUBDIRECTORY(${fastnoise2_SOURCE_DIR} ${fastnoise2_BINARY_DIR})
ELSE()
  ADD_SUBDIRECTORY(${fastnoise2_SOURCE_DIR} ${fastnoise2_BINARY_DIR} EXCLUDE_FROM_ALL)
ENDIF()

# Dependency: Lua
FIND_PACKAGE(Lua REQUIRED)
ADD_LIBRARY(Lua-Lua INTERFACE)
ADD_LIBRARY(Lua::Lua ALIAS Lua-Lua)
TARGET_LINK_LIBRARIES(Lua-Lua INTERFACE ${LUA_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(Lua-Lua INTERFACE ${LUA_INCLUDE_DIR})

# Dependency: sol2
FetchContent_Declare (sol2
        GIT_REPOSITORY https://github.com/tswow/sol2
        GIT_TAG b9c83d5ecf6bc9503dc66779f2395dc32dffb1e5
        )
FetchContent_MakeAvailable (sol2)
# sol2::sol2 neither links lua nor sets include directories as system so will clobber us with
# loads of warnings, sadly. It also wants to be install(EXPORT)ed which is not what we want.
ADD_LIBRARY(sane-sol2 INTERFACE)
ADD_LIBRARY(sol2::sane ALIAS sane-sol2)
TARGET_LINK_LIBRARIES(sane-sol2 INTERFACE Lua::Lua)
TARGET_INCLUDE_DIRECTORIES(sane-sol2 SYSTEM INTERFACE "${sol2_SOURCE_DIR}/include")

TARGET_LINK_LIBRARIES(noggit
  lodepng
  FastNoise
  nlohmann_json::nlohmann_json
  sol2::sane
  dl
)
