if(NOT CMAKE_DUSK_COMPILER_WORKS)
  message(STATUS "Check for working Dusk compiler: ${CMAKE_DUSK_COMPILER}")
  set(CMAKE_DUSK_COMPILER_WORKS 1 CACHE INTERNAL "")
  message(STATUS "Check for working Dusk compiler: ${CMAKE_DUSK_COMPILER} - works")
endif()

set(CMAKE_DUSK_ABI_COMPILED 1)
