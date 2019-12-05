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
  , eyeLEDRequest_(*this)
  , earLEDRequest_(*this)
  , chestLEDRequest_(*this)
  , gameControllerState_(*this)
  , whistleData_(*this)
  , cmd_(CHEST_MAX + 2 * EAR_MAX + 2 * EYE_MAX + HEAD_MAX + 2 * FOOT_MAX, 0.f)
  , cycleCount_(0)
  , rainbowCycle_(0)
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
    rainbowRed = 1.0f;
    rainbowGreen = 1.0f;
    rainbowBlue = 1.0f;
}

void LEDHandler::cycle()
{
  Chronometer time(debug(), mount_ + ".cycleTime");

  if ((cycleCount_ % 20) == 0)
  {
    rainbowCycle_++;
    setEarLeftLEDsCharge(1.f, 1.f);
    setEarRightLEDsCharge(1.f, 1.f);
    switch (eyeLEDRequest_->leftEyeMode)
    {
      case EyeMode::OFF:
        setEyeLeftLEDsColor(0, 0, 0);
        break;
      case EyeMode::COLOR:
        setEyeLeftLEDsColor(eyeLEDRequest_->leftR, eyeLEDRequest_->leftG, eyeLEDRequest_->leftB);
        break;
      case EyeMode::RAINBOW:
        setEyeLeftRainbow();
        break;
    }
    switch (eyeLEDRequest_->rightEyeMode)
    {
      case EyeMode::OFF:
        setEyeRightLEDsColor(0, 0, 0);
        break;
      case EyeMode::COLOR:
        setEyeRightLEDsColor(eyeLEDRequest_->rightR, eyeLEDRequest_->rightG,
                             eyeLEDRequest_->rightB);
        break;
      case EyeMode::RAINBOW:
        setEyeRightRainbow();
        break;
    }

    switch (earLEDRequest_->rightEarMode)
    {
        case EarMode ::OFF:
            setRightEarBrightness(0.0f);
            break;
            case EarMode ::BRIGHTNESS:
              setRightEarBrightness(earLEDRequest_->brightnessRight);
              break;
          case EarMode ::LOADING:
              setRightEarContinueLoading();
              break;
          case EarMode ::PROGRESS:
                setRightEarProgress(earLEDRequest_->progressRight);
          break;
          case EarMode::PULSATE:
                setRightEarPulsating(earLEDRequest_->speedRight);
              break;
      }
      switch (earLEDRequest_->leftEarMode)
      {
          case EarMode ::OFF:
              setLeftEarBrightness(0.0f);
              break;
          case EarMode ::BRIGHTNESS:
              setLeftEarBrightness(earLEDRequest_->brightnessLeft);
              break;
          case EarMode ::LOADING:
              setLeftEarContinueLoading();
              break;
          case EarMode ::PROGRESS:
              setLeftEarProgress(earLEDRequest_->progressLeft);
              break;
          case EarMode::PULSATE:
              setLeftEarPulsating(earLEDRequest_->speedLeft);
              break;
      }
      switch (chestLEDRequest_->chestMode)
      {
          case ChestMode ::OFF:
              setChestLEDs(0.0f,0.0f,0.0f);
              break;
          case ChestMode ::COLOR:
              setChestLEDs(chestLEDRequest_->red,chestLEDRequest_->green,chestLEDRequest_->blue);
              break;
          case ChestMode ::RAINBOW:
              setChestRainbowColors();
              break;
      }

    //showRobotStateOnChestLEDs();
    showTeamColorOnLeftFootLEDs();
    showKickOffTeamOnRightFootLEDs();
    //showWhistleStatusOnEarLEDs();
    robotInterface().setLEDs(cmd_);
  }
  cycleCount_++;
}

void LEDHandler::setChestLEDs(const float red, const float green, const float blue)
{
  cmd_[0] = blue;
  cmd_[1] = green;
  cmd_[2] = red;
}

void LEDHandler::setEarLeftLEDsCharge(const float charge, const float value)
{
  const unsigned int base = CHEST_MAX;
  const unsigned int ledCount = EAR_MAX * charge;
  for (unsigned int i = 0; i < EAR_MAX; i++)
  {
    if (i < ledCount)
    {
      cmd_[base + i] = value;
    }
    else
    {
      cmd_[base + i] = 0.0f;
    }
  }
}

