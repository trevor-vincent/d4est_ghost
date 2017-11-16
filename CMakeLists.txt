#Compile command: cd ../build && rm -rf ./* && cmake ../src -DDO_NOT_BUNDLE_P4EST=1 -DDO_NOT_BUNDLE_ZLIB=1 -DCMAKE_BUILD_TYPE=Debug && make

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)

project(d4est_ghost)

#HARDCODED FOR NOW
ADD_DEFINITIONS("-DD4EST_DIM=3")

if(DO_NOT_BUNDLE_P4EST)
  SET(P4EST_INCLUDE_DIRS "/home/tvincent/Dropbox/Research/Codes/disco4est/src_new_build/ThirdParty/p4est/install/include")
  SET(P4EST_LIBRARIES "/home/tvincent/Dropbox/Research/Codes/disco4est/src_new_build/ThirdParty/p4est/install/lib/libp4est.a" "/home/tvincent/Dropbox/Research/Codes/disco4est/src_new_build/ThirdParty/p4est/install/lib/libsc.a")
endif()


if(DO_NOT_BUNDLE_ZLIB)
  SET(ZLIB_INCLUDE_DIRS "/home/tvincent/Dropbox/Research/Codes/disco4est/src_new_build/ThirdParty/zlib/install/include")
  SET(ZLIB_LIBRARIES  "/home/tvincent/Dropbox/Research/Codes/disco4est/src_new_build/ThirdParty/zlib/install/lib/libz.a")
endif()

find_package(MPI REQUIRED)

include_directories(${P4EST_INCLUDE_DIRS})  
include_directories(${ZLIB_INCLUDE_DIRS})  
include_directories(${CMAKE_SOURCE_DIR}/Dictionary)
include_directories(${CMAKE_SOURCE_DIR}/Utilities)
include_directories(${CMAKE_SOURCE_DIR}/Ghosts)
include_directories(${CMAKE_SOURCE_DIR}/Base)
include_directories(${CMAKE_SOURCE_DIR}/Mesh)
include_directories(${MPI_C_INCLUDE_PATH})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MPI_C_COMPILE_FLAG}")

SET (
  SRCS
  Utilities/d4est_util.c
  Dictionary/d4est_dictionary.c
  Dictionary/d4est_critbit.c
  Mesh/d4est_mesh_data.c
  Mesh/d4est_mesh.c
  Mesh/d4est_element_data.c
  Ghosts/d4est_ghost.c
  Ghosts/d4est_ghost_data.c
  Base/d4est_base.c
  
  # Mesh/d4est
  )

add_library(d4est_ghost ${SRCS})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_C_LINK_FLAGS}")

SET (
  LIBS
  ${P4EST_LIBRARIES}
  ${MPI_C_LIBRARIES}
  m
  dl
  ${ZLIB_LIBRARIES}
  )

message("LIBS=${LIBS}")
target_link_libraries(d4est_ghost ${LIBS})

add_executable(test_d4est_mesh_data ${CMAKE_SOURCE_DIR}/test_d4est_mesh_data.c)
target_link_libraries(test_d4est_mesh_data d4est_ghost)

add_executable(test_d4est_ghost_data ${CMAKE_SOURCE_DIR}/test_d4est_ghost_data.c)
target_link_libraries(test_d4est_ghost_data d4est_ghost)


# target_compile_definitions(${exec} PRIVATE DIM=2)
