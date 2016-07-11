#!/bin/bash

#If it was sourced as . setup.sh then you can't scrub off the end... assume that
#we are in the correct directory.
if ! echo "${BASH_SOURCE}" | grep "/" --silent; then
  SETUPDIR=$(readlink -f $PWD)
else
  SETUPDIR=$(readlink -f ${BASH_SOURCE%/*})
fi

if [[ ! -e ${SETUPDIR}/utils/.git ]] || [[ ! -e ${SETUPDIR}/LesHouchesParserClasses_CPP/.git ]]; then
  cd ${SETUPDIR}
  git submodule init
  git submodule update
  cd -
fi

if ! [[ ":$PATH:" == *":${SETUPDIR}/GiBUUTools/bin:"* ]]; then
  export PATH=${SETUPDIR}/GiBUUTools/bin:$PATH
fi

if [[ -e "${SETUPDIR}/GiBUUInstall/release2016/bin/gibuu"]]; then
  if ! [[ ":$PATH:" == *":${SETUPDIR}/GiBUUInstall/release2016/bin:"* ]]; then
    export GIBUU=${SETUPDIR}/GiBUUInstall/release2016
    export GIBUUINPUTS=${SETUPDIR}/GiBUUInstall/buuinput_2016
    export PATH=${GIBUU}:$PATH
  fi
fi

unset SETUPDIR
