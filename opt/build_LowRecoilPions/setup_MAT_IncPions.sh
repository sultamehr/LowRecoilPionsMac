#!/bin/bash

#Make this package relocatable by making PREFIX overrideable.  Default to the old behavior of setting up from a fixed location.
#You might use this feature to run on the FermiGrid for example.
PREFIX=${MINERVA_PREFIX:-"/minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt"}

#Make sure libraries can be found at runtime and executable names can
#be autocompleted by bash.
export LD_LIBRARY_PATH=${PREFIX}/lib:$LD_LIBRARY_PATH
export PATH=${PREFIX}/bin:$PATH
export DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${LD_LIBRARY_PATH}

#Bash autocompletion support for playlist files.  Auto-complete will kick in for
#any executable this package produces and suggest playlist names from the install
#directory as well as the current directory.
if command -v complete &> /dev/null #Don't set up autocompletion on zsh which is the new default shell on macos
then
  complete -o filenames -W '$(find ${PREFIX}/etc/playlists -type f)' -o default -o bashdefault $(find ${PREFIX}/bin -type f -printf %f\\n)
fi
