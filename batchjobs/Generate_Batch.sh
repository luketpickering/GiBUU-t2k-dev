#!/bin/sh

FLUX_FILE=""
WSB_FLUX_FILE=""
INPUT_JOBCARD="${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in"
FLUX_IS_ANTI="0"
N_CC_JOBS="1"
TARGET_A="12"
TARGET_Z="6"
USE_NC="0"
N_H_IN_COMPOSITE="0"
JOB_NAME="gibuu_gen"
DENSITY_SWITCH="2"

#Overall run options
USE_OSET_INMED_BROAD=".true."
FIX_BE=".true."
N_RUNS="10"
N_ENSEMBLES="4000"
N_H_ENSEMBLES=$(python -c "print ${N_ENSEMBLES};")

SCRIPTNAME="${0}"

while [[ ${#} -gt 0 ]]; do

  key="$1"
  case $key in

      -F|--flux-file)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      FLUX_FILE="$2"
      echo "[OPT]: Reading flux from ${FLUX_FILE}"
      shift # past argument
      ;;

      -A|--flux-is-anti-nu)

      FLUX_IS_ANTI="1"
      ;;

      -w|--wrong-sign-flux-file)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      WSB_FLUX_FILE="$2"
      echo "[OPT]: Reading wrong sign flux from ${WSB_FLUX_FILE}"
      shift # past argument
      ;;

      -n|--num-cc-jobs)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      N_CC_JOBS="$2"
      echo "[OPT]: Running ${N_CC_JOBS} CC right sign flux jobs"
      shift # past argument
      ;;

      -a|--target-a)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      TARGET_A="$2"
      echo "[OPT]: Main target has nucleon number: ${TARGET_A}"
      shift # past argument
      ;;

      -z|--target-z)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      TARGET_Z="$2"
      echo "[OPT]: Main target has proton number: ${TARGET_Z}"
      shift # past argument
      ;;

      -N|--do-NC)
      USE_NC="1"
      echo "[OPT]: Running NC jobs"
      ;;

      -W|--woods-saxon-density)
      DENSITY_SWITCH="3"
      echo "[OPT]: Using Woods Saxon density"
      ;;

      -c|--no-const-binding-energy)
      DENSITY_SWITCH="3"
      echo "[OPT]: Not re-adjusting for constant binding energy."
      FIX_BE=".false."
      ;;

      -h|--num-h-in-target)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      N_H_IN_COMPOSITE="$2"
      echo "[OPT]: Using a Hydrogen composite target with ${N_H_IN_COMPOSITE} Hydrogen components"
      shift
      ;;

      -i|--input-job-card)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      INPUT_JOBCARD="$2"
      echo "[OPT]: Specialising the input jobcard: ${INPUT_JOBCARD}"
      shift
      ;;

      -J|--job-name)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      JOB_NAME="$2"
      echo "[OPT]: Using JobName: ${JOB_NAME}"
      shift
      ;;

      -B|--no-oset-broadening)
      USE_OSET_INMED_BROAD=".false."
      ;;

      -?|--help)

      echo "[RUNLIKE] ${SCRIPTNAME}"
      echo "         -?|--help <type=default_value>: Display this help message"
      echo "         -F|--flux-file  </path/to/input/flux>:"
      echo "         -w|--wrong-sign-flux-file  </path/to/input/wrong_sign_flux>:"
      echo "         -A|--flux-is-anti-nu : Main flux component is anti-numu (NP/RHC/NF)"
      echo "         -n|--num-cc-jobs <int=1> : The number of CC main flux component jobs to run"
      echo "         -a|--target-a <int=12> : Needs to be known for correct stdhep event output"
      echo "         -z|--target-z <int=6> : Needs to be known for correct stdhep event output"
      echo "         -N|--do-NC : Will also run NC jobs"
      echo "         -h|--num-h-in-target <int=0>: Whether to add any free protons to the composite target, useful for generating CHx targets"
      echo "         -i|--input-job-card </path/to/input/jobcard=${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in>: input jobcard to specialise"
      echo "         -J|--job-name <jobname=gibuu_gen> : The name of this job, used for directory structure"
      echo "         -W|--woods-saxon-density : Use Woods Saxon density, for targets, such as Ar40, that do not have an NPA 554 parameterisation"
      echo "         -c|--no-const-binding-energy : Do not re-adjust nuclear potential for a constant binding energy. Useful for Deuterium targets."
      echo "         -B|--no-oset-broadening : Do not use collisional broadening of baryonic resonances"
      exit 0
      ;;

      *)
              # unknown option
      echo "Unknown option $1"
      exit 1
      ;;
  esac
  shift # past argument or value
