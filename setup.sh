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

unset SETUPDIR
