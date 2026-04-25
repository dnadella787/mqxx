function(saltpepper_enable_sanitizers target_name)
    if(NOT SALTPEPPER_ENABLE_SANITIZERS)
        return()
    endif()

    if(MSVC)
        message(STATUS "Sanitizers are not configured for MSVC yet.")
        return()
    endif()

    target_compile_options(
        ${target_name}
        PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer)

    target_link_options(
        ${target_name}
        PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer)
endfunction()