done

################################################################################
##                               Check CLI options
################################################################################
if [[ ! ${FLUX_FILE} ]]; then
  echo "[ERROR]: Expected to be passed at least -F \"/path/to/flux/file\"."
  exit 1
fi

if [[ ! -e ${FLUX_FILE} ]]; then
  echo "[ERROR]: Flux file \"${FLUX_FILE}\" does not exist."
  exit 1
fi

NLinesFlux=$(cat ${FLUX_FILE} | wc -l)
if [[ ${NLinesFlux} -gt 210 ]]; then
  echo "[ERROR]: The input flux file: ${FLUX_FILE} has too many lines (${NLinesFlux}), GiBUU can read files with a maximum of 210 lines. Please shorten it."
  exit 1
fi

if [[ ! -e ${GIBUUTOOLSROOT}/setup.sh ]]; then
  echo "[ERROR]: Expected to find a GiBUU environment set up script at \"\$GIBUUTOOLSROOT/setup.sh\". Is this sourced?"
  exit 1
fi

hash GiBUU.x &> /dev/null
if [[ "${?}" != "0" ]]; then
  echo "[ERROR]: Expected to find the GiBUU executable, GiBUU.x, but it wasn't available."
  exit 1
fi

if [[ "${WSB_FLUX_FILE}" ]]; then
  if [[ ! -e "${WSB_FLUX_FILE}" ]]; then
    echo "[ERROR]: Wrong sign flux file: \"${WSB_FLUX_FILE}\" could not be found."
    exit 1
  else
    NLinesFluxWSB=$(cat ${WSB_FLUX_FILE} | wc -l)
    if [[ ${NLinesFlux} -gt 210 ]]; then
      echo "[ERROR]: The input flux file: ${WSB_FLUX_FILE} has too many lines (${NLinesFluxWSB}), GiBUU can read files with a maximum of 210 lines. Please shorten it."
      exit 1
    fi
  fi
fi

if [[ ! -e "${INPUT_JOBCARD}" ]]; then
  echo "[ERROR]: Specified input job card \"${INPUT_JOBCARD}\" does not exist."
  exit 1
fi
INPUT_JOBCARD=$(readlink -f ${INPUT_JOBCARD})

if [[ "${FLUX_IS_ANTI}" == "1" ]]; then
 FLUX_CC_NU_SPEC="-2"
 FLUX_NC_NU_SPEC="-3"
 WSB_FLUX_CC_NU_SPEC="2"
 WSB_FLUX_NC_NU_SPEC="3"
 FLUX_NAME="numub_flux"
 WSB_FLUX_NAME="numu_flux"
 NU_SPECIES="-14"
 WSB_NU_SPECIES="14"
else # right sign is numu
 FLUX_CC_NU_SPEC="2"
 FLUX_NC_NU_SPEC="3"
 WSB_FLUX_CC_NU_SPEC="-2"
 WSB_FLUX_NC_NU_SPEC="-3"
 FLUX_NAME="numu_flux"
 WSB_FLUX_NAME="numub_flux"
 NU_SPECIES="14"
 WSB_NU_SPECIES="-14"
fi

################################################################################
################################################################################
mkdir ${JOB_NAME}
cd ${JOB_NAME}

N_CC_JOBS_WSB="0"
if [[ "${WSB_FLUX_FILE}" ]]; then
  N_CC_JOBS_WSB=$(python -c "from math import ceil; print int(ceil(float(${N_CC_JOBS})/float(10)));")
  echo "[INFO]: Farming ${N_CC_JOBS_WSB} CC wrong sign background jobs."
fi

N_NC_JOBS="0"
N_NC_JOBS_WSB="0"
if [[ "${USE_NC}" != "0" ]]; then
  N_NC_JOBS=$(python -c "from math import ceil; print int(ceil(float(${N_CC_JOBS})/float(5)));")
  echo "[INFO]: Farming ${N_NC_JOBS} NC Jobs."
  if [[ "${WSB_FLUX_FILE}" ]]; then
    N_NC_JOBS_WSB=$(python -c "from math import ceil; print int(ceil(float(${N_CC_JOBS_WSB})/float(5)));")
    echo "[INFO]: Farming ${N_NC_JOBS_WSB} NC wrong sign background jobs."
  fi
