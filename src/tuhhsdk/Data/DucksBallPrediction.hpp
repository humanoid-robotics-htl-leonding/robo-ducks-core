//
// Created by obyoxar on 18/02/2020.
//
#pragma once

#include <array>
#include "Framework/DataType.hpp"
#include <Tools/Time.hpp>

struct DucksBallPrediction: public DataType<DucksBallPrediction>
{
public:
	DataTypeName name = "DucksBallPrediction";

	struct BallFrame : public Uni::From, public Uni::To {
		Vector2f position;
		Vector2f velocity;
		float certainty;
		TimePoint time;

		void toValue(Uni::Value &value) const override
		{
			value = Uni::Value(Uni::ValueType::OBJECT);
			value["position"] << position;
			value["velocity"] << velocity;
			value["certainty"] << certainty;
//			value["time"] << time;
		}
		void fromValue(const Uni::Value &value) override
		{
			value["position"] >> position;
			value["velocity"] >> velocity;
			value["certainty"] >> certainty;
//			value["time"] >> time;
		}
	};

	float age = 0;

	std::array<BallFrame, 10> forecast;

	void reset() override
	{
		age = 0;

		for(auto & it : forecast){
			it.certainty = 0;
		}
	}
	void toValue(Uni::Value &value) const override
	{
		value = Uni::Value(Uni::ValueType::OBJECT);
		value["age"] << age;
		value["forecast"] << forecast;

	}
	void fromValue(const Uni::Value &value) override
	{
		value["age"] >> age;
		value["forecast"] >> forecast;
	}
};

