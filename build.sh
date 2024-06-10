#!/bin/bash

#!/bin/bash

make config

if [ $? != 0 ]; then
    echo "[ERROR] config fail."
    exit 1
fi

make buildInc buildIncRoot="netstack" 
# make buildSrc valName="SRC" buildSrcRoot="src/"

make