#pragma once

#include "Framework/DataType.hpp"

class AudioRequest : public DataType<AudioRequest>
{
public:
  DataTypeName name = "AudioRequest";
  float frequency = 0;

  void reset() override
  {
    frequency = 0;
  }

  void toValue(Uni::Value& value) const override
  {
    value = Uni::Value(Uni::ValueType::OBJECT);
    value["frequency"] << frequency;
  }

  void fromValue(const Uni::Value& value) override
  {
    value["frequency"] >> frequency;
  }
};
