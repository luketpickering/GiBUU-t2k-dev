if (DEFINED USE_GiBUU AND USE_GiBUU)
    set(USE_GiBUU 1)
  include(ExternalProject)


  if (DEFINED NO_EXTERNAL_UPDATE AND NO_EXTERNAL_UPDATE)
      set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   PROPERTY EP_UPDATE_DISCONNECTED 1)
      message(STATUS "Will not attempt to update third party tools for each build.")
    else()
      set(NO_EXTERNAL_UPDATE 0)
  endif()

  if(GIBUU_PULL_USE_SVN)
    if (NOT DEFINED GIBUUVER)
      # SET(GIBUUVER 6910)
      SET(GIBUUVER 7339)
    endif()

    SET(GIBUUVERNAME "GiBUU2016")
    SET(GIBUUINPREPO "http://gibuu.hepforge.org/svn/releases/release2016")
    SET(BUUINPREPO "http://gibuu.hepforge.org/svn/releases/buuinput2016")
    if(${GIBUUVER} EQUAL 6910)
      SET(PATCHCOMMAND cd ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/${GIBUUVERNAME}/src/GiBUU && patch -p1 --ignore-whitespace < ${PROJECT_SOURCE_DIR}/patches/GiBUU2016_ArbFlux_ProdCharge.patch)
    endif()

    if(${GIBUUVER} EQUAL 7339)
      SET(GIBUUVERNAME "GiBUU2017")
      SET(GIBUUINPREPO "http://gibuu.hepforge.org/svn/releases/release2017")
      SET(BUUINPREPO "http://gibuu.hepforge.org/svn/releases/buuinput2017")
      if (DEFINED PATCHNDK AND PATCHNDK)
        SET(PATCHCOMMAND cd ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/${GIBUUVERNAME}/src/GiBUU && patch -p1 --ignore-whitespace < ${PROJECT_SOURCE_DIR}/patches/GiBUU2017_ArbFlux_ProdCharge.patch && patch -p1 --ignore-whitespace < ${PROJECT_SOURCE_DIR}/patches/ND.GiBUU2017.patch)
      else()
        SET(PATCHCOMMAND cd ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/${GIBUUVERNAME}/src/GiBUU && patch -p1 --ignore-whitespace < ${PROJECT_SOURCE_DIR}/patches/GiBUU2017_ArbFlux_ProdCharge.patch)
      endif()
    endif()

    message(STATUS "Using GiBUU version ${GIBUUVERNAME} from repo ${GIBUUINPREPO}")

    ExternalProject_Add(GiBUU
      PREFIX "${CMAKE_INSTALL_PREFIX}/GiBUUInstall/${GIBUUVERNAME}"
      SVN_REPOSITORY ${GIBUUINPREPO}
      SVN_REVISION -r ${GIBUUVER}
      UPDATE_COMMAND ""
      PATCH_COMMAND ${PATCHCOMMAND}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND cd ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/${GIBUUVERNAME}/src/GiBUU && make
      INSTALL_COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/bin/gibuu && cp ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/${GIBUUVERNAME}/src/GiBUU/objects/GiBUU.x ${CMAKE_INSTALL_PREFIX}/bin/gibuu)

    ExternalProject_Add(BUUInput
      PREFIX "${CMAKE_INSTALL_PREFIX}/GiBUUInstall/BUUInput"
      SVN_REPOSITORY ${BUUINPREPO}
      UPDATE_COMMAND ""
      PATCH_COMMAND ""
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND "")

  else() #Use git to check out GiBUU2017
    SET(GIBUUVERNAME "GiBUU2017")
    if (DEFINED PATCHNDK AND PATCHNDK)
      SET(PATCHCOMMAND cd ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/${GIBUUVERNAME}/src/GiBUU && patch -p1 --ignore-whitespace < ${PROJECT_SOURCE_DIR}/patches/GiBUU2017_ArbFlux_ProdCharge.patch && patch -p1 --ignore-whitespace < ${PROJECT_SOURCE_DIR}/patches/NDK.GiBUU2017.patch)
    else()
      SET(PATCHCOMMAND cd ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/${GIBUUVERNAME}/src/GiBUU && patch -p1 --ignore-whitespace < ${PROJECT_SOURCE_DIR}/patches/GiBUU2017_ArbFlux_ProdCharge.patch)
    endif()

    ExternalProject_Add(GiBUU
      PREFIX "${CMAKE_INSTALL_PREFIX}/GiBUUInstall/${GIBUUVERNAME}"
      GIT_REPOSITORY https://github.com/janusw/GiBUU_2017.git
      UPDATE_COMMAND ""
      PATCH_COMMAND ${PATCHCOMMAND}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND cd ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/${GIBUUVERNAME}/src/GiBUU && make
      INSTALL_COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/bin/gibuu && cp ${CMAKE_INSTALL_PREFIX}/GiBUUInstall/${GIBUUVERNAME}/src/GiBUU/objects/GiBUU.x ${CMAKE_INSTALL_PREFIX}/bin/gibuu)

    ExternalProject_Add(BUUInput
      PREFIX "${CMAKE_INSTALL_PREFIX}/GiBUUInstall/BUUInput"
      GIT_REPOSITORY https://github.com/janusw/buuinput_2017.git
      UPDATE_COMMAND ""
      PATCH_COMMAND ""
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND "")

  endif()

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

  FILE(GLOB BATCHJOBS_CARDS ${PROJECT_SOURCE_DIR}/batchjobs/*.in)
  install(FILES ${BATCHJOBS_CARDS} DESTINATION batchjobs)

  FILE(GLOB BATCHJOBS_FLUX ${PROJECT_SOURCE_DIR}/batchjobs/fluxes/*.*)
  install(FILES ${BATCHJOBS_FLUX} DESTINATION batchjobs/fluxes)

  FILE(GLOB BATCHJOBS_SCRIPTS ${PROJECT_SOURCE_DIR}/batchjobs/*.sh)
  install(PROGRAMS ${BATCHJOBS_SCRIPTS} DESTINATION batchjobs)

else()
    set(USE_GiBUU 0)
endif()
