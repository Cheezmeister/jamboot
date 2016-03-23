mkdir('cmake') or die 'trying';
mkdir('cmake/Modules') or die 'trying';
open(my $fh, '>', 'cmake/Modules/FindGLEW.cmake') or die "Could not open file 'FindGLEW.cmake' $!";
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
open(my $fh, '>', 'cmake/Modules/FindSDL2.cmake') or die "Could not open file 'FindSDL2.cmake' $!";
close $fh;
