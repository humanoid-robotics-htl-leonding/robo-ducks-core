//
// Created by max on 12/5/19.
//

#pragma once

#include "Framework/DataType.hpp"

/**
 * @brief Modes that can be applied to single eyes.
 */
enum class EyeMode
{
    OFF,
    COLOR,
    RAINBOW
};

enum class EarMode
{
    OFF,
    BRIGHTNESS,
    LOADING,
    PROGRESS,
    PULSATE
};

enum class ChestMode
{
    OFF,
    COLOR,
    RAINBOW
};

class LEDRequest : public DataType<LEDRequest>
{
public:
    DataTypeName name = "LEDRequest";

    EyeMode leftEyeMode = EyeMode::OFF;
    float leftEyeR = 0;
    float leftEyeG = 0;
    float leftEyeB = 0;

    EyeMode rightEyeMode = EyeMode::OFF;
    float rightEyeR = 0;
    float rightEyeG = 0;
    float rightEyeB = 0;

    EarMode leftEarMode = EarMode::OFF;
    uint8_t leftEarProgress = 0;
    uint8_t leftEarPulseSpeed = 0; //in 100ms steps //period duration
    float leftEarBrightness = 0;

    EarMode rightEarMode = EarMode::OFF;
    uint8_t rightEarProgress = 0;
    uint8_t rightEarPulseSpeed = 0; //in 100ms steps //period duration
    float rightEarBrightness = 0;

    ChestMode chestMode = ChestMode::OFF;
    float chestR = 0;
    float chestG = 0;
    float chestB = 0;

    void reset() override
    {
      leftEyeMode = EyeMode::OFF;
      leftEyeR = 0.f;
      leftEyeG = 0.f;
      leftEyeB = 0.f;

      rightEyeMode = EyeMode::OFF;
      rightEyeR = 0.f;
      rightEyeG = 0.f;
      rightEyeB = 0.f;


      leftEarMode = EarMode::OFF;
      leftEarProgress = 0;
      leftEarPulseSpeed = 0;
      leftEarBrightness = 0;

      rightEarMode = EarMode::OFF;
      rightEarProgress = 0;
      rightEarPulseSpeed = 0;
      rightEarBrightness = 0;


      chestMode = ChestMode::OFF;
      chestR = 0;
      chestG = 0;
      chestB = 0;
    }

    void toValue(Uni::Value& value) const override
    {
        value = Uni::Value(Uni::ValueType::OBJECT);
        value["leftEyeMode"] << static_cast<int>(leftEyeMode);
        value["leftEyeR"] << leftEyeR;
        value["leftEyeG"] << leftEyeG;
        value["leftEyeB"] << leftEyeB;

        value["rightEyeMode"] << static_cast<int>(rightEyeMode);
        value["rightEyeR"] << rightEyeR;
        value["rightEyeG"] << rightEyeG;
        value["rightEyeB"] << rightEyeB;


        value["leftEarMode"] << static_cast<int>(leftEarMode);
        value["leftEarProgress"] << leftEarProgress;
        value["leftEarPulseSpeed"] << leftEarPulseSpeed;
        value["leftEarBrightness"] << leftEarBrightness;

        value["rightEarMode"] << static_cast<int>(rightEarMode);
        value["rightEarProgress"] << rightEarProgress;
        value["rightEarPulseSpeed"] << rightEarPulseSpeed;
        value["rightEarBrightness"] << rightEarBrightness;


        value["chestMode"] << static_cast<int>(chestMode);
        value["chestR"] << chestR;
        value["chestG"] << chestG;
        value["chestB"] << chestB;
    }

    void fromValue(const Uni::Value& value) override
    {
        leftEyeMode = static_cast<EyeMode>(value["leftEyeMode"].asInt32()) ;
        value["leftEyeR"] >> leftEyeR;
        value["leftEyeG"] >> leftEyeG;
        value["leftEyeB"] >> leftEyeB;

        rightEyeMode = static_cast<EyeMode>(value["rightEyeMode"].asInt32()) ;
        value["rightEyeR"] >> rightEyeR;
        value["rightEyeG"] >> rightEyeG;
        value["rightEyeB"] >> rightEyeB;


        leftEarMode = static_cast<EarMode >(value["leftEarMode"].asInt32()) ;
        leftEarProgress = static_cast<uint8_t>(value["leftEarProgress"].asInt32());
        leftEarPulseSpeed = static_cast<uint8_t >(value["leftEarPulseSpeed"].asInt32());
        value["leftEarBrightness"] >> leftEarBrightness;

        rightEarMode = static_cast<EarMode >(value["rightEarMode"].asInt32()) ;
        rightEarProgress = static_cast<uint8_t>(value["rightEarProgress"].asInt32());
        rightEarPulseSpeed = static_cast<uint8_t >(value["rightEarPulseSpeed"].asInt32());
        value["rightEarBrightness"] >> rightEarBrightness;


        chestMode = static_cast<ChestMode >(value["chestMode"].asInt32());
        value["chestR"] >> chestR;
        value["chestG"] >> chestG;
        value["chestB"] >> chestB;
    }
};

