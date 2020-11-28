#!/bin/sh

SCRIPTDIR="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

echo "\n\n===== Installing Required Packages =====\n\n"
sudo apt-get update
sudo apt install python3 python3-pip zip unzip snapd git cmake build-essential libfontconfig1 mesa-common-dev libglu1-mesa-dev qt5-default qttools5-dev-tools qttools5-dev qtbase5-dev libqt5opengl5-dev libqt5svg5-dev libxml2-dev libode-dev libjpeg-dev libpng-dev libglew-dev libfftw3-dev libeigen3-dev libboost-all-dev portaudio19-dev python3-pip -y

echo "\n\n===== Installing SANPs =====\n\n"
sudo snap install gitkraken --classic
sudo snap install code --classic

echo "\n\n===== Installing Python Requirements =====\n\n"
python3 -m pip install -r $SCRIPTDIR/../pythonRequirements.txt

echo && echo "====== Installing Requirements ======" && echo
python3 -m pip install gdown
export PATH=$PATH:~/.local/bin

echo && echo '====== Downloading Toolchain ======' && echo
gdown https://drive.google.com/uc?id=1TSgr5hTnxapW797hapgRUgBRt794Yart
unzip toolchain.zip -d $SCRIPTDIR/
rm $SCRIPTDIR/toolchain/Toolchains.adoc
mv $SCRIPTDIR/toolchain/* $SCRIPTDIR/../toolchain/ 
rm -R $SCRIPTDIR/toolchain
rm toolchain.zip

echo && echo "====== Removing requirement that is no longer needed ======" && echo
python3 -m pip uninstall -y gdown

echo "\n\n===== Building Simrobot =====\n\n"
cd $SCRIPTDIR/../tools/SimRobot/
./build_simrobot

cd $SCRIPTDIR/..
./scripts/midas setup simrobot
./scripts/midas compile develop simrobot
