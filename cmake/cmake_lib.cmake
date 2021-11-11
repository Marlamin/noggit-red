function(
        collect_files
        output
        base_dir
        do_recurse
        globbing_exprs
        exclude_dirs
)
    if("${do_recurse}")
        set(glob GLOB_RECURSE)
    else()
        set(glob GLOB)
    endif()

    set(base_dir "${CMAKE_SOURCE_DIR}/${base_dir}")
    list(
            TRANSFORM
            globbing_exprs
            PREPEND "${base_dir}/"
    )
    file(
            ${glob}
            files
            CONFIGURE_DEPENDS
            ${globbing_exprs}
    )

    foreach(
            file
            IN LISTS files
    )
        set(match FALSE)

        foreach(
                dir
                IN LISTS exclude_dirs
        )
            if("${file}" MATCHES "/${dir}/")
                set(match TRUE)
            endif()
        endforeach()

        if(NOT ${match})
            list(
                    APPEND
                    result
                    "${file}"
            )
        endif()
    endforeach()

    set(
            ${output} "${result}"
            PARENT_SCOPE
    )
endfunction()

function(
        contains_filter
        output
        files
        regex
)
  foreach(
          file
          IN LISTS files
  )
    file(
            STRINGS
            "${file}"
            contents
            REGEX "${regex}"
    )

    if("${contents}")
      list(
              APPEND
              result
              "${file}"
      )
      MESSAGE("Moced: ${file}")
    endif()
  endforeach()

  set(
          ${output} "${result}"
          PARENT_SCOPE
  )
endfunction()