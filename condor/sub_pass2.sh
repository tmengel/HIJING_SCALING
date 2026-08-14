#!/bin/bash

JETIDS=(12 20 30)
for JETID in "${JETIDS[@]}"; do
    mkdir -p "/sphenix/user/tmengel/JetUESub-JSTG-TF03/condor/logs/jet${JETID}"
    condor_submit overlay_pass2.job \
        -a "jetid=${JETID}" 
done