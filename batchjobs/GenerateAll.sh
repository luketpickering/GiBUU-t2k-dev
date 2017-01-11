#!/bin/sh

if [[ ! "${GIBUUTOOLSROOT}" ]]; then
  echo "[ERROR]: Expected \${GIBUUTOOLSROOT} to be defined, have your sourced the GiBUUTools setup script?"
  exit 1
fi

DOCB=""
NAMESUFFIX=""
if [[ "${1}" ]]; then

  if [[ "${1}" == "--help" ]] || [[ "${1}" == "-?" ]]; then
    echo "[RUNLIKE]: ${0} [--no-collisional-broadening|-B] "
    echo -e "[RUNLIKE]:\t-B: Generate event with collisional broadening off."
    exit 0
  elif [[ "${1}" == "--no-collisional-broadening" ]] || [[ "${1}" == "-B" ]]; then
    DOCB="-B"
    NAMESUFFIX="_NoCB"
  else
    echo "[ERROR]: Unexpected argument \"${1}\""
    echo "[RUNLIKE]: ${0} [--no-collisional-broadening|-B] "
    echo -e "[RUNLIKE]:\t-B: Generate event with collisional broadening off (increased pion production cross section)."
    exit 1
  fi

fi

echo "[INFO]: This is about to farm off a very large number of SGE jobs, Ctrl+C if you don't want that to happen."
sleep 5
echo "[INFO]: Here I go..."
sleep 1

# MiniBooNE
## NP (RHC)
${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_NP_numub.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_NP_numu.txt \
  -A -n 10 -a 12 -z 6 -N -h 2 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in \
  -J MiniBooNE_NP_CH2_BNLPiProd${NAMESUFFIX} ${DOCB}

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_NP_numub.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_NP_numu.txt \
  -A -n 10 -a 12 -z 6 -N -h 2 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_ANLPiProd.job.in \
  -J MiniBooNE_NP_CH2_ANLPiProd${NAMESUFFIX} ${DOCB}

## PP (FHC)
${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_PP_numu.txt \
  -n 10 -a 12 -z 6 -N -h 2 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in \
  -J MiniBooNE_PP_CH2_BNLPiProd${NAMESUFFIX} ${DOCB}

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_PP_numu.txt \
  -n 10 -a 12 -z 6 -N -h 2 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_ANLPiProd.job.in \
  -J MiniBooNE_PP_CH2_ANLPiProd${NAMESUFFIX} ${DOCB}

# MINERvA
## RHC
${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_numubar_rhc_2015.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_numu_rhc_2015.txt \
  -A -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in \
  -J MINERvA_RHC_CH_BNLPiProd${NAMESUFFIX} ${DOCB}

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_numubar_rhc_2015.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_numu_rhc_2015.txt \
  -A -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_ANLPiProd.job.in \
  -J MINERvA_RHC_CH_ANLPiProd${NAMESUFFIX} ${DOCB}

## FHC
${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_numu_fhc_2015.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_numubar_fhc_2015.txt \
  -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in \
  -J MINERvA_FHC_CH_BNLPiProd${NAMESUFFIX} ${DOCB}

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_numu_fhc_2015.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_numubar_fhc_2015.txt \
  -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_ANLPiProd.job.in \
  -J MINERvA_FHC_CH_ANLPiProd${NAMESUFFIX} ${DOCB}

# T2K
## FHC
${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/T2K_ND5_FHC_numu.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/T2K_ND5_FHC_numub.txt \
  -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in \
  -J T2K_ND5_FHC_CH_BNLPiProd${NAMESUFFIX} ${DOCB}

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/T2K_ND5_FHC_numu.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/T2K_ND5_FHC_numub.txt \
  -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_ANLPiProd.job.in \
  -J T2K_ND5_FHC_CH_ANLPiProd${NAMESUFFIX} ${DOCB}
