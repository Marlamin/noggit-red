
MACRO(add_compiler_flag_if_supported _VAR _FLAG)
        STRING (MAKE_C_IDENTIFIER "CXX_COMPILER_SUPPORTS_${_FLAG}" _test_variable)
        check_cxx_compiler_flag ("${_FLAG}" ${_test_variable})
        IF(${_test_variable})
                IF("${${_VAR}}" STREQUAL "")
                        SET(${_VAR} "${_FLAG}")
                ELSE()
                        SET(${_VAR} "${${_VAR}} ${_FLAG}")
                ENDIF()
        ENDIF()
ENDMACRO()

#Platform include
MACRO(includePlatform SUFFIX)
        IF(UNIX)
                IF(APPLE)
                        INCLUDE("${CMAKE_SOURCE_DIR}/cmake/apple_${SUFFIX}.cmake")
                ELSE(APPLE)
                        INCLUDE("${CMAKE_SOURCE_DIR}/cmake/linux_${SUFFIX}.cmake")
                ENDIF(APPLE)
        ELSE(UNIX)
                IF(WIN32)
                        INCLUDE("${CMAKE_SOURCE_DIR}/cmake/win32_${SUFFIX}.cmake")
                        #adds for library repo
                        SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/lib/")
                        #storm lib
                        SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/include/")
                        #boost
                        INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/")
                        SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/libs/")
                        SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/")
	        ENDIF(WIN32)
        ENDIF(UNIX)
ENDMACRO(includePlatform)