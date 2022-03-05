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

#Including cmake dependencies
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(cmake/cmake_function.cmake)
INCLUDE(cmake/cmake_macro.cmake)
INCLUDE(FetchContent)

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
#Allow Big obj for msvc compilation
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS /bigobj)

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
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

OPTION(USE_SQL "Enable sql uid save ? (require mysql installed)" OFF)
OPTION(VALIDATE_OPENGL_PROGRAMS "Validate Opengl programs" OFF)

IF(VALIDATE_OPENGL_PROGRAMS)
  ADD_DEFINITIONS(-DVALIDATE_OPENGL_PROGRAMS)
  IF(APPLE)
    MESSAGE(WARNING "Noggit will most likely not work on a mac with this option enabled.")
  ENDIF()
ENDIF()

includePlatform("prefind")
FIND_PACKAGE(Lua REQUIRED)
FIND_PACKAGE(StormLib REQUIRED)
FIND_PACKAGE(CascLib REQUIRED)
#External packages
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(Json REQUIRED)
FIND_PACKAGE(lodepng REQUIRED)

set(FASTNOISE2_NOISETOOL OFF CACHE BOOL "")
FIND_PACKAGE(FastNoise2 REQUIRED)

FIND_PACKAGE(Sol2 REQUIRED)
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
ADD_SUBDIRECTORY("${EXTERNAL_SOURCE_DIR}/glm")

# Add the found include directories to our include list.
INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_SOURCE_DIR}/include/")

OPTION(NOGGIT_BUILD_NODE_DATAMODELS "Build node data models? (turn of for faster compiling)" ON)
IF(NOT NOGGIT_BUILD_NODE_DATAMODELS)
  MESSAGE(STATUS "Not building Node data models. Do not attempt to use Node Editor. Do not enable this for deployment!")
  ADD_DEFINITIONS(-DDO_NOT_BUILD_NODES )
ENDIF(NOT NOGGIT_BUILD_NODE_DATAMODELS)

