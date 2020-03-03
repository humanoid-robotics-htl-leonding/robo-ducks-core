#include <Modules/Log/Log.h>

#include <cmath>
#include "Tools/Chronometer.hpp"
#include "../print.hpp"
#include "LEDHandler.hpp"

using namespace keys::led;


std::array<float, EYE_MAX> LEDHandler::rainbowLeft_ = {
        {0.7f, 0.0f, 0.0f, 0.0f, 0.3f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.7f, 1.0f,
                1.0f, 1.0f, 0.3f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f}};

std::array<float, EYE_MAX> LEDHandler::rainbowRight_ = {
        {0.7f, 1.0f, 1.0f, 1.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 1.0f,
                1.0f, 1.0f, 0.7f, 0.0f, 1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f}};


LEDHandler::LEDHandler(const ModuleManagerInterface& manager)
  : Module(manager)
  , cycleInfo_(*this)
  , ledRequest_(*this)
  , cmd_(CHEST_MAX + 2 * EAR_MAX + 2 * EYE_MAX + HEAD_MAX + 2 * FOOT_MAX, 0.f)
  , cycleCount_(0)
  , rainbowCycle_(0)
  , lastLoadingRightEar(EAR_MAX,0.0f)
  , lastLoadingLeftEar(EAR_MAX,0.0f)
{
    isIncreasingHalfCycleRight = true;
    lastStartTimeRight = 0;
    currentSpeedRight = 0;
    loaderRightLength = 4;
    loadPosRightEnd = 0;
    lastLoadRightTime = 0;

    isIncreasingHalfCycleLeft = true;
    lastStartTimeLeft = 0;
    currentSpeedLeft = 0;
    loaderLeftLength = 4;
    loadPosLeftEnd = EAR_MAX-1;
    lastLoadLeftTime = 0;



    lastStartTimeChest = 0;
    chestRainbowRed = 1.0f;
    chestRainbowGreen = 1.0f;
    chestRainbowBlue = 1.0f;


    lastStartTimeRightFoot = 0;
    rightFootRainbowRed = 1.0f;
    rightFootRainbowGreen = 1.0f;
    rightFootRainbowBlue = 1.0f;

    lastStartTimeLeftFoot = 0;
    leftFootRainbowRed = 1.0f;
    leftFootRainbowGreen = 1.0f;
    leftFootRainbowBlue = 1.0f;
}

void LEDHandler::cycle()
{
  Chronometer time(debug(), mount_ + ".cycleTime");

  if ((cycleCount_ % 20) == 0)
  {
    rainbowCycle_++;
    switch (ledRequest_->leftEyeMode)
    {
      case EyeMode::OFF:
        setEyeLeftLEDsColor(0, 0, 0);
        break;
      case EyeMode::COLOR:
        setEyeLeftLEDsColor(ledRequest_->leftEyeR, ledRequest_->leftEyeG, ledRequest_->leftEyeB);
        break;
      case EyeMode::RAINBOW:
        setEyeLeftRainbow();
        break;
    }
    switch (ledRequest_->rightEyeMode)
    {
      case EyeMode::OFF:
        setEyeRightLEDsColor(0, 0, 0);
        break;
      case EyeMode::COLOR:
        setEyeRightLEDsColor(ledRequest_->rightEyeR, ledRequest_->rightEyeG,
                             ledRequest_->rightEyeB);
        break;
      case EyeMode::RAINBOW:
        setEyeRightRainbow();
        break;
    }
  }

    switch (ledRequest_->rightEarMode)
    {
        case EarMode ::OFF:
            setRightEarBrightness(0.0f);
            break;
        case EarMode ::BRIGHTNESS:
            setRightEarBrightness(ledRequest_->rightEarBrightness);
            break;
        case EarMode ::LOADING:
            setRightEarContinueLoading();
            break;
        case EarMode ::PROGRESS:
            setRightEarProgress(ledRequest_->rightEarProgress);
            break;
        case EarMode::PULSATE:
            setRightEarPulsating(ledRequest_->rightEarPulseSpeed);
            break;
    }
    switch (ledRequest_->leftEarMode)
    {
        case EarMode ::OFF:
            setLeftEarBrightness(0.0f);
            break;
        case EarMode ::BRIGHTNESS:
            setLeftEarBrightness(ledRequest_->leftEarBrightness);
            break;
        case EarMode ::LOADING:
            setLeftEarContinueLoading();
            break;
        case EarMode ::PROGRESS:
            setLeftEarProgress(ledRequest_->leftEarProgress);
            break;
        case EarMode::PULSATE:
            setLeftEarPulsating(ledRequest_->leftEarPulseSpeed);
            break;
    }
    switch (ledRequest_->chestMode)
    {
        case ChestMode ::OFF:
            setChestLEDs(0.0f,0.0f,0.0f);
            break;
        case ChestMode ::COLOR:
            setChestLEDs(ledRequest_->chestR,ledRequest_->chestG,ledRequest_->chestB);
            break;
        case ChestMode ::RAINBOW:
            setChestRainbowColors();
            break;
    }
    switch (ledRequest_->rightFootMode)
    {
        case FootMode ::OFF:
            setFootRightLEDs(0.0f,0.0f,0.0f);
            break;
        case FootMode ::COLOR:
            setFootRightLEDs(ledRequest_->rightFootR,ledRequest_->rightFootG,ledRequest_->rightFootB);
            break;
        case FootMode ::RAINBOW:
            setFootRightRainbowColors();
            break;
    }
    switch (ledRequest_->leftFootMode)
    {
        case FootMode ::OFF:
            setFootLeftLEDs(0.0f,0.0f,0.0f);
            break;
        case FootMode ::COLOR:
            setFootLeftLEDs(ledRequest_->leftFootR,ledRequest_->leftFootG,ledRequest_->leftFootB);
            break;
        case FootMode ::RAINBOW:
            setFootLeftRainbowColors();
            break;
    }
    robotInterface().setLEDs(cmd_);

  cycleCount_++;
}

