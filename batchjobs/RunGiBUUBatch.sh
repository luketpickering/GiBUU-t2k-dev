#!/bin/bash
#PBS -l walltime=10:00:00
#PBS -l mem=2048MB
#PBS -j oe

## Source your environment here
source ${GIBUUTOOLSROOT}/setup.sh

which GiBUU.x
ldd $(which GiBUU.x)
echo "PATH: $PATH"
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"

WD=$(readlink -f ./)
TDIR=${TMP}
TASK_ID=${SGE_TASK_ID}
ARR_IT=${SGE_TASK_ID}
if [ ! -z "${PBS_O_WORKDIR}" ]; then #Probably running @ MSU or similar
  TDIR=${TMPDIR}
  WD=${PBS_O_WORKDIR}
  TASK_ID="${PBS_JOBID}.${PBS_ARRAYID}"
  ARR_IT=${PBS_ARRAYID}
fi


CARDFILEINP=$(readlink -f ${WD}/job.card)
if [[ -e "${WD}/flux.txt" ]]; then
  FLUX=$(readlink -f ${WD}/flux.txt)
elif [[ -e "${WD}/wsb_flux.txt" ]]; then
  FLUX=$(readlink -f ${WD}/wsb_flux.txt)
elif [[ -e "${WD}/NUE_flux.txt" ]]; then
  FLUX=$(readlink -f ${WD}/NUE_flux.txt)
elif [[ -e "${WD}/WSB_NUE_flux.txt" ]]; then
  FLUX=$(readlink -f ${WD}/WSB_NUE_flux.txt)
else
  echo "[ERROR]: Couldn't find flux.txt or wsb_flux.txt"
  exit 1
fi

touch ${WD}/job.${TASK_ID}.running

mkdir ${TDIR}/GiBUU_${TASK_ID}

cp ${FLUX} ${TDIR}/GiBUU_${TASK_ID}/

cd ${TDIR}/GiBUU_${TASK_ID}

MakeBUUInputSoftlink

sleep ${ARR_IT}

SEED=${RANDOM}
echo "SEED: ${SEED}"

cat ${CARDFILEINP} | sed "s/__RANDOM_SEED__/${SEED}/" > job.${TASK_ID}.card

date

echo "Reading: ${CARDFILEINP}"

cp job.${TASK_ID}.card ${WD}/

GiBUU.x < job.${TASK_ID}.card &>  g.${TASK_ID}.run
if [[ "$?" != "0" ]]; then
  rm ${WD}/job.${TASK_ID}.running
  touch ${WD}/job.${TASK_ID}.failed
  mv g.${TASK_ID}.run ${WD}/
  exit 1
fi

date

mv FinalEvents.dat ${WD}/FinalEvents_${TASK_ID}.dat

cd ${WD}

mv job.${TASK_ID}.running job.${TASK_ID}.done
