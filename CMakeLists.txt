cmake_minimum_required(VERSION 2.8)

project("PCM Solitaire")

#Set the version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

#Activate warnings
if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=native -fopenmp -std=c++0x -pedantic -Wall -Wextra -Wno-long-long")
endif()

set(EXECUTABLE_OUTPUT_PATH bin)

file(
	GLOB_RECURSE
	solitaire_source_files
	src/solitaire.cpp
)

file(
	GLOB_RECURSE
	main_source_files
	src/main.cpp
)

add_executable(solitaire ${solitaire_source_files})
add_executable(main ${main_source_files})
