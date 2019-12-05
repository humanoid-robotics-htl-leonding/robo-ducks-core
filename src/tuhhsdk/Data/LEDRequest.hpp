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
    }
};

