# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\CodeViewer_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CodeViewer_autogen.dir\\ParseCache.txt"
  "CodeViewer_autogen"
  )
endif()
