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

class LEDRequest : public DataType<LEDRequest>
{
public:
    /// the name of this DataType
    DataTypeName name = "LEDRequest";
    /// The mode for the left eye EyeLED
    EyeMode leftEyeMode;
    /// the red value of the left EyeLED
    float eyeLeftR;
    /// the green value of the left EyeLED
    float leftG;
    /// the blue value of the left EyeLED
    float leftB;
    /// The mode for the right eye EyeLED
    EyeMode rightEyeMode;
    /// the red value of the right EyeLED
    float rightR;
    /// the green value of the right EyeLED
    float rightG;
    /// the blue value of the right EyeLED
    float rightB;
    /**
     * @brief reset does nothing
     */
    void reset()
    {
        leftEyeMode = EyeMode::OFF;
        eyeLeftR = 0.f;
        leftG = 0.f;
        leftB = 0.f;

        rightEyeMode = EyeMode::OFF;
        rightR = 0.f;
        rightG = 0.f;
        rightB = 0.f;
    }

    virtual void toValue(Uni::Value& value) const
    {
        value = Uni::Value(Uni::ValueType::OBJECT);
        value["eyeLeftR"] << eyeLeftR;
        value["leftG"] << leftG;
        value["leftB"] << leftB;
        value["rightR"] << rightR;
        value["rightG"] << rightG;
        value["rightB"] << rightB;
    }

    virtual void fromValue(const Uni::Value& value)
    {
        value["eyeLeftR"] >> eyeLeftR;
        value["leftG"] >> leftG;
        value["leftB"] >> leftB;
        value["rightR"] >> rightR;
        value["rightG"] >> rightG;
        value["rightB"] >> rightB;
    }
};

