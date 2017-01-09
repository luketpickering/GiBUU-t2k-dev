include(ExternalProject)

if (DEFINED NO_EXTERNAL_UPDATE AND NO_EXTERNAL_UPDATE)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                 PROPERTY EP_UPDATE_DISCONNECTED 1)
    cmessage(STATUS "Will not attempt to update third party tools for each build.")
  else()
    set(NO_EXTERNAL_UPDATE 0)
endif()

ExternalProject_Add(LUtils
PREFIX "${PROJECT_BINARY_DIR}/LUtils"
GIT_REPOSITORY http://git.ursaminorbeta.org.uk/luketpickering/lukes-utils.git
CMAKE_ARGS
-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
-DFORCECPP03=YES)

set(LUTILS_INCLUDE_DIRS ${PROJECT_BINARY_DIR}/LUtils/src/LUtils-build/Linux/include)
set(LUTILS_LIB ${PROJECT_BINARY_DIR}/LUtils/src/LUtils-build/Linux/lib/libLUtils.a)
