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

# Compiler-specific C++11 activation.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (NOT (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8))
        message(STATUS "[NOTE]: If you are trying to a use a non-system compiler, CMake is too clever for you. You must manually pass \'-DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_C_COMPILER=$(which gcc)\' when configuring (You should flatten this build directory and start again unless you know how to clear CMake cache variables).")
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.8 or greater (c++11 support).")
    endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else ()
    message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")

if(${USELINKEDPLUGINS})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLINKEDPLUGINS")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DLINKEDPLUGINS")

endif()

message(STATUS "C++ Compiler      : " ${CXX_COMPILER_NAME})
message(STATUS "    Release flags : " ${CMAKE_CXX_FLAGS_RELEASE})
message(STATUS "    Debug flags   : " ${CMAKE_CXX_FLAGS_DEBUG})


##################################  ROOT  ######################################
if ( NOT DEFINED ENV{ROOTSYS} )
  message (FATAL_ERROR "$ROOTSYS is not defined, please set up root first.")
endif()

find_program (ROOTCONFIG root-config PATHS $ENV{ROOTSYS}/bin)

if ( ROOTCONFIG-NOTFOUND )
  message (FATAL_ERROR "root-config could not be found, is ROOT built in $ROOTSYS = $ENV{ROOTSYS}")
else()
  message( STATUS "Found root-config: ${ROOTCONFIG}")
  execute_process (COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process (COMMAND root-config --libs OUTPUT_VARIABLE ROOT_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  message ( STATUS "root-config --cflags: " ${ROOT_CXX_FLAGS} )
  message ( STATUS "root-config --libs: " ${ROOT_LD_FLAGS} )
  set(ROOTSYS $ENV{ROOTSYS})
endif()

################################  LUtils  ######################################
include(${PROJECT_SOURCE_DIR}/cmake/LUtils.cmake)
###########################  GiBUUToStdHep  ####################################

add_executable(GiBUUToStdHep src/GiBUUToStdHep.cxx src/GiBUUToStdHep_Utils.cxx src/GiRooTracker.cxx)
target_include_directories(GiBUUToStdHep PUBLIC ${CMAKE_INSTALL_PREFIX}/include ${LUTILS_INCLUDE_DIRS} ./)
set_target_properties(GiBUUToStdHep PROPERTIES COMPILE_FLAGS ${ROOT_CXX_FLAGS})
add_dependencies(GiBUUToStdHep LUtils)
target_link_libraries(GiBUUToStdHep ${LUTILS_LIB})
set_target_properties(GiBUUToStdHep PROPERTIES LINK_FLAGS ${ROOT_LD_FLAGS})

add_executable(GiBUUFluxTools src/GiBUUFluxTools.cxx)
target_include_directories(GiBUUFluxTools PUBLIC ${CMAKE_INSTALL_PREFIX}/include ${LUTILS_INCLUDE_DIRS} ./)
set_target_properties(GiBUUFluxTools PROPERTIES COMPILE_FLAGS ${ROOT_CXX_FLAGS})
add_dependencies(GiBUUFluxTools LUtils)
target_link_libraries(GiBUUFluxTools ${LUTILS_LIB})
set_target_properties(GiBUUFluxTools PROPERTIES LINK_FLAGS ${ROOT_LD_FLAGS})

include(${PROJECT_SOURCE_DIR}/cmake/GiBUU.cmake)

configure_file(${PROJECT_SOURCE_DIR}/cmake/toconfigure/setup.sh.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/setup.sh" @ONLY)
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/setup.sh" DESTINATION ${CMAKE_INSTALL_PREFIX})

install(TARGETS GiBUUToStdHep GiBUUFluxTools DESTINATION bin)


############################### Doxygen  #######################################
include(${PROJECT_SOURCE_DIR}/cmake/docsSetup.cmake)