fi



HOLD_JID=""

################################################################################
##             Generate: Main target CC
################################################################################
mkdir CC_Flux; cd CC_Flux

cp ${FLUX_FILE} flux.txt
cp ${INPUT_JOBCARD} jobcard.in

################################################################################
##                         Build Replacements: Main target CC
################################################################################
echo -e "__NU_FLAVOR_CODE__ 2\n\
__NU_INTERACTION_TYPE__ ${FLUX_CC_NU_SPEC}\n\
__FLUX_FILE__ ./flux.txt\n\
__TARGET_A__ ${TARGET_A}\n\
__TARGET_Z__ ${TARGET_Z}\n\
__FIX_BE__ ${FIX_BE}\n\
__N_ENSEMBLE__ ${N_ENSEMBLES}\n\
__N_TSTEPS__ 150\n\
__N_RUNS__ ${N_RUNS}\n\
__HIGH_FLUX_CUT__ 50\n\
__DENSITY_SWITCH__ ${DENSITY_SWITCH}\n\
__OSET_DELTA_BROAD__ ${USE_OSET_INMED_BROAD}\n\
__BUU_INPUT__ ./BUUInput" > job.rpl

################################################################################
##                         Build jobcard: Main target CC
################################################################################

TMPFILEA=jobcard.in_tmp1
TMPFILEB=jobcard.in_tmp2
cp jobcard.in ${TMPFILEA}

while read ln; do
  FIRST=$(echo ${ln} | cut -d " " -f 1)
  SECOND=$(echo ${ln} | cut -d " " -f 2)
  echo -e "\tReplacing ${FIRST} with ${SECOND}"
  cat ${TMPFILEA} | sed "s:${FIRST}:${SECOND}:" > ${TMPFILEB}
  mv ${TMPFILEB} ${TMPFILEA}
done < job.rpl

mv ${TMPFILEA} job.card

################################################################################
##                         Farm jobs: Main target CC
################################################################################

Flux_CC_JID_MSG=$(qsub -N Gen_F_CC -v GIBUUTOOLSROOT=${GIBUUTOOLSROOT} -t 1-${N_CC_JOBS} ${GIBUUTOOLSROOT}/batchjobs/RunGiBUUBatch.sh)
Flux_CC_JID=$(echo "${Flux_CC_JID_MSG}" | sed "s|^Your job-array \([0-9]\+\)\..*|\1|g")
echo "[INFO]: Flux_CC jobs farmed with JID: ${Flux_CC_JID}"

HOLD_JID="${Flux_CC_JID}"

cd ../


if [[ "${N_NC_JOBS}" != "0" ]]; then
################################################################################
##             Generate: Main target NC
################################################################################
  mkdir NC_Flux; cd NC_Flux

  cp ${FLUX_FILE} flux.txt
  cp ${INPUT_JOBCARD} jobcard.in

################################################################################
##                         Build Replacements: Main target NC
################################################################################
  echo -e "__NU_FLAVOR_CODE__ 2\n\
  __NU_INTERACTION_TYPE__ ${FLUX_NC_NU_SPEC}\n\
  __FLUX_FILE__ ./flux.txt\n\
  __TARGET_A__ ${TARGET_A}\n\
  __TARGET_Z__ ${TARGET_Z}\n\
  __FIX_BE__ ${FIX_BE}\n\
  __N_ENSEMBLE__ ${N_ENSEMBLES}\n\
  __N_TSTEPS__ 150\n\
  __N_RUNS__ ${N_RUNS}\n\
  __HIGH_FLUX_CUT__ 50\n\
  __DENSITY_SWITCH__ ${DENSITY_SWITCH}\n\
  __OSET_DELTA_BROAD__ ${USE_OSET_INMED_BROAD}\n\
  __BUU_INPUT__ ./BUUInput" > job.rpl