OPTION(NOGGIT_ENABLE_TRACY_PROFILER "Enable tracy profiler" OFF)
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
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/blizzard-database-library/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${EXTERNAL_SOURCE_DIR}/blizzard-archive-library/include")
INCLUDE_DIRECTORIES(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

#Load noggit source files
collect_files(math_sources src/math FALSE "*.cpp" "")
collect_files(opengl_sources src/opengl FALSE "*.cpp" "")

IF(NOT NOGGIT_BUILD_NODE_DATAMODELS)
  collect_files(noggit_root_sources src/noggit TRUE "*.cpp" "NodeEditor/Nodes/Containers;NodeEditor/Nodes/Data;NodeEditor/Nodes/Functions;NodeEditor/Nodes/Math;NodeEditor/Nodes/World")
ELSE()
  collect_files(noggit_root_sources src/noggit TRUE "*.cpp" "")
ENDIF()

collect_files(png_blp_sources src/external/PNG2BLP TRUE "*.c;*.cpp;" "")
collect_files(imguizmo_sources src/external/imguizmo FALSE "*.c;*.cpp;" "")
collect_files(imguipiemenu_sources src/external/imguipiemenu FALSE "*.c;*.cpp;" "")
collect_files(gradienteditor_sources src/external/qtgradienteditor FALSE "*.c;*.cpp;" "")
collect_files(tracy_sources src/external/tracy FALSE "*.c;*.cpp;" "")
collect_files(archive_sources src/external/blizzard-archive-library/src FALSE "*.c;*.cpp;" "")
collect_files(database_sources src/external/blizzard-database-library/src TRUE "*.c;*.cpp;" "")
collect_files(util_sources src/util TRUE "*.c;*.cpp;" "")
collect_files(util_headers src/util TRUE "*.h;*.hpp" "")

collect_files(noggit_root_headers src/noggit TRUE "*.h;*.hpp;*.inl" "")
collect_files(math_headers src/math FALSE "*.h;*.hpp" "")
collect_files(opengl_headers src/opengl FALSE "*.h;*.hpp" "")
collect_files(shaders src/glsl FALSE "*.glsl" "")

IF(NOT NOGGIT_BUILD_NODE_DATAMODELS)
  collect_files(red_headers src/noggit/ui/tools TRUE "*.h;*.hpp" "NodeEditor/Nodes/Containers;NodeEditor/Nodes/Data;NodeEditor/Nodes/Functions;NodeEditor/Nodes/Math;NodeEditor/Nodes/World")
ELSE()
  collect_files(red_headers src/noggit/ui/tools TRUE "*.h;*.hpp" "")
ENDIF()

collect_files(png_blp_headers src/external/PNG2BLP TRUE "*.h;*.hpp" "")
collect_files(imguizmo_headers src/external/imguizmo FALSE "*.h;*.hpp" "")
collect_files(imguipiemenu_headers src/external/imguipiemenu FALSE "*.h;*.hpp" "")
collect_files(gradienteditor_headers src/external/qtgradienteditor FALSE "*.h;*.hpp" "")
collect_files(tracy_headers src/external/tracy FALSE "*.h;*.hpp" "")
collect_files(archive_headers src/external/blizzard-archive-library/include FALSE "*.h;*.hpp" "")
collect_files(database_headers src/external/blizzard-database-library/include TRUE "*.h;*.hpp" "")

IF(WIN32)
  collect_files(os_sources include/win FALSE "*.c;*.cpp;" "")
  collect_files(os_headers include/win FALSE "*.h;*.hpp" "")
ENDIF(WIN32)

collect_files(resource_files resources FALSE "*.qrc" "")
qt5_add_resources(compiled_resource_files ${resource_files})
collect_files(ui_files src TRUE "*.ui" "")
qt5_wrap_ui(compiled_ui_files ${ui_files})

assign_specific_source_group("noggit/ui/generated" ${compiled_ui_files})
assign_specific_source_group("util/win" ${os_sources} ${os_headers})
assign_specific_source_group("resources" ${ResFiles})
assign_specific_source_group("resources" ${compiled_resource_files})

assign_source_group(
  ${noggit_root_sources} 
  ${noggit_root_headers}
  ${opengl_sources} 
  ${opengl_headers}
  ${math_sources} 
  ${math_headers}
  ${external_sources} 
  ${external_headers}
  ${util_sources}
  ${util_headers}
  ${shaders}
  ${red_headers} 
  ${red_sources}
  ${png_blp_headers} 
  ${png_blp_sources}
  ${imguizmo_headers} 
  ${imguizmo_sources}
  ${gradienteditor_headers} 
  ${gradienteditor_sources}
  ${resource_files}
  ${ui_files}
  ${tracy_sources}
  ${tracy_headers}
  ${archive_sources}
  ${archive_headers}
  ${database_sources}
  ${database_headers}
  ${imguipiemenu_sources}
  ${imguipiemenu_headers}
)


MESSAGE(STATUS ${moced})

ADD_EXECUTABLE(noggit
  WIN32
  MACOSX_BUNDLE
  ${noggit_root_sources}
  ${opengl_sources}
  ${database_sources}
  ${database_headers}
  ${math_sources}
  ${external_sources}
  ${mysql_sources}
  ${os_sources}
  ${util_sources}
  ${util_headers}
  ${red_sources}
  ${png_blp_sources}
  ${imguizmo_sources}
  ${imguipiemenu_sources}
  ${gradienteditor_sources}
  ${tracy_sources}
  ${archive_sources}
  ${noggit_root_headers}
  ${opengl_headers}
  ${math_headers}
  ${external_headers}
  ${mysql_headers}
  ${os_headers}
  ${png_blp_headers}
  ${ResFiles}
  #${moced}
  ${red_headers}
  ${imguizmo_headers}
  ${imguipiemenu_headers}
  ${gradienteditor_headers}
  ${tracy_headers}
  ${archive_headers}
  ${compiled_resource_files}
  ${compiled_ui_files}
  ${shaders}
  ${force_update_file}
)

TARGET_LINK_LIBRARIES (noggit
  ${OPENGL_LIBRARIES}
  StormLib
  CascLib
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
  glm
  lodepng
  FastNoise
  nlohmann_json::nlohmann_json
  sol2::sane
)

#add distribution themes
add_custom_command(TARGET noggit POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/dist/themes"
        $<TARGET_FILE_DIR:noggit>/themes
        COMMAND ${CMAKE_COMMAND} -E remove -f 
        $<TARGET_FILE_DIR:noggit>/themes/.git)

#add distribution definitions
add_custom_command(TARGET noggit POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/dist/listfile"
        $<TARGET_FILE_DIR:noggit>
        COMMAND ${CMAKE_COMMAND} -E remove -f 
        $<TARGET_FILE_DIR:noggit>/.git )

#add distribution listfile
add_custom_command(TARGET noggit POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/dist/definitions"
        $<TARGET_FILE_DIR:noggit>/definitions
        COMMAND ${CMAKE_COMMAND} -E remove -f 
        $<TARGET_FILE_DIR:noggit>/definitions/.git)

# deploy for Qt on Windows
IF(WIN32)
  include(windeployqt)
  windeployqt(noggit)
ENDIF()

SET_PROPERTY(TARGET noggit PROPERTY AUTOMOC ON)

IF(APPLE)
  TARGET_INCLUDE_DIRECTORIES(noggit PRIVATE /usr/local/Cellar/llvm/13.0.0_2/include/c++/v1/)
  TARGET_LINK_LIBRARIES(noggit /usr/local/Cellar/llvm/13.0.0_2/lib/libc++.1.0.dylib)
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

if (UNIX)
  FIND_PACKAGE(BZip2 REQUIRED)
  TARGET_LINK_LIBRARIES (noggit BZip2::BZip2)
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

