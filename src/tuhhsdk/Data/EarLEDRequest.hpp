#pragma once

#include "Framework/DataType.hpp"

#warning Using old EarLedRequest pls dont.

/**
 * @brief Modes that can be applied to single eyes.
 */
enum class EarMode
{
  OFF,
  BRIGHTNESS,
  LOADING,
  PROGRESS,
  PULSATE
};

class EarLEDRequest : public DataType<EarLEDRequest>
{
public:
  /// the name of this DataType
  DataTypeName name = "EarLEDRequest";
  /// The mode for the left eye LED
  EarMode leftEarMode;
  /// the red value of the left LED
  short progressLeft;
  uint8_t speedLeft; //in 100ms steps //period duration
  float brightnessLeft;

  EarMode rightEarMode;
  /// the red value of the right LED
  short progressRight;
    uint8_t speedRight; //in 100ms steps //period duration
  float brightnessRight;
  /**
   * @brief reset does nothing
   */
  void reset()
  {
    leftEarMode = EarMode ::OFF;
    progressLeft = 0.f;
    brightnessLeft = 0.f;

    rightEarMode = EarMode ::OFF;
    progressRight = 0.f;
    brightnessRight = 0.f;

    speedLeft = 0;
    speedRight = 0;
  }

  virtual void toValue(Uni::Value& value) const
  {
    value = Uni::Value(Uni::ValueType::OBJECT);
    value["progressLeft"] << progressLeft;
    value["brightnessLeft"] << brightnessLeft;
    value["progressRight"] << progressRight;
    value["brightnessRight"] << brightnessRight;
  }

  virtual void fromValue(const Uni::Value& value)
  {
    int32_t pLeft, pRight;
    value["progressLeft"] >> pLeft;
    value["brightnessLeft"] >> brightnessLeft;
    value["progressRight"] >> pRight;
    value["brightnessRight"] >> brightnessRight;

    progressLeft = pLeft;
    progressRight = pRight;
  }
};
