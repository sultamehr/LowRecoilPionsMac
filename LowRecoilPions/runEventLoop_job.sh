#!/bin/bash
printenv
set -x #start bash debugging at this point
echo Start  `date`
echo Site:${GLIDEIN_ResourceName}
echo "the worker node is " `hostname` "OS: "  `uname -a`
echo "the user id is " `whoami`
echo "the output of id is " `id`
set +x #stop bash debugging at this point

umask 002 #set the read/write permission of files created to be 775

cd $_CONDOR_SCRATCH_DIR

echo "pwd is " `pwd`

#Output location defined by use of $CONDOR_DIR_OUT defined by -d OUT /pnfs/minerva/scratch/users/dlast/

if [ -z ${GRID_USER} ]; then
GRID_USER=`basename $X509_USER_PROXY | cut -d "_" -f 2`
fi

echo "GRID_USER = `echo $GRID_USER`"

OUTTAG=`date +%m_%d_%Y_%H_%M`_${CLUSTER}.${PROCESS}

echo "Here is the your environment in this job: " > job_output_${OUTTAG}.log 2> job_output_${OUTTAG}.err #Creates file for logging information, note that this is stdout only
env >> job_output_${OUTTAG}.log 2>> job_output_${OUTTAG}.err #since file already exists use ">>" instead of ">"

/usr/bin/time -v -o tarTime.txt tar -xzvf $CONDOR_DIR_INPUT/dlast_*.tgz

if [ $? -ne 0 ]; then
        echo "Failed to unpack input tarball"
        echo "Are you sure this file exists?:"
        echo "/pnfs/minerva/resilient/tarballs/dlast_TEST_Ana_Tarball.tgz"
        exit 71
fi

export MINERVA_PREFIX=`pwd`/opt

source setup_env.sh

source setup_utils.sh

#Arguments Reminder
#1: playlist data
#2: playlist MC
#3: skip syst? anything non-zero means yes.
#4: MnvTune: 1 or 2 only options currently (10/27/2021)
#5: "Tracker" or "Targets" for FV definition
#6: Necessary TgtNum for single target selection
#7: optional naming of the files
#8: optional switch to turn off neutron cuts when passed a 0.

/usr/bin/time -v -o runEventLoopTime.txt runEventLoop ./Target-Antinu-Neutrons/playlists/$1 ./Target-Antinu-Neutrons/playlists/$2 $3 $4 $5 $6 $7 $8 > Dump_runEventLoop.txt

mv *.txt $CONDOR_DIR_OUT
mv *.log *.err $CONDOR_DIR_OUT
mv *.root $CONDOR_DIR_OUT