//region basefunctions

void LEDHandler::setChestLEDs(const float red, const float green, const float blue)
{
  cmd_[0] = blue;
  cmd_[1] = green;
  cmd_[2] = red;
}



void LEDHandler::setEyeLeftLEDsColor(const float red, const float green, const float blue)
{
  const unsigned int base = CHEST_MAX + 2 * EAR_MAX;
  for (unsigned int i = 0; i < 8; i++)
  {
    cmd_[base + i] = blue;
    cmd_[base + i + 8] = green;
    cmd_[base + i + 16] = red;
  }
}

void LEDHandler::setEyeRightLEDsColor(const float red, const float green, const float blue)
{
  const unsigned int base = CHEST_MAX + 2 * EAR_MAX + EYE_MAX;
  for (unsigned int i = 0; i < 8; i++)
  {
    cmd_[base + i] = blue;
    cmd_[base + i + 8] = green;
    cmd_[base + i + 16] = red;
  }
}

void LEDHandler::setFootLeftLEDs(const float red, const float green, const float blue)
{
  const unsigned int base = CHEST_MAX + 2 * EAR_MAX + 2 * EYE_MAX + HEAD_MAX;
  cmd_[base] = blue;
  cmd_[base + 1] = green;
  cmd_[base + 2] = red;
}

void LEDHandler::setFootRightLEDs(const float red, const float green, const float blue)
{
  const unsigned int base = CHEST_MAX + 2 * EAR_MAX + 2 * EYE_MAX + HEAD_MAX + FOOT_MAX;
  cmd_[base] = blue;
  cmd_[base + 1] = green;
  cmd_[base + 2] = red;
}

void LEDHandler::setEarLeftLEDs(const float* earSegmentBrightnesses)
{
  // the base index for this led group
  const unsigned int base = CHEST_MAX;
  // update all ear LEDs
  for (uint8_t ledIndex = 0; ledIndex < EAR_MAX; ledIndex++)
  {
    cmd_[base + ledIndex] = earSegmentBrightnesses[ledIndex];
  }
}

void LEDHandler::setEarRightLEDs(const float* earSegmentBrightnesses)
{
  // the base index for this led group
  const unsigned int base = CHEST_MAX + EAR_MAX;
  // update all ear LEDs
  for (uint8_t ledIndex = 0; ledIndex < EAR_MAX; ledIndex++)
  {
    cmd_[base + ledIndex] = earSegmentBrightnesses[ledIndex];
  }
}
//endregion
//region eyeRainbows

void LEDHandler::setEyeLeftRainbow()
{
    for (unsigned int i = 0; i < 8; i++)
    {
        const unsigned int rainbowCycleOffset = 1;
        int l = (rainbowCycle_ + rainbowCycleOffset + i) % 8;
        cmd_.at(CHEST_MAX + 2 * EAR_MAX + i) = rainbowLeft_[l];
        cmd_.at(CHEST_MAX + 2 * EAR_MAX + i + 8) = rainbowLeft_[(l + 8)];
        cmd_.at(CHEST_MAX + 2 * EAR_MAX + i + 16) = rainbowLeft_[(l + 16)];
    }
}