################################################################################
##                         Build jobcard: Main target NC
################################################################################

  TMPFILEA=jobcard.in_tmp1
  TMPFILEB=jobcard.in_tmp2
  cp jobcard.in ${TMPFILEA}

  while read ln; do
    FIRST=$(echo ${ln} | cut -d " " -f 1)
    SECOND=$(echo ${ln} | cut -d " " -f 2)
    echo -e "\tReplacing ${FIRST} with ${SECOND}"
    cat ${TMPFILEA} | sed "s:${FIRST}:${SECOND}:" > ${TMPFILEB}
    mv ${TMPFILEB} ${TMPFILEA}
  done < job.rpl

  mv ${TMPFILEA} job.card

################################################################################
##                         Farm jobs: Main target NC
################################################################################

  Flux_NC_JID_MSG=$(qsub -N Gen_F_NC -v GIBUUTOOLSROOT=${GIBUUTOOLSROOT} -t 1-${N_NC_JOBS} ${GIBUUTOOLSROOT}/batchjobs/RunGiBUUBatch.sh)
  Flux_NC_JID=$(echo "${Flux_NC_JID_MSG}" | sed "s|^Your job-array \([0-9]\+\)\..*|\1|g")
  echo "[INFO]: Flux_NC jobs farmed with JID: ${Flux_NC_JID}"

  HOLD_JID="${HOLD_JID} ${Flux_NC_JID}"

  cd ../
fi # end N_NC_JOBS

if [[ "${N_H_IN_COMPOSITE}" != "0" ]]; then

  ################################################################################
  ##             Generate: H target CC
  ################################################################################
  mkdir CC_Flux_H; cd CC_Flux_H

  cp ${FLUX_FILE} flux.txt
  cp ${INPUT_JOBCARD} jobcard.in

  ################################################################################
  ##                         Build Replacements: H target CC
  ################################################################################
  echo -e "__NU_FLAVOR_CODE__ 2\n\
  __NU_INTERACTION_TYPE__ ${FLUX_CC_NU_SPEC}\n\
  __FLUX_FILE__ ./flux.txt\n\
  __TARGET_A__ 1\n\
  __TARGET_Z__ 1\n\
  __FIX_BE__ .false.\n\
  __N_ENSEMBLE__ ${N_H_ENSEMBLES}\n\
  __N_TSTEPS__ 0\n\
  __N_RUNS__ ${N_RUNS}\n\
  __HIGH_FLUX_CUT__ 50\n\
  __DENSITY_SWITCH__ ${DENSITY_SWITCH}\n\
  __OSET_DELTA_BROAD__ .false.\n\
  __BUU_INPUT__ ./BUUInput" > job.rpl

  ################################################################################
  ##                         Build jobcard: H target CC
  ################################################################################

  TMPFILEA=jobcard.in_tmp1
  TMPFILEB=jobcard.in_tmp2
  cp jobcard.in ${TMPFILEA}

  while read ln; do
    FIRST=$(echo ${ln} | cut -d " " -f 1)
    SECOND=$(echo ${ln} | cut -d " " -f 2)
    echo -e "\tReplacing ${FIRST} with ${SECOND}"
    cat ${TMPFILEA} | sed "s:${FIRST}:${SECOND}:" > ${TMPFILEB}
    mv ${TMPFILEB} ${TMPFILEA}
  done < job.rpl

  mv ${TMPFILEA} job.card

  ################################################################################
  ##                         Farm jobs: H target CC
  ################################################################################

  Flux_CC_H_JID_MSG=$(qsub -N Gen_F_CC_H -v GIBUUTOOLSROOT=${GIBUUTOOLSROOT} -t 1-${N_CC_JOBS} ${GIBUUTOOLSROOT}/batchjobs/RunGiBUUBatch.sh)
  Flux_CC_H_JID=$(echo "${Flux_CC_H_JID_MSG}" | sed "s|^Your job-array \([0-9]\+\)\..*|\1|g")
  echo "[INFO]: Flux_CC H jobs farmed with JID: ${Flux_CC_H_JID}"

  HOLD_JID="${HOLD_JID} ${Flux_CC_H_JID}"

  cd ../


  if [[ "${N_NC_JOBS}" != "0" ]]; then
  ################################################################################
  ##             Generate: H target NC
  ################################################################################
    mkdir NC_Flux_H; cd NC_Flux_H

    cp ${FLUX_FILE} flux.txt
    cp ${INPUT_JOBCARD} jobcard.in

  ################################################################################
  ##                         Build Replacements: H target NC
  ################################################################################
    echo -e "__NU_FLAVOR_CODE__ 2\n\
    __NU_INTERACTION_TYPE__ ${FLUX_NC_NU_SPEC}\n\
    __FLUX_FILE__ ./flux.txt\n\
    __TARGET_A__ 1\n\
    __TARGET_Z__ 1\n\
    __FIX_BE__ .false.\n\
    __N_ENSEMBLE__ ${N_H_ENSEMBLES}\n\
    __N_TSTEPS__ 0\n\
    __N_RUNS__ ${N_RUNS}\n\
    __HIGH_FLUX_CUT__ 50\n\
    __DENSITY_SWITCH__ ${DENSITY_SWITCH}\n\
    __OSET_DELTA_BROAD__ .false.\n\
    __BUU_INPUT__ ./BUUInput" > job.rpl

  ################################################################################
  ##                         Build jobcard: H target NC
  ################################################################################

    TMPFILEA=jobcard.in_tmp1
    TMPFILEB=jobcard.in_tmp2
    cp jobcard.in ${TMPFILEA}

    while read ln; do
      FIRST=$(echo ${ln} | cut -d " " -f 1)
      SECOND=$(echo ${ln} | cut -d " " -f 2)
      echo -e "\tReplacing ${FIRST} with ${SECOND}"
      cat ${TMPFILEA} | sed "s:${FIRST}:${SECOND}:" > ${TMPFILEB}
      mv ${TMPFILEB} ${TMPFILEA}
    done < job.rpl

    mv ${TMPFILEA} job.card

  ################################################################################
  ##                         Farm jobs: H target NC
  ################################################################################

    Flux_NC_H_JID_MSG=$(qsub -N Gen_F_NC_H -v GIBUUTOOLSROOT=${GIBUUTOOLSROOT} -t 1-${N_NC_JOBS} ${GIBUUTOOLSROOT}/batchjobs/RunGiBUUBatch.sh)
    Flux_NC_H_JID=$(echo "${Flux_NC_H_JID_MSG}" | sed "s|^Your job-array \([0-9]\+\)\..*|\1|g")
    echo "[INFO]: Flux_NC H jobs farmed with JID: ${Flux_NC_H_JID}"

    HOLD_JID="${HOLD_JID} ${Flux_NC_H_JID}"

    cd ../
  fi # end N_NC_JOBS

