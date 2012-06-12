project(LogSymmetricDemonsSuperBuild)

#-----------------------------------------------------------------------------
# Requires a recent version of CMake
cmake_minimum_required(VERSION 2.6)

include(ExternalProject)

set(COMMON_EXTERNAL_PROJECT_ARGS
  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
  -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
)

foreach(build_type RELEASE DEBUG RELWITHDEBINFO)
  list(APPEND COMMON_EXTERNAL_PROJECT_ARGS
    -DCMAKE_CXX_FLAGS_${build_type}:STRING=${CMAKE_CXX_FLAGS_${build_type}})
  list(APPEND COMMON_EXTERNAL_PROJECT_ARGS
    -DCMAKE_C_FLAGS_${build_type}:STRING=${CMAKE_C_FLAGS_${build_type}})
endforeach()

include(${CMAKE_CURRENT_LIST_DIR}/External_ITKv4.cmake)

get_filename_component(LogSourceDir ${CMAKE_CURRENT_LIST_DIR} PATH)

ExternalProject_add(LogSymmetricDemons
  SOURCE_DIR ${LogSourceDir}
  BINARY_DIR LogSymmetricDemons-build
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
  CMAKE_ARGS
  ${COMMON_EXTERNAL_PROJECT_ARGS}
  -DITK_DIR:PATH=${ITK_DIR}
  DEPENDS ITKv4
)
