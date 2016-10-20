#!/bin/sh
#$-cwd
#$-q hep.q
#$-l h_rt=2:0:0

## Source your environment here
source ${GIBUUTOOLSROOT}/setup.sh

touch stdhep.running
if ! GiBUUToStdHep -@ stdhep.conv.opts -o GiBUU.${JOB_NAME}.stdhep.root; then
  echo "[ERROR]: Failed to process to StdHep format."
  mv stdhep.running stdhep.Failed
  exit 1
fi

echo "[INFO]: Processed events to StdHep format!"
mv stdhep.running stdhep.done
