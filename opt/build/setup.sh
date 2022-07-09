INSTALL_DIR=${MINERVA_PREFIX:-"/minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt"}
OLD_PATH=${PATH}
OLD_LIBS=${LD_LIBRARY_PATH}

source ${INSTALL_DIR}/bin/setup_MAT.sh
source ${INSTALL_DIR}/bin/setup_MAT-MINERvA.sh
source ${INSTALL_DIR}/bin/setup_UnfoldUtils.sh
#source ${INSTALL_DIR}/bin/setup_GENIEXSecExtract.sh

#Don't repeat a lot of copies of INSTALL_DIR on PATH and LD_LIBRARY_PATH.
#WARNING: This is very specific to the tutorial.  Don't copy this blindly!
export PATH=${OLD_PATH}:${INSTALL_DIR}/bin
export LD_LIBRARY_PATH=${OLD_LIBS}:${INSTALL_DIR}/lib


#Set up MParamFiles for my laptop
export MPARAMFILESROOT=${INSTALL_DIR}/etc/MParamFiles
export MATFLUXANDWEIGHTFILES=${INSTALL_DIR}/etc/MATFluxAndReweightFiles
export MPARAMFILES=${MPARAMFILESROOT}/data
