#pragma once

#include "Framework/DataType.hpp"

/**
 * @brief Modes that can be applied to single eyes.
 */
enum class ChestMode
{
  OFF,
  COLOR,
  RAINBOW
};

class ChestLEDRequest : public DataType<ChestLEDRequest> {
public:
    /// the name of this DataType
    DataTypeName name = "ChestLEDRequest";
    /// The mode for the left eye LED
    ChestMode chestMode;
    /// the red value of the left LED

    float red;
    float green;
    float blue;

    /**
     * @brief reset does nothing
     */
    void reset() {
        chestMode = ChestMode::OFF;
        red = 0.0f;
        green = 0.0f;
        blue = 0.0f;

    }

    virtual void toValue(Uni::Value &value) const {
        value = Uni::Value(Uni::ValueType::OBJECT);
        value["red"] << red;
        value["green"] << green;
        value["blue"] << blue;

    }

    virtual void fromValue(const Uni::Value &value) {
        value["red"] >> red;
        value["green"] >> green;
        value["blue"] >> blue;

    }
};
};