fi # end N_H_IN_COMPOSITE

################################################################################



##                    Generate Wrong sign backgrounds



################################################################################


if [[ "${N_CC_JOBS_WSB}" != "0" ]]; then
  ################################################################################
  ##             Generate: WSB Main target CC
  ################################################################################
  mkdir CC_Flux_WSB; cd CC_Flux_WSB

  cp ${WSB_FLUX_FILE} wsb_flux.txt
  cp ${INPUT_JOBCARD} jobcard.in

  ################################################################################
  ##                         Build Replacements: WSB Main target CC
  ################################################################################
  echo -e "__NU_FLAVOR_CODE__ 2\n\
  __NU_INTERACTION_TYPE__ ${WSB_FLUX_CC_NU_SPEC}\n\
  __FLUX_FILE__ ./wsb_flux.txt\n\
  __TARGET_A__ ${TARGET_A}\n\
  __TARGET_Z__ ${TARGET_Z}\n\
  __FIX_BE__ ${FIX_BE}\n\
  __N_ENSEMBLE__ ${N_ENSEMBLES}\n\
  __N_TSTEPS__ 150\n\
  __N_RUNS__ ${N_RUNS}\n\
  __HIGH_FLUX_CUT__ 50\n\
  __DENSITY_SWITCH__ ${DENSITY_SWITCH}\n\
  __OSET_DELTA_BROAD__ ${USE_OSET_INMED_BROAD}\n\
  __BUU_INPUT__ ./BUUInput" > job.rpl

  ################################################################################
  ##                         Build jobcard: WSB Main target CC
  ################################################################################

  TMPFILEA=jobcard.in_tmp1
  TMPFILEB=jobcard.in_tmp2
  cp jobcard.in ${TMPFILEA}

  while read ln; do
    FIRST=$(echo ${ln} | cut -d " " -f 1)
    SECOND=$(echo ${ln} | cut -d " " -f 2)
    echo -e "\tReplacing ${FIRST} with ${SECOND}"
    cat ${TMPFILEA} | sed "s:${FIRST}:${SECOND}:" > ${TMPFILEB}
    mv ${TMPFILEB} ${TMPFILEA}
  done < job.rpl

  mv ${TMPFILEA} job.card

  ################################################################################
  ##                         Farm jobs: WSB Main target CC
  ################################################################################

  Flux_WSB_CC_JID_MSG=$(qsub -N Gen_WSBF_CC -v GIBUUTOOLSROOT=${GIBUUTOOLSROOT} -t 1-${N_CC_JOBS_WSB} ${GIBUUTOOLSROOT}/batchjobs/RunGiBUUBatch.sh)
  Flux_WSB_CC_JID=$(echo "${Flux_WSB_CC_JID_MSG}" | sed "s|^Your job-array \([0-9]\+\)\..*|\1|g")
  echo "[INFO]: Flux_WSB_CC jobs farmed with JID: ${Flux_WSB_CC_JID}"

  HOLD_JID="${HOLD_JID} ${Flux_WSB_CC_JID}"

  cd ../


  if [[ "${N_NC_JOBS_WSB}" != "0" ]]; then
  ################################################################################
  ##             Generate: WSB Main target NC
  ################################################################################
    mkdir NC_Flux_WSB; cd NC_Flux_WSB

    cp ${WSB_FLUX_FILE} wsb_flux.txt
    cp ${INPUT_JOBCARD} jobcard.in

  ################################################################################
  ##                         Build Replacements: WSB Main target NC
  ################################################################################
    echo -e "__NU_FLAVOR_CODE__ 2\n\
    __NU_INTERACTION_TYPE__ ${WSB_FLUX_NC_NU_SPEC}\n\
    __FLUX_FILE__ ./wsb_flux.txt\n\
    __TARGET_A__ ${TARGET_A}\n\
    __TARGET_Z__ ${TARGET_Z}\n\
    __FIX_BE__ ${FIX_BE}\n\
    __N_ENSEMBLE__ ${N_ENSEMBLES}\n\
    __N_TSTEPS__ 150\n\
    __N_RUNS__ ${N_RUNS}\n\
    __HIGH_FLUX_CUT__ 50\n\
    __DENSITY_SWITCH__ ${DENSITY_SWITCH}\n\
    __OSET_DELTA_BROAD__ ${USE_OSET_INMED_BROAD}\n\
    __BUU_INPUT__ ./BUUInput" > job.rpl

  ################################################################################
  ##                         Build jobcard: WSB Main target NC
  ################################################################################

    TMPFILEA=jobcard.in_tmp1
    TMPFILEB=jobcard.in_tmp2
    cp jobcard.in ${TMPFILEA}

    while read ln; do
      FIRST=$(echo ${ln} | cut -d " " -f 1)
      SECOND=$(echo ${ln} | cut -d " " -f 2)
      echo -e "\tReplacing ${FIRST} with ${SECOND}"
      cat ${TMPFILEA} | sed "s:${FIRST}:${SECOND}:" > ${TMPFILEB}
      mv ${TMPFILEB} ${TMPFILEA}
    done < job.rpl

    mv ${TMPFILEA} job.card

  ################################################################################
  ##                         Farm jobs: WSB Main target NC
  ################################################################################

    Flux_WSB_NC_JID_MSG=$(qsub -N Gen_WSBF_NC -v GIBUUTOOLSROOT=${GIBUUTOOLSROOT} -t 1-${N_NC_JOBS_WSB} ${GIBUUTOOLSROOT}/batchjobs/RunGiBUUBatch.sh)
    Flux_WSB_NC_JID=$(echo "${Flux_WSB_NC_JID_MSG}" | sed "s|^Your job-array \([0-9]\+\)\..*|\1|g")
    echo "[INFO]: Flux_WSB_NC jobs farmed with JID: ${Flux_WSB_NC_JID}"

    HOLD_JID="${HOLD_JID} ${Flux_WSB_NC_JID}"

    cd ../
  fi # end N_NC_JOBS

  if [[ "${N_H_IN_COMPOSITE}" != "0" ]]; then

    ################################################################################
    ##             Generate: WSB H target CC
    ################################################################################
    mkdir CC_Flux_WSB_H; cd CC_Flux_WSB_H

    cp ${WSB_FLUX_FILE} wsb_flux.txt
    cp ${INPUT_JOBCARD} jobcard.in

    ################################################################################
    ##                         Build Replacements: WSB H target CC
    ################################################################################
    echo -e "__NU_FLAVOR_CODE__ 2\n\
    __NU_INTERACTION_TYPE__ ${WSB_FLUX_CC_NU_SPEC}\n\
    __FLUX_FILE__ ./wsb_flux.txt\n\
    __TARGET_A__ 1\n\
    __TARGET_Z__ 1\n\
    __FIX_BE__ .false.\n\
    __N_ENSEMBLE__ ${N_H_ENSEMBLES}\n\
    __N_TSTEPS__ 0\n\
    __N_RUNS__ ${N_RUNS}\n\
    __HIGH_FLUX_CUT__ 50\n\
    __DENSITY_SWITCH__ ${DENSITY_SWITCH}\n\
    __OSET_DELTA_BROAD__ .false.\n\
    __BUU_INPUT__ ./BUUInput" > job.rpl

    ################################################################################
    ##                         Build jobcard: WSB H target CC
    ################################################################################

    TMPFILEA=jobcard.in_tmp1
    TMPFILEB=jobcard.in_tmp2
    cp jobcard.in ${TMPFILEA}

    while read ln; do
      FIRST=$(echo ${ln} | cut -d " " -f 1)
      SECOND=$(echo ${ln} | cut -d " " -f 2)
      echo -e "\tReplacing ${FIRST} with ${SECOND}"
      cat ${TMPFILEA} | sed "s:${FIRST}:${SECOND}:" > ${TMPFILEB}
      mv ${TMPFILEB} ${TMPFILEA}
    done < job.rpl

    mv ${TMPFILEA} job.card

    ################################################################################
    ##                         Farm jobs: WSB H target CC
    ################################################################################

    Flux_WSB_CC_H_JID_MSG=$(qsub -N Gen_WSBF_CC_H -v GIBUUTOOLSROOT=${GIBUUTOOLSROOT} -t 1-${N_CC_JOBS_WSB} ${GIBUUTOOLSROOT}/batchjobs/RunGiBUUBatch.sh)
    Flux_WSB_CC_H_JID=$(echo "${Flux_WSB_CC_H_JID_MSG}" | sed "s|^Your job-array \([0-9]\+\)\..*|\1|g")
    echo "[INFO]: Flux_WSB_CC H jobs farmed with JID: ${Flux_WSB_CC_H_JID}"

    HOLD_JID="${HOLD_JID} ${Flux_WSB_CC_H_JID}"

    cd ../


    if [[ "${N_NC_JOBS_WSB}" != "0" ]]; then
    ################################################################################
    ##             Generate: WSB H target NC
    ################################################################################
      mkdir NC_Flux_WSB_H; cd NC_Flux_WSB_H

      cp ${WSB_FLUX_FILE} wsb_flux.txt
      cp ${INPUT_JOBCARD} jobcard.in

    ################################################################################
    ##                         Build Replacements: WSB H target NC
    ################################################################################
      echo -e "__NU_FLAVOR_CODE__ 2\n\
      __NU_INTERACTION_TYPE__ ${WSB_FLUX_NC_NU_SPEC}\n\
      __FLUX_FILE__ ./wsb_flux.txt\n\
      __TARGET_A__ 1\n\
      __TARGET_Z__ 1\n\
      __FIX_BE__ .false.\n\
      __N_ENSEMBLE__ ${N_H_ENSEMBLES}\n\
      __N_TSTEPS__ 0\n\
      __N_RUNS__ ${N_RUNS}\n\
      __HIGH_FLUX_CUT__ 50\n\
      __DENSITY_SWITCH__ ${DENSITY_SWITCH}\n\
      __OSET_DELTA_BROAD__ .false.\n\
      __BUU_INPUT__ ./BUUInput" > job.rpl

    ################################################################################
    ##                         Build jobcard: WSB H target NC
    ################################################################################

      TMPFILEA=jobcard.in_tmp1
      TMPFILEB=jobcard.in_tmp2
      cp jobcard.in ${TMPFILEA}

      while read ln; do
        FIRST=$(echo ${ln} | cut -d " " -f 1)
        SECOND=$(echo ${ln} | cut -d " " -f 2)
        echo -e "\tReplacing ${FIRST} with ${SECOND}"
        cat ${TMPFILEA} | sed "s:${FIRST}:${SECOND}:" > ${TMPFILEB}
        mv ${TMPFILEB} ${TMPFILEA}
      done < job.rpl

      mv ${TMPFILEA} job.card

    ################################################################################
    ##                         Farm jobs: WSB H target NC
    ################################################################################

      Flux_WSB_NC_H_JID_MSG=$(qsub -N Gen_WSBF_NC_H -v GIBUUTOOLSROOT=${GIBUUTOOLSROOT} -t 1-${N_NC_JOBS_WSB} ${GIBUUTOOLSROOT}/batchjobs/RunGiBUUBatch.sh)
      Flux_WSB_NC_H_JID=$(echo "${Flux_WSB_NC_H_JID_MSG}" | sed "s|^Your job-array \([0-9]\+\)\..*|\1|g")
      echo "[INFO]: Flux_WSB_NC H jobs farmed with JID: ${Flux_WSB_NC_H_JID}"

      HOLD_JID="${HOLD_JID} ${Flux_WSB_NC_H_JID}"

      cd ../
    fi # end N_NC_JOBS

  fi # end N_H_IN_COMPOSITE

