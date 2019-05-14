#!/usr/bin/env bash

BASEDIR=$(dirname $(readlink -f $0))

cd $BASEDIR

source $(dirname $(which conda))/../etc/profile.d/conda.sh
conda activate MateEnv
echo "Starting Mate with python at: '$(which python)'"
python ./run.py
