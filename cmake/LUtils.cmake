include(ExternalProject)

ExternalProject_Add(LUtils
PREFIX "${PROJECT_BINARY_DIR}/LUtils"
GIT_REPOSITORY http://git.ursaminorbeta.org.uk/luketpickering/lukes-utils.git
CMAKE_ARGS
-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
-DFORCECPP03=YES)

set(LUTILS_INCLUDE_DIRS ${PROJECT_BINARY_DIR}/LUtils/src/LUtils-build/Linux/include)
set(LUTILS_LIB ${PROJECT_BINARY_DIR}/LUtils/src/LUtils-build/Linux/lib/libLUtils.a)