fi # end N_CC_JOBS_WSB
################################################################################
################################################################################



################################################################################
##                 Collect up the events
################################################################################

mkdir stdhep; cd stdhep

HOLD_JID=$(echo ${HOLD_JID} | tr " " ",")

echo "[INFO]: Holding on ${HOLD_JID}"

TOTAL_RESCALE=$(python -c "print (${TARGET_A} + ${N_H_IN_COMPOSITE});")

echo "-u ${NU_SPECIES} -a ${TARGET_A} -z ${TARGET_Z} -W ${TARGET_A} -f ../CC_Flux/FinalEvents*.dat -F ${FLUX_NAME},${FLUX_FILE}" > stdhep.conv.opts

if [[ "${Flux_NC_JID}" ]]; then
  echo "-N -W ${TARGET_A} -f ../NC_Flux/FinalEvents*.dat" >> stdhep.conv.opts
fi
if [[ "${Flux_CC_H_JID}" ]]; then
  echo "-a 1 -z 1 -W ${N_H_IN_COMPOSITE} -f ../CC_Flux_H/FinalEvents*.dat" >> stdhep.conv.opts
fi
if [[ "${Flux_NC_H_JID}" ]]; then
  echo "-N -a 1 -z 1 -W ${N_H_IN_COMPOSITE} -f ../NC_Flux_H/FinalEvents*.dat" >> stdhep.conv.opts
