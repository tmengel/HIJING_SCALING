#!/bin/bash

INSTALLDIR="/sphenix/user/tmengel/hijing_scaling/HIJING_SCALING/install"
source /opt/sphenix/core/bin/sphenix_setup.sh -n new
source $OPT_SPHENIX/bin/setup_local.sh $INSTALLDIR

SEG="$1"
JETID="$2"

TODAY="$(date +%Y%m%d)"
OUTDIR="/sphenix/tg/tg01/jets/tmengel/HIJING_SCALING/pass2/jet${JETID}/${TODAY}"
mkdir -p "$OUTDIR"


MACRODIR="/sphenix/user/tmengel/hijing_scaling/HIJING_SCALING/macros"
cd "$MACRODIR" || exit 1

NEVENTS=-1
RUN=31
OUTFILE="${OUTDIR}/DST_EMB_hijing31_pythia28_jet${JETID}_pass2-$(printf "%05d" $SEG).root"
EMBFILE="/sphenix/tg/tg01/jets/tmengel/HIJING_SCALING/pass1/${TODAY}/hijing_run${RUN}_pass1-$(printf "%05d" $SEG).root"

root -l -b -q "Fun4All_UEScaling_Pass2.C(${NEVENTS}, ${RUN}, ${SEG}, ${JETID}, \"${EMBFILE}\", \"${OUTFILE}\")"
EXITCODE=$?
if [ $EXITCODE -ne 0 ]; then
    echo "Error: pass2 failed with exit code $EXITCODE"
    exit $EXITCODE
else
    echo "pass2 completed successfully"
    exit 0
fi

