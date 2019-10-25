#pragma once

#include "Framework/DataType.hpp"

class AudioRequest : public DataType<AudioRequest>
{
public:
  /// the name of this DataType
  DataTypeName name = "AudioRequest";
  /// The mode for the left eye LED
  float frequency;
  /**
   * @brief reset does nothing
   */
  void reset()
  {
    frequency = 0;
  }

  virtual void toValue(Uni::Value& value) const
  {
    value = Uni::Value(Uni::ValueType::OBJECT);
    value["frequency"] << frequency;
  }

  virtual void fromValue(const Uni::Value& value)
  {
    value["frequency"] >> frequency;
  }
};
