cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

project (GiBUUToStdHep)

set (GiBUUToStdHep_VERSION_MAJOR 0)
set (GiBUUToStdHep_VERSION_MINOR 9)
set (GiBUUToStdHep_VERSION_REVISION 0)

set (VERBOSE FALSE)

set (CMAKE_BUILD_TYPE DEBUG)

if(DEFINED ENV{INSTALL_PREFIX})
  set(CMAKE_INSTALL_PREFIX $ENV{INSTALL_PREFIX})
else()
  set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_NAME})
endif()

##################################  CXX  #################£#####################
get_filename_component (CXX_COMPILER_NAME ${CMAKE_CXX_COMPILER} NAME)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")

if(${USELINKEDPLUGINS})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLINKEDPLUGINS")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DLINKEDPLUGINS")

endif()

message ("C++ Compiler      : " ${CXX_COMPILER_NAME})
message ("    Release flags : " ${CMAKE_CXX_FLAGS_RELEASE})
message ("    Debug flags   : " ${CMAKE_CXX_FLAGS_DEBUG})


##################################  ROOT  ######################################
if ( NOT DEFINED ENV{ROOTSYS} )
  message (FATAL_ERROR "$ROOTSYS is not defined, please set up root first.")
endif()

include ($ENV{ROOTSYS}/etc/cmake/FindROOT.cmake)
set(ROOTSYS $ENV{ROOTSYS})

if ( NOT ROOT_FOUND )
  message (FATAL_ERROR "root-config could not be found, is ROOT built in $ROOTSYS = $ENV{ROOTSYS}")
else()
  execute_process (COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process (COMMAND root-config --libs OUTPUT_VARIABLE ROOT_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  message ( STATUS "root-config --cflags: " ${ROOT_CXX_FLAGS} )
  message ( STATUS "root-config --libs: " ${ROOT_LD_FLAGS} )
endif()

################################  LUtils  ######################################
include(${PROJECT_SOURCE_DIR}/cmake/LUtils.cmake)
###########################  GiBUUToStdHep  ####################################

add_executable(GiBUUToStdHep src/GiBUUToStdHep.cxx src/GiBUUToStdHep_Utils.cxx src/GiRooTracker.cxx)
target_include_directories(GiBUUToStdHep PUBLIC ${CMAKE_INSTALL_PREFIX}/include ${LUTILS_INCLUDE_DIRS} ./)
set_target_properties(GiBUUToStdHep PROPERTIES COMPILE_FLAGS ${ROOT_CXX_FLAGS})
add_dependencies(GiBUUToStdHep LUtils)
target_link_libraries(GiBUUToStdHep ${CMAKE_INSTALL_PREFIX}/lib/libLUtils.a)
set_target_properties(GiBUUToStdHep PROPERTIES LINK_FLAGS ${ROOT_LD_FLAGS})

add_executable(GiBUUFluxTools src/GiBUUFluxTools.cxx)
target_include_directories(GiBUUFluxTools PUBLIC ${CMAKE_INSTALL_PREFIX}/include ${LUTILS_INCLUDE_DIRS} ./)
set_target_properties(GiBUUFluxTools PROPERTIES COMPILE_FLAGS ${ROOT_CXX_FLAGS})
add_dependencies(GiBUUFluxTools LUtils)
target_link_libraries(GiBUUFluxTools ${CMAKE_INSTALL_PREFIX}/lib/libLUtils.a)
set_target_properties(GiBUUFluxTools PROPERTIES LINK_FLAGS ${ROOT_LD_FLAGS})

include(${PROJECT_SOURCE_DIR}/cmake/GiBUU.cmake)

configure_file(${PROJECT_SOURCE_DIR}/cmake/toconfigure/setup.sh.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/setup.sh" @ONLY)
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/setup.sh" DESTINATION ${CMAKE_INSTALL_PREFIX})

install(TARGETS GiBUUToStdHep GiBUUFluxTools DESTINATION bin)


############################### Doxygen  #######################################
include(${PROJECT_SOURCE_DIR}/cmake/docsSetup.cmake)
