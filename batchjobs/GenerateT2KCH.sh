#!/bin/sh

if [[ ! "${GIBUUTOOLSROOT}" ]]; then
  echo "[ERROR]: Expected \${GIBUUTOOLSROOT} to be defined, have your sourced the GiBUUTools setup script?"
  exit 1
fi


${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/T2K_ND5_FHC_numu.txt \
  -n 82 -a 12 -z 6 -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_2017DefaultCard.in \
  -J T2K_ND5_FHC_CH -B