void LEDHandler::setEyeRightRainbow()
{
    for (unsigned int i = 0; i < 8; i++)
    {
        int r = (rainbowCycle_ - i) % 8;
        cmd_.at(CHEST_MAX + 2 * EAR_MAX + EYE_MAX + i) = rainbowRight_[r];
        cmd_.at(CHEST_MAX + 2 * EAR_MAX + EYE_MAX + i + 8) = rainbowRight_[r + 8];
        cmd_.at(CHEST_MAX + 2 * EAR_MAX + EYE_MAX + i + 16) = rainbowRight_[r + 16];
    }
}
//endregion
//region rightEar

void LEDHandler::setRightEarBrightness(float brightness) {
    std::vector<float> rightEar = std::vector<float>(EAR_MAX,brightness);
    setEarRightLEDs(rightEar.data());
}

void LEDHandler::setRightEarContinueLoading() {
    if((unsigned int)( cycleInfo_->startTime) - lastLoadRightTime > 200){
        lastLoadRightTime = (unsigned int)( cycleInfo_->startTime);
        std::vector<float> rightEar = std::vector<float>(EAR_MAX,0.0f);
        for(int i =0; i < loaderRightLength; i++){
            rightEar[(loadPosRightEnd + i) % (EAR_MAX)] = 1.0f;
        }
        loadPosRightEnd= (loadPosRightEnd + 1) % (EAR_MAX);
        lastLoadingRightEar =  rightEar;
        setEarRightLEDs(rightEar.data());
    }
    else {
        setEarRightLEDs(lastLoadingRightEar.data());
    }
}

void LEDHandler::setRightEarProgress(short progress) {
    std::vector<float> rightEar = std::vector<float>(EAR_MAX,0.0f);
    int fullyLoaded = progress/10;
    float lastPercentage = (progress%10)/10.0f;
    for(int i = 0;i<fullyLoaded;i++){
        rightEar[i] = 1.0f;
    }
    rightEar[fullyLoaded] = lastPercentage;
    setEarRightLEDs(rightEar.data());
}

void LEDHandler::setRightEarPulsating(uint8_t speed) {
    float brightness = 0.0f;

    if(speed != currentSpeedRight){
        resetRightEarPulsating(speed);
    }
    int halfcycleTime = speed *100/2;
    float halfCycleCompleteness = ((unsigned int)( cycleInfo_->startTime) - lastStartTimeRight) / (float)halfcycleTime;
    if(isIncreasingHalfCycleRight){
        brightness = std::min(halfCycleCompleteness,1.0f);
    }
    else {
        brightness = std::max(1.0f- halfCycleCompleteness,0.0f);
    }
    if(brightness == 1.0f || brightness == 0.0f){
        lastStartTimeRight = (unsigned int)cycleInfo_->startTime;
        isIncreasingHalfCycleRight = !isIncreasingHalfCycleRight;
    }
    std::vector<float> rightEar = std::vector<float>(EAR_MAX,brightness);
    setEarRightLEDs(rightEar.data());
}

void LEDHandler::resetRightEarPulsating(uint8_t right) {
    lastStartTimeRight = (unsigned int)cycleInfo_->startTime;
    currentSpeedRight = right;
    isIncreasingHalfCycleRight = true;
}
//endregion
//region leftEar

void LEDHandler::setLeftEarBrightness(float brightness) {
    std::vector<float> leftEar = std::vector<float>(EAR_MAX,brightness);
    setEarLeftLEDs(leftEar.data());
}

void LEDHandler::setLeftEarContinueLoading() {
    if((unsigned int)( cycleInfo_->startTime) - lastLoadLeftTime > 200){
        lastLoadLeftTime = (unsigned int)( cycleInfo_->startTime);
        std::vector<float> leftEar = std::vector<float>(EAR_MAX, 0.0f);
        for(int i =0; i < loaderLeftLength; i++){
            int index = (loadPosLeftEnd -i >=0)?loadPosLeftEnd-i:10+(loadPosLeftEnd-i);
            leftEar[index] = 1.0f;
        }
        loadPosLeftEnd= (loadPosLeftEnd -1 >=0)?loadPosLeftEnd-1:10+(loadPosLeftEnd-1);
        lastLoadingLeftEar =  leftEar;
        setEarLeftLEDs(leftEar.data());
    }
    else {
        setEarLeftLEDs(lastLoadingLeftEar.data());
    }
}

void LEDHandler::setLeftEarProgress(short progress) {
    std::vector<float> leftEar = std::vector<float>(EAR_MAX,0.0f);
    int fullyLoaded = progress/10;
    float lastPercentage = (progress%10)/10.0f;
    for(int i = 0;i<fullyLoaded;i++){
        leftEar[EAR_MAX-1-i] = 1.0f;
    }
    leftEar[EAR_MAX-1-fullyLoaded] = lastPercentage;
    setEarLeftLEDs(leftEar.data());
}

