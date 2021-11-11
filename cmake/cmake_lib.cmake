#collect source files from given directory
FUNCTION(collect_files output base_dir do_recurse globbing_exprs exclude_dirs)
        IF("${do_recurse}")
                SET(glob GLOB_RECURSE)
        ELSE()
                SET(glob GLOB)
        ENDIF()

        SET(base_dir "${CMAKE_SOURCE_DIR}/${base_dir}")
        LIST(TRANSFORM globbing_exprs PREPEND "${base_dir}/")
        FILE(${glob} files CONFIGURE_DEPENDS ${globbing_exprs})

        FOREACH(file IN LISTS files)
                SET(match FALSE)

                FOREACH(dir IN LISTS exclude_dirs)
                        IF("${file}" MATCHES "/${dir}/")
                        SET(match TRUE)
                        ENDIF()
                ENDFOREACH()

                IF(NOT ${match})
                        LIST(APPEND result "${file}")
                ENDIF()

        ENDFOREACH()
        SET(${output} "${result}" PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(contains_filter output files regex)

        FOREACH(file IN LISTS files)
                FILE(STRINGS "${file}" contents REGEX "${regex}")
                IF("${contents}")
                        LIST(APPEND result "${file}")
                        MESSAGE("Moced: ${file}")
                ENDIF()
        ENDFOREACH()
        SET(${output} "${result}"PARENT_SCOPE)
ENDFUNCTION()