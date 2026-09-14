if(NOT CMAKE_DUSK_COMPILER)
  if(NOT "$ENV{DUSKC}" STREQUAL "")
    get_filename_component(CMAKE_DUSK_COMPILER_INIT "$ENV{DUSKC}" PROGRAM)
  endif()

  if(CMAKE_DUSK_COMPILER_INIT)
    set(CMAKE_DUSK_COMPILER "${CMAKE_DUSK_COMPILER_INIT}" CACHE FILEPATH "Dusk compiler")
  else()
    find_program(CMAKE_DUSK_COMPILER NAMES duskc DOC "Dusk compiler")
  endif()

  mark_as_advanced(CMAKE_DUSK_COMPILER)
endif()

if(NOT CMAKE_DUSK_COMPILER)
  message(FATAL_ERROR "No Dusk compiler found. Set CMAKE_DUSK_COMPILER or the DUSKC environment variable to the duskc binary.")
endif()

set(CMAKE_DUSK_SOURCE_FILE_EXTENSIONS dsk)
set(CMAKE_DUSK_OUTPUT_EXTENSION .o)
set(CMAKE_DUSK_COMPILER_ENV_VAR "DUSKC")
set(CMAKE_DUSK_COMPILER_ID "DuskC")
set(CMAKE_DUSK_COMPILER_ID_RUN TRUE)
set(CMAKE_DUSK_COMPILER_LOADED 1)

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/CMakeDUSKCompiler.cmake.in
  ${CMAKE_PLATFORM_INFO_DIR}/CMakeDUSKCompiler.cmake
)
