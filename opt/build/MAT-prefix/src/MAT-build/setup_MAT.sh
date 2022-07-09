#Make this package relocatable by making PREFIX overrideable.  Default to the old behavior of setting up from a fixed location.
#You might use this feature to run on the FermiGrid for example.
PREFIX=${MINERVA_PREFIX:-"/minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt"}

#Install directory is filled in by CMake at project configure time.
export PLOTUTILSROOT=${PREFIX}/lib
export LD_LIBRARY_PATH=${PLOTUTILSROOT}:$LD_LIBRARY_PATH
export PATH=${PREFIX}/bin:$PATH

export PLOTUTILSTYPE="STANDALONE"
export PLOTUTILSVERSION="ROOT6"

export ROOT_INCLUDE_PATH=${PREFIX}/include/PlotUtils:${PREFIX}/include:${ROOT_INCLUDE_PATH}
