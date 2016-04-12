# [Jamboot](https://github.com/Cheezmeister/jamboot)

Minimalist boilerplate for procedural games using SDL2+OpenGL2.1.

## Features

* Cross-platform window creation and GL initialization
* Gamepad, Mouse, and Keyboard input
* Manage yer shaders

### Not Features

* No components
* No particles
* No mobile
* No steam
* No sound (yet)

## CMake scripts

This document doubles as a bootstrapping script in literate Perl.

CMake has problems detecting SDL2 and GLEW. OpenGL itself is usually detected fine, but that's about it.

Let's generate a `FindGLEW.cmake` and a `FindSDL2.cmake`.

    mkdir('cmake') or die 'trying';
    mkdir('cmake/Modules') or die 'trying';

The former was taken directly from a recent version of CMake and stripped down somewhat. 
It should still work with older CMakes.

    open(my $fh, '>', 'cmake/Modules/FindGLEW.cmake') or die "Could not open file 'FindGLEW.cmake' $!";
    print $fh <<'HERE';
    find_path(GLEW_INCLUDE_DIR GL/glew.h)
    find_library(GLEW_LIBRARY NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)
    set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
    set(GLEW_LIBRARIES ${GLEW_LIBRARY})
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

The latter was swiped courtesy of GitHub user [tcbrindle](https://raw.githubusercontent.com/tcbrindle/sdl2-cmake-scripts/master/FindSDL2.cmake),
and also stripped down.

    open(my $fh, '>', 'cmake/Modules/FindSDL2.cmake') or die "Could not open file 'FindSDL2.cmake' $!";
		print $fh <<'THERE';
		message("<FindSDL2.cmake>")
		SET(SDL2_SEARCH_PATHS
			~/Library/Frameworks
			/Library/Frameworks
			/usr/local
			/usr
			/sw # Fink
			/opt/local # DarwinPorts
			/opt/csw # Blastwave
			/opt
			${SDL2_PATH}
		)
		FIND_PATH(SDL2_INCLUDE_DIR SDL.h
			HINTS
			$ENV{SDL2DIR}
			PATH_SUFFIXES include/SDL2 include
			PATHS ${SDL2_SEARCH_PATHS}
		)
		FIND_LIBRARY(SDL2_LIBRARY_TEMP
			NAMES SDL2
			HINTS
			$ENV{SDL2DIR}
			PATH_SUFFIXES lib64 lib
			PATHS ${SDL2_SEARCH_PATHS}
		)
		IF(NOT SDL2_BUILDING_LIBRARY)
			IF(NOT ${SDL2_INCLUDE_DIR} MATCHES ".framework")
				# Non-OS X framework versions expect you to also dynamically link to
				# SDL2main. This is mainly for Windows and OS X. Other (Unix) platforms
				# seem to provide SDL2main for compatibility even though they don't
				# necessarily need it.
				FIND_LIBRARY(SDL2MAIN_LIBRARY
					NAMES SDL2main
					HINTS
					$ENV{SDL2DIR}
					PATH_SUFFIXES lib64 lib
					PATHS ${SDL2_SEARCH_PATHS}
				)
			ENDIF(NOT ${SDL2_INCLUDE_DIR} MATCHES ".framework")
		ENDIF(NOT SDL2_BUILDING_LIBRARY)
		IF(NOT APPLE)
			FIND_PACKAGE(Threads)
		ENDIF(NOT APPLE)
		IF(MINGW)
			SET(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
		ENDIF(MINGW)
		IF(SDL2_LIBRARY_TEMP)
			# For SDL2main
			IF(NOT SDL2_BUILDING_LIBRARY)
				IF(SDL2MAIN_LIBRARY)
					SET(SDL2_LIBRARY_TEMP ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY_TEMP})
				ENDIF(SDL2MAIN_LIBRARY)
			ENDIF(NOT SDL2_BUILDING_LIBRARY)
			IF(APPLE)
				SET(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} "-framework Cocoa")
			ENDIF(APPLE)
			IF(NOT APPLE)
				SET(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} ${CMAKE_THREAD_LIBS_INIT})
			ENDIF(NOT APPLE)
			IF(MINGW)
				SET(SDL2_LIBRARY_TEMP ${MINGW32_LIBRARY} ${SDL2_LIBRARY_TEMP})
			ENDIF(MINGW)
			SET(SDL2_LIBRARY ${SDL2_LIBRARY_TEMP} CACHE STRING "Where the SDL2 Library can be found")
			SET(SDL2_LIBRARY_TEMP "${SDL2_LIBRARY_TEMP}" CACHE INTERNAL "")
		ENDIF(SDL2_LIBRARY_TEMP)
		message("</FindSDL2.cmake>")
		INCLUDE(FindPackageHandleStandardArgs)
		FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2 REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR)
		THERE
    close $fh;

