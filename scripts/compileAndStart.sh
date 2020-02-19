#!/bin/bash

scriptDir=$(dirname $(readlink -f $0))
cd ${scriptDir}/../tools/SimRobot
echo "Executing in $(pwd)"

ls -la $(pwd)/Scenes/

sceneDir=""

if [[ $# -ge 1 ]]; then
    sceneDir=$(pwd)/Scenes/$1
fi

../../scripts/midas compile develop simrobot && build/SimRobot ${sceneDir}
