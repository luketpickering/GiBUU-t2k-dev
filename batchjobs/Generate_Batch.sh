#!/bin/sh

FLUX_FILE=""
WSB_FLUX_FILE=""
INPUT_JOBCARD="${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in"
N_CC_JOBS="1"
TARGET_A="12"
TARGET_Z="6"
USE_NC="0"
N_H_IN_COMPOSITE="0"
JOB_NAME="gibuu_gen"

while [[ ${#} -gt 0 ]]; do

  key="$1"
  case $key in

      -F|--flux-file)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      FLUX_FILE="$2"
      shift # past argument
      ;;

      -W|--wrong-sign-flux-file)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      WSB_FLUX_FILE="$2"
      shift # past argument
      ;;

      -n|--num-cc-jobs)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      N_CC_JOBS="$2"
      shift # past argument
      ;;

      -a|--target-a)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      TARGET_A="$2"
      shift # past argument
      ;;

      -z|--target-z)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      TARGET_Z="$2"
      shift # past argument
      ;;

      -N|--do-NC)
      USE_NC="1"
      ;;

      -h|--num-h-in-target)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      N_H_IN_COMPOSITE="$2"
      shift
      ;;

      -i|--input-job-card)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      INPUT_JOBCARD="$2"
      shift
      ;;

      -J|--job-name)

      if [[ ${#} -lt 2 ]]; then
        echo "[ERROR]: ${1} expected a value."
        exit 1
      fi

      JOB_NAME="$2"
      shift
      ;;

      -?|--help)

      echo "[RUNLIKE] "

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

if [[ ! -e ${GIBUUTOOLSROOT}/setup.sh ]]; then
  echo "[ERROR]: Expected to find a GiBUU environment set up script at \"\$GIBUUTOOLSROOT/setup.sh\". Is this sourced?"
  exit 1
fi

hash GiBUU.x &> /dev/null
if [[ "${?}" != "0" ]]; then
  echo "[ERROR]: Expected to find the GiBUU executable, GiBUU.x, but it wasn't available."
  exit 1
fi

if [[ "${WSB_FLUX_FILE}" ]] && [[ ! -e "${WSB_FLUX_FILE}" ]]; then
  echo "[ERROR]: Wrong sign flux file: \"${WSB_FLUX_FILE}\" could not be found."
  exit 1
fi

if [[ ! -e "${INPUT_JOBCARD}" ]]; then
  echo "[ERROR]: Specified input job card \"${INPUT_JOBCARD}\" does not exist."
  exit 1
fi
INPUT_JOBCARD=$(readlink -f ${INPUT_JOBCARD})

################################################################################
################################################################################
mkdir ${JOB_NAME}
cd ${JOB_NAME}


N_NC_JOBS="0"

if [[ "${USE_NC}" != "0" ]]; then
  N_NC_JOBS=$(python -c "from math import ceil; print int(ceil(float(${N_CC_JOBS})/float(5)));")
  echo "[INFO]: Farming ${N_NC_JOBS} NC Jobs."
fi

################################################################################
##             Generate: Main target numu CC
################################################################################
mkdir CC_numu; cd CC_numu

cp ${FLUX_FILE} flux.txt
cp ${INPUT_JOBCARD} jobcard.in

################################################################################
##                         Build Replacements: Main target numu CC
################################################################################
echo -e "__NU_FLAVOR_CODE__ 2\n\
__NU_INTERACTION_TYPE__ 2\n\
__FLUX_FILE__ ./flux.txt\n\
__TARGET_A__ ${TARGET_A}\n\
__TARGET_Z__ ${TARGET_Z}\n\
__FIX_BE__ .true.\n\
__N_ENSEMBLE__ 4000\n\
__N_TSTEPS__ 150\n\
__N_RUNS__ 20\n\
__HIGH_FLUX_CUT__ 50\n\
__OSET_DELTA_BROAD__ .true.\n\
__BUU_INPUT__ ./BUUInput" > job.rpl

################################################################################
##                         Build jobcard: Main target numu CC
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
##                         Farm jobs: Main target numu CC
################################################################################

NUMU_CC_JID_MSG=$(qsub -v GIBUUTOOLSROOT=${GIBUUTOOLSROOT} -t 1-${N_CC_JOBS} ${GIBUUTOOLSROOT}/batchjobs/RunGiBUUBatch.sh)
NUMU_CC_JID=$(echo "${NUMU_CC_JID_MSG}" | sed "s|^Your job-array \([0-9]\+\)\..*|\1|g")
echo "[INFO]: NUMU_CC jobs farmed with JID: ${NUMU_CC_JID}"

cd ../

################################################################################
################################################################################

################################################################################
##                 Collect up the events
################################################################################

mkdir stdhep; cd stdhep

echo "-u 14 -a ${TARGET_A} -z ${TARGET_Z} -f CC_numu/FinalEvents*.dat -F numu_flux,${FLUX_FILE}" > stdhep.conv.opts

qsub -hold_jid ${NUMU_CC_JID} -v GIBUUTOOLSROOT=${GIBUUTOOLSROOT} ${GIBUUTOOLSROOT}/batchjobs/ProcessToStdHep.sh

cd ../

