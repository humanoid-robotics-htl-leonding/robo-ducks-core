#!/bin/sh

sudo apt-get update
sudo apt install snapd -y
sudo apt install git -y
sudo apt install cmake -y
sudo apt install build-essential -y
sudo apt install libfontconfig1 -y
sudo apt install mesa-common-dev -y
sudo apt install libglu1-mesa-dev -y
sudo apt install qt5-default qttools5-dev-tools qttools5-dev -y
sudo apt install qtbase5-dev libqt5opengl5-dev libqt5svg5-dev -y
sudo apt install libxml2-dev -y
sudo apt install libode-dev -y
sudo apt install libjpeg-dev -y
sudo apt install libpng-dev -y
sudo apt install libglew-dev -y
sudo apt install libfftw3-dev -y
sudo apt install libeigen3-dev -y
sudo apt install libboost-all-dev -y

echo "Install SANPs\n\n"
sudo snap install gitkraken
sudo snap install code --classic
#sudo snap install clion --classic -y 

echo "Build Simrobot\n\n"
cd ..
work=$(pwd)
echo $work
$work/tools/SimRobot/build_simrobot -j 8

echo "Start SNAPs\n\n"
snap run gitkraken&
snap run code&