void LEDHandler::resetLeftEarPulsating(uint8_t speed) {
    lastStartTimeLeft = (unsigned int)cycleInfo_->startTime;
    currentSpeedLeft = speed;
    isIncreasingHalfCycleLeft = true;
}

void LEDHandler::setLeftEarPulsating(uint8_t speed) {
    float brightness = 0.0f;

    if(speed != currentSpeedLeft){
        resetLeftEarPulsating(speed);
    }
    int halfcycleTime = speed *100/2;
    float halfCycleCompleteness = ((unsigned int)( cycleInfo_->startTime) - lastStartTimeLeft) / (float)halfcycleTime;
    if(isIncreasingHalfCycleLeft){
        brightness = std::min(halfCycleCompleteness,1.0f);
    }
    else {
        brightness = std::max(1.0f- halfCycleCompleteness,0.0f);
    }
    if(brightness == 1.0f || brightness == 0.0f){
        lastStartTimeLeft = (unsigned int)cycleInfo_->startTime;
        isIncreasingHalfCycleLeft = !isIncreasingHalfCycleLeft;
    }
    std::vector<float> leftEar = std::vector<float>(EAR_MAX,brightness);
    setEarLeftLEDs(leftEar.data());
}




//endregion
//region chest

void LEDHandler::setChestRainbowColors() {
    if ((unsigned int) (cycleInfo_->startTime) - lastStartTimeChest > 50) {
        lastStartTimeChest = (unsigned int) (cycleInfo_->startTime);

        chestRainbowRed += diff * (float) ((rand() % 2) * 2 - 1);
        chestRainbowRed = std::fmin(1.0f, chestRainbowRed);
        chestRainbowRed = std::fmax(0.0f, chestRainbowRed);
        chestRainbowGreen += diff * (float) ((rand() % 2) * 2 - 1);
        chestRainbowGreen = std::fmin(1.0f, chestRainbowGreen);
        chestRainbowGreen = std::fmax(0.0f, chestRainbowGreen);
        chestRainbowBlue += diff * (float) ((rand() % 2) * 2 - 1);
        chestRainbowBlue = std::fmin(1.0f, chestRainbowBlue);
        chestRainbowBlue = std::fmax(0.0f, chestRainbowBlue);
    }
    setChestLEDs(chestRainbowRed, chestRainbowGreen, chestRainbowBlue);
}


//endregion
//region feet

void LEDHandler::setFootRightRainbowColors() {
    if ((unsigned int) (cycleInfo_->startTime) - lastStartTimeRightFoot > 50) {
        lastStartTimeRightFoot = (unsigned int) (cycleInfo_->startTime);

        rightFootRainbowRed += diff * (float) ((rand() % 2) * 2 - 1);
        rightFootRainbowRed = std::fmin(1.0f, rightFootRainbowRed);
        rightFootRainbowRed = std::fmax(0.0f, rightFootRainbowRed);
        rightFootRainbowGreen += diff * (float) ((rand() % 2) * 2 - 1);
        rightFootRainbowGreen = std::fmin(1.0f, rightFootRainbowGreen);
        rightFootRainbowGreen = std::fmax(0.0f, rightFootRainbowGreen);
        rightFootRainbowBlue += diff * (float) ((rand() % 2) * 2 - 1);
        rightFootRainbowBlue = std::fmin(1.0f, rightFootRainbowBlue);
        rightFootRainbowBlue = std::fmax(0.0f, rightFootRainbowBlue);
    }
    setFootRightLEDs(rightFootRainbowRed, rightFootRainbowGreen, rightFootRainbowBlue);
}

void LEDHandler::setFootLeftRainbowColors() {
    if ((unsigned int) (cycleInfo_->startTime) - lastStartTimeLeftFoot > 50) {
        lastStartTimeLeftFoot = (unsigned int) (cycleInfo_->startTime);

        leftFootRainbowRed += diff * (float) ((rand() % 2) * 2 - 1);
        leftFootRainbowRed = std::fmin(1.0f, leftFootRainbowRed);
        leftFootRainbowRed = std::fmax(0.0f, leftFootRainbowRed);
        leftFootRainbowGreen += diff * (float) ((rand() % 2) * 2 - 1);
        leftFootRainbowGreen = std::fmin(1.0f, leftFootRainbowGreen);
        leftFootRainbowGreen = std::fmax(0.0f, leftFootRainbowGreen);
        leftFootRainbowBlue += diff * (float) ((rand() % 2) * 2 - 1);
        leftFootRainbowBlue = std::fmin(1.0f, leftFootRainbowBlue);
        leftFootRainbowBlue = std::fmax(0.0f, leftFootRainbowBlue);
    }
    setFootLeftLEDs(leftFootRainbowRed, leftFootRainbowGreen, leftFootRainbowBlue);
}
//endregion