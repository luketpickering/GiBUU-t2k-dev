#!/bin/sh
#$-cwd
#$-q hep.q
#$-l h_rt=10:0:0

## Source your environment here
source ${GIBUUTOOLSROOT}/setup.sh

which GiBUU.x

CARDFILEINP=$(readlink -f ./job.card)
if [[ -e "flux.txt" ]]; then
  FLUX=$(readlink -f ./flux.txt)
elif [[ -e "wsb_flux.txt" ]]; then
  FLUX=$(readlink -f ./wsb_flux.txt)
elif [[ -e "NUE_flux.txt" ]]; then
  FLUX=$(readlink -f ./NUE_flux.txt)
elif [[ -e "WSB_NUE_flux.txt" ]]; then
  FLUX=$(readlink -f ./WSB_NUE_flux.txt)
else
  echo "[ERROR]: Couldn't find flux.txt or wsb_flux.txt"
  exit 1
fi

OUTPUTDIR=$(readlink -f .)

touch job.${SGE_TASK_ID}.running

mkdir ${TMP}/GiBUU_${SGE_TASK_ID}

cp ${FLUX} ${TMP}/GiBUU_${SGE_TASK_ID}/

cd ${TMP}/GiBUU_${SGE_TASK_ID}

MakeBUUInputSoftlink

sleep ${SGE_TASK_ID}

SEED=${RANDOM}
echo "SEED: ${SEED}"

cat ${CARDFILEINP} | sed "s/__RANDOM_SEED__/${SEED}/" > job.${SGE_TASK_ID}.card

date

echo "Reading: ${CARDFILEINP}"

cp job.${SGE_TASK_ID}.card ${OUTPUTDIR}/

GiBUU.x < job.${SGE_TASK_ID}.card &>  g.${SGE_TASK_ID}.run
if [[ "$?" != "0" ]]; then
  rm ${OUTPUTDIR}/job.${SGE_TASK_ID}.running
  touch ${OUTPUTDIR}/job.${SGE_TASK_ID}.failed
  mv g.${SGE_TASK_ID}.run ${OUTPUTDIR}/
  exit 1
fi

date

mv FinalEvents.dat ${OUTPUTDIR}/FinalEvents_${SGE_TASK_ID}.dat

cd ${OUTPUTDIR}

mv job.${SGE_TASK_ID}.running job.${SGE_TASK_ID}.done
