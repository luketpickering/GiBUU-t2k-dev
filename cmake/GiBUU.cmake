if (DEFINED USE_GIBUU AND USE_GIBUU)
    set(USE_GIBUU 1)
  include(ExternalProject)

  SET(GIBUUVER 6910)

  if(${GIBUUVER} EQUAL 6910)
    SET(PATCHNAME GiBUU2016_ArbFlux_ProdCharg.patch)
  endif()

  ExternalProject_Add(GiBUU
    PREFIX "${CMAKE_INSTALL_PREFIX}/GiBUUInstall/GiBUU2016"
    SVN_REPOSITORY http://gibuu.hepforge.org/svn/releases/release2016
    SVN_REVISION -r ${GIBUUVER}
    UPDATE_COMMAND ""
    PATCH_COMMAND cd ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/GiBUU2016/src/GiBUU && patch -p1 --ignore-whitespace < ${PROJECT_SOURCE_DIR}/patches/${PATCHNAME}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND cd ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/GiBUU2016/src/GiBUU && make
    INSTALL_COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/bin/gibuu && cp ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/GiBUU2016/src/GiBUU/objects/GiBUU.x ${CMAKE_INSTALL_PREFIX}/bin/gibuu)

  ExternalProject_Add(BUUInput
    PREFIX "${CMAKE_INSTALL_PREFIX}/GiBUUInstall/BUUInput"
    SVN_REPOSITORY http://gibuu.hepforge.org/svn/releases/buuinput2016
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")


SET(ShellScripts
    MakeBUUInputSoftlink
    GiBUUInitNeutrinoJobCard
    Generate_ND280_numuCC_CH_Events_GIBUU
    Generate_DUNE_numuCC_Ar_Events_GIBUU
    Generate_MINERvA_numuCC_CH_Events_GIBUU
    Generate_ArgoNeut_numuCC_Ar_Events_GIBUU
    Generate_MiniBooNE_numuCC_CH2_Events_GIBUU
    Select_CC1pi_GiBUUStdHep)

foreach(script ${ShellScripts})
    message(STATUS "Configuring: ${script}")
    configure_file(${PROJECT_SOURCE_DIR}/cmake/toconfigure/${script}.in
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${script}" @ONLY)
    install(PROGRAMS
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${script}" DESTINATION bin/gibuu)
endforeach()

FILE(GLOB JOBCARDS ${PROJECT_SOURCE_DIR}/jobcards/*.job)
install(FILES ${JOBCARDS} DESTINATION jobcards)

FILE(GLOB COMPDATA ${PROJECT_SOURCE_DIR}/compdata/*.dat)
install(FILES ${COMPDATA} DESTINATION compdata)

FILE(GLOB CINT_MACROS ${PROJECT_SOURCE_DIR}/cint_macros/*.C)
install(FILES ${CINT_MACROS} DESTINATION cint_macros)

else()
    set(USE_GIBUU 0)
endif()