fi

################################################################################
# WSB
if [[ "${Flux_WSB_CC_JID}" ]]; then
  echo "-u ${WSB_NU_SPECIES} -a ${TARGET_A} -z ${TARGET_Z} -W ${TARGET_A} -f ../CC_Flux_WSB/FinalEvents*.dat -F ${WSB_FLUX_NAME},${WSB_FLUX_FILE}" >> stdhep.conv.opts
fi
if [[ "${Flux_WSB_NC_JID}" ]]; then
  echo "-N -W ${TARGET_A} -f ../NC_Flux_WSB/FinalEvents*.dat" >> stdhep.conv.opts
fi
if [[ "${Flux_WSB_CC_H_JID}" ]]; then
  echo "-a 1 -z 1 -W ${N_H_IN_COMPOSITE} -f ../CC_Flux_WSB_H/FinalEvents*.dat" >> stdhep.conv.opts
fi
if [[ "${Flux_WSB_NC_H_JID}" ]]; then
  echo "-N -a 1 -z 1 -W ${N_H_IN_COMPOSITE} -f ../NC_Flux_WSB_H/FinalEvents*.dat" >> stdhep.conv.opts
fi
################################################################################


echo "-R i${TOTAL_RESCALE}" >> stdhep.conv.opts
echo "-o ${JOB_NAME}_GiBUU.stdhep.root" >> stdhep.conv.opts

STDHEPPROC_JID_MSG=$(qsub -hold_jid ${HOLD_JID} -v GIBUUTOOLSROOT=${GIBUUTOOLSROOT} ${GIBUUTOOLSROOT}/batchjobs/ProcessToStdHep.sh)
STDHEPPROC_JID=$(echo "${STDHEPPROC_JID_MSG}" | sed "s|^Your job \([0-9]\+\) .*|\1|g")
cd ../

echo -e "#!/bin/sh\necho \"Killing jobs: ${HOLD_JID}\";qdel ${HOLD_JID},${STDHEPPROC_JID}" > killjobs.sh
chmod +x killjobs.sh
