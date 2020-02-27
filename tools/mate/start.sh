#!/usr/bin/env bash

echoerr() { echo "$@" 1>&2; }

BASEDIR=$(dirname $(readlink -f $0))
cd ${BASEDIR}

CONDA_EXECUTABLE=$(which conda)

print_help() {
    echo "== Mate =="
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --hep                Displays this help"
    echo "  -c, --conda [PATH]       Manually specify path to the conda executable"
}

while [[ $# -gt 0 ]]; do
    case $1 in
        "-c" | "--conda")
            shift
            CONDA_EXECUTABLE=$1
        ;;
        "-h" | "--help")
            print_help
            exit 0
        ;;
    esac
    shift
done

if [[ ! -e ${CONDA_EXECUTABLE} ]] || [[ -z ${CONDA_EXECUTABLE} ]]; then
    echoerr "Path to conda not found. Please specify manually with '-c [PATH_OF_CONDA_EXECUTABLE]'. Path without the executable itself"

fi

CONDA_DIR=$(dirname ${CONDA_EXECUTABLE})

echo "Using conda in ${CONDA_DIR}"

CONDA_LIB=$(readlink -f ${CONDA_DIR}/../etc/profile.d/conda.sh)

echo "Sourcing Conda from $CONDA_LIB"
source ${CONDA_LIB}

conda activate MateEnv
echo "Starting Mate with python at: '$(which python)'"
echo "======="

MATE_EXIT=1
while [[ ${MATE_EXIT} -ne 0 ]]; do
    python ./run.py
    MATE_EXIT=$?
    echo "Mathe finished with exit code ${MATE_EXIT}"
done

exit ${MATE_EXIT}