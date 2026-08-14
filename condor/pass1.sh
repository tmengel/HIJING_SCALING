#!/bin/bash

INSTALLDIR="/sphenix/user/tmengel/hijing_scaling/HIJING_SCALING/install"
source /opt/sphenix/core/bin/sphenix_setup.sh -n new
source $OPT_SPHENIX/bin/setup_local.sh $INSTALLDIR

SEG="$1"

TODAY="20260813"
OUTDIR="/sphenix/tg/tg01/jets/tmengel/HIJING_SCALING/pass1/${TODAY}"
mkdir -p "$OUTDIR"

MACRODIR="/sphenix/user/tmengel/hijing_scaling/HIJING_SCALING/macros"
cd "$MACRODIR" || exit 1

NEVENTS=-1
RUN=31
EMBFILE="${OUTDIR}/hijing_run${RUN}_pass1-$(printf "%05d" $SEG).root"
root -l -b -q "Fun4All_UEScaling_Pass1.C(${NEVENTS}, ${RUN}, ${SEG}, \"${EMBFILE}\")"
EXITCODE=$?
if [ $EXITCODE -ne 0 ]; then
    echo "ERROR: pass1 failed with exit code $EXITCODE"
    exit $EXITCODE
fi

for JETID in 10 20 30 ; do
    OUTDIR_JET="/sphenix/tg/tg01/jets/tmengel/HIJING_SCALING/pass2/jet${JETID}/${TODAY}"
    mkdir -p "$OUTDIR_JET"
    OUTFILE="${OUTDIR_JET}/DST_EMB_hijing31_jet${JETID}_pass2-$(printf "%05d" $SEG).root"
   
    cd "$MACRODIR" || exit 1
    root -l -b -q "Fun4All_UEScaling_Pass2.C(${NEVENTS}, ${RUN}, ${SEG}, ${JETID}, \"${EMBFILE}\", \"${OUTFILE}\")"
    EXITCODE=$?
    if [ $EXITCODE -ne 0 ]; then
        echo "ERROR: pass2 for jet ${JETID} failed with exit code $EXITCODE"
    fi
done
