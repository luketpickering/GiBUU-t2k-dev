#!/bin/sh

if [[ ! "${GIBUUTOOLSROOT}" ]]; then
  echo "[ERROR]: Expected \${GIBUUTOOLSROOT} to be defined, have your sourced the GiBUUTools setup script?"
  exit 1
fi

echo "[INFO]: This is about to farm off a very large number of SGE jobs, Ctrl+C if you don't want that to happen."
sleep 10
echo "[INFO]: Here I go..."

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_NP_numub.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_NP_numu.txt \
  -A -n 10 -a 12 -z 6 -N -h 2 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in \
  -J MiniBooNE_NP_CH2_BNLPiProd

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_NP_numub.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_NP_numu.txt \
  -A -n 10 -a 12 -z 6 -N -h 2 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_ANLPiProd.job.in \
  -J MiniBooNE_NP_CH2_ANLPiProd

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_PP_numu.txt \
  -n 10 -a 12 -z 6 -N -h 2 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in \
  -J MiniBooNE_PP_CH2_BNLPiProd

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MiniBooNE_PP_numu.txt \
  -n 10 -a 12 -z 6 -N -h 2 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_ANLPiProd.job.in \
  -J MiniBooNE_PP_CH2_ANLPiProd


${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_rhc_numubar_2015.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_rhc_numu_2015.txt \
  -A -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in \
  -J MINERvA_RHC_CH_BNLPiProd

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_rhc_numubar_2015.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_rhc_numu_2015.txt \
  -A -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_ANLPiProd.job.in \
  -J MINERvA_RHC_CH_ANLPiProd

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_fhc_numu_2015.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_fhc_numubar_2015.txt \
  -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in \
  -J MINERvA_FHC_CH_BNLPiProd

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_fhc_numu_2015.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/MINERvA_fhc_numubar_2015.txt \
  -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_ANLPiProd.job.in \
  -J MINERvA_FHC_CH_ANLPiProd


${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/T2K_ND5_FHC_numu.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/T2K_ND5_FHC_numub.txt \
  -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_BNLPiProd.job.in \
  -J T2K_ND5_FHC_CH_BNLPiProd

${GIBUUTOOLSROOT}/batchjobs/Generate_Batch.sh \
  -F ${GIBUUTOOLSROOT}/batchjobs/fluxes/T2K_ND5_FHC_numu.txt \
  -w ${GIBUUTOOLSROOT}/batchjobs/fluxes/T2K_ND5_FHC_numub.txt \
  -n 10 -a 12 -z 6 -N -h 1 \
  -i ${GIBUUTOOLSROOT}/batchjobs/GiBUU_ANLPiProd.job.in \
  -J T2K_ND5_FHC_CH_ANLPiProd
