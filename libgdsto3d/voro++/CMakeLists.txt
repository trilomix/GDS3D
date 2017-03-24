set(SRC
  src/voro++.cc
)

file(GLOB_RECURSE HDR RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
append_gmsh_src(contrib/voro++ "${SRC};${HDR}")