void LEDHandler::setEarRightLEDsCharge(const float charge, const float value)
{
  const unsigned int base = CHEST_MAX + EAR_MAX;
  const unsigned int ledCount = EAR_MAX * charge;
  for (unsigned int i = 0; i < EAR_MAX; i++)
  {
    if (i < ledCount)
    {
      cmd_[base + i] = value;
    }
    else
    {
      cmd_[base + i] = 0.0f;
    }
  }
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

void LEDHandler::setEyeLeftRainbow()
{
  cmd_.at(CHEST_MAX + 2 * EAR_MAX + rainbowCycle_ % 24) = 1.0;
  cmd_.at(CHEST_MAX + 2 * EAR_MAX + (rainbowCycle_-8) % 24) = 0.0;
}

void LEDHandler::setEyeRightRainbow()
{
  cmd_.at(CHEST_MAX + 2 * EAR_MAX + EYE_MAX + 24 - (rainbowCycle_ % 24)) = 1.0;
  cmd_.at(CHEST_MAX + 2 * EAR_MAX + EYE_MAX + 24 - ((rainbowCycle_-8) % 24)) = 0.0;
}

void LEDHandler::showRobotStateOnChestLEDs()
{
  float redValue = 0.0f;
  float greenValue = 0.0f;
  float blueValue = 0.0f;

  // See rules section 3.2
  if (gameControllerState_->penalty != Penalty::NONE)
  {
    // Red.
    redValue = 1.0f;
  }
  else
  {
    switch (gameControllerState_->gameState)
    {
      case GameState::INITIAL:
        // Off.
        break;
      case GameState::READY:
        // Blue.
        blueValue = 1.0f;
        break;
      case GameState::SET:
        // Yellow.
        redValue = 1.0f;
        greenValue = 0.6f;
        break;
      case GameState::PLAYING:
        // Green.
        greenValue = 1.0f;
        break;
      case GameState::FINISHED:
      default:
        // Off.
        break;
    }
  }
  setChestLEDs(redValue, greenValue, blueValue);
}

void LEDHandler::showTeamColorOnLeftFootLEDs()
{
  float redValue = 0.0f, blueValue = 0.0f, greenValue = 0.0f;

  switch (gameControllerState_->teamColor)
  {
    case TeamColor::BLUE:
      blueValue = 1.0f;
      break;
    case TeamColor::RED:
      redValue = 1.0f;
      break;
    case TeamColor::YELLOW:
      redValue = 1.0f;
      greenValue = 0.6f;
      break;
    case TeamColor::BLACK:
      break;
    case TeamColor::WHITE:
      redValue = blueValue = greenValue = 1.0f;
      break;
    case TeamColor::GREEN:
      greenValue = 1.0f;
      break;
    case TeamColor::ORANGE:
      redValue = 1.0f;
      greenValue = 0.65f;
      break;
    case TeamColor::PURPLE:
      redValue = 0.5f;
      blueValue = 1.0f;
      break;
    case TeamColor::BROWN:
      redValue = greenValue = 0.15f;
      blueValue = 0.65f;
      break;
    case TeamColor::GRAY:
      redValue = blueValue = greenValue = 0.5f;
      break;
    default:
      break;
  }

  setFootLeftLEDs(redValue, greenValue, blueValue);
}

void LEDHandler::showKickOffTeamOnRightFootLEDs()
{
  const GameState state = gameControllerState_->gameState;
  const bool stateThatRequiresDisplay =
      GameState::INITIAL == state || GameState::READY == state || GameState::SET == state;
  const float value = (gameControllerState_->kickingTeam && stateThatRequiresDisplay) ? 1.0f : 0.0f;

  setFootRightLEDs(value, value, value);
}

void LEDHandler::showWhistleStatusOnEarLEDs()
{
  // Check for whistle heard in the last second and turn half of the ear LEDs on.
  if (cycleInfo_->getTimeDiff(whistleData_->lastTimeWhistleHeard) < 1.f)
  {
    const float halfEars[] = {1.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f};
    setEarRightLEDs(halfEars);
    setEarLeftLEDs(halfEars);
  }
  // Check if we are in the playing state and turn all ear LEDs on.
  else if (gameControllerState_->gameState == GameState::PLAYING)
  {
    const float fullEars[] = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};
    setEarRightLEDs(fullEars);
    setEarLeftLEDs(fullEars);
  }
  else
  {
    const float minEars[] = {1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
    setEarRightLEDs(minEars);
    setEarLeftLEDs(minEars);
  }
}


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
            int index = (loadPosLeftEnd -i >0)?loadPosLeftEnd-i:10+(loadPosLeftEnd-i);
            leftEar[index] = 1.0f;
        }
        loadPosLeftEnd= (loadPosLeftEnd -1 >0)?loadPosLeftEnd-1:10-(loadPosLeftEnd-1);
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

void LEDHandler::setChestRainbowColors() {
    if((unsigned int)( cycleInfo_->startTime) - lastStartTimeChest > 100){
        lastStartTimeChest = (unsigned int)( cycleInfo_->startTime);

        rainbowRed += diff *(float)((rand() % 2)*2 - 1);
        rainbowRed = std::fmin(1.0f,rainbowRed);
        rainbowRed = std::fmax(0.0f,rainbowRed);
        rainbowGreen +=  diff *(float)((rand() % 2)*2 - 1);
        rainbowGreen = std::fmin(1.0f,rainbowGreen);
        rainbowGreen = std::fmax(0.0f,rainbowGreen);
        rainbowBlue +=  diff *(float)((rand() % 2)*2 - 1);
        rainbowBlue = std::fmin(1.0f,rainbowBlue);
        rainbowBlue = std::fmax(0.0f,rainbowBlue);
    }
    setChestLEDs(rainbowRed,rainbowGreen,rainbowBlue);
}


//endregion