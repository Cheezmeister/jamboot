
This is the readme for jamboot, minimalist boilerplate for procedural games using SDL2+OpenGL2.1.

It is also a bootstrapping script in literate Perl.


## CMake scripts

CMake was really slick when it came out, but it hasn't aged well. Its real selling point is/was
the ability to detect common libraries out of the box, but in practice that's rarely the case.
OpenGL itself is usually fine, but that's about it.

Let's generate a `FindGLEW.cmake` and a `FindSDL2.cmake`.

The former was taken directly from a recent version of CMake and stripped down somewhat. 
It should still work with older CMakes.

    mkdir('cmake') or die 'trying';
    mkdir('cmake/Modules') or die 'trying';
    open(my $fh, '>', 'cmake/Modules/FindGLEW.cmake') or die "Could not open file '$filename' $!";
    print $fh <<'HERE';
    find_path(GLEW_INCLUDE_DIR GL/glew.h)
    find_library(GLEW_LIBRARY NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)
    set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
    set(GLEW_LIBRARIES ${GLEW_LIBRARY})
    include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
    find_package_handle_standard_args(GLEW REQUIRED_VARS GLEW_INCLUDE_DIR GLEW_LIBRARY)
    if(GLEW_FOUND AND NOT TARGET GLEW::GLEW)
      add_library(GLEW::GLEW UNKNOWN IMPORTED)
      set_target_properties(GLEW::GLEW PROPERTIES
        IMPORTED_LOCATION "${GLEW_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIRS}")
    endif()
    mark_as_advanced(GLEW_INCLUDE_DIR GLEW_LIBRARY)
    HERE
    close $fh;

The latter can be found courtesy of GitHub user [tcbrindle](https://raw.githubusercontent.com/tcbrindle/sdl2-cmake-scripts/master/FindSDL2.cmake).

Omitted for now because it's configured in CI and I'm lazy.

    
