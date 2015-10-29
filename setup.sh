#!/bin/bash

#If it was sourced as . setup.sh then you can't scrub off the end... assume that
#we are in the correct directory.
if ! echo "${BASH_SOURCE}" | grep "/" --silent; then
  SETUPDIR=$(readlink -f $PWD)
else
  SETUPDIR=$(readlink -f ${BASH_SOURCE%/*})
fi

if [[ ! -e utils/.git ]] || [[ ! -e LesHouchesParserClasses_CPP/.git ]]; then
  git submodule init
  git submodule update
fi

if ! [[ ":$PATH:" == *":${SETUPDIR}/GiBUUToStdHep/bin:"* ]]; then
  export PATH=${SETUPDIR}/GiBUUToStdHep/bin:$PATH
fi

if [[ -e "${SETUPDIR}/GiBUUInstall/release1.6/bin/gibuu" ]]; then
  if ! [[ ":$PATH:" == *":${SETUPDIR}/GiBUUInstall/release1.6/bin:"* ]]; then
    export GIBUU=${SETUPDIR}/GiBUUInstall/release1.6
    export PATH=${GIBUU}/bin:${PATH}
    echo "Added patched GiBUU install to PATH @ ${GIBUU}/bin/gibuu"
  fi
fi

unset SETUPDIR
