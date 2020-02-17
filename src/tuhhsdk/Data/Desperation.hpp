#pragma once

#include "Framework/DataType.hpp"
#include "Tools/Time.hpp"

#include "Tools/Math/Eigen.hpp"

class Desperation : public DataType<Desperation>
{
public:
	DataTypeName name = "Desperation";

	Desperation()
	: lookAtBallUrgency(0)
	{};


  float lookAtBallUrgency;

  void reset() override
  {
  	lookAtBallUrgency = 0;
  }

  void toValue(Uni::Value& value) const override
  {
    value = Uni::Value(Uni::ValueType::OBJECT);
    value["lookAtBallUrgency"] << lookAtBallUrgency;
  }

  void fromValue(const Uni::Value& value) override
  {
    value["lookAtBallUrgency"] >> lookAtBallUrgency;
  }
};
