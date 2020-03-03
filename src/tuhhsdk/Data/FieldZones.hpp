#pragma once

#include <cmath>

#include "Framework/DataType.hpp"
#include "Modules/Configuration/Configuration.h"
#include "Tools/Math/Eigen.hpp"
#include "Tools/Math/Rectangle.hpp"

class FieldZones : public DataType<FieldZones>
{
public:
	/// the name of this DataType
	DataTypeName name = "FieldZones";
	/// the defender kick zone
	Rectangle<float> defenderKick;
	/// the defender dribble zone
	Rectangle<float> defenderDribble;
	/// the keeper zone
	Rectangle<float> keeper;

	/**
	 * @brief reset does nothing
	 */
	void reset()
	{
	}

	/**
	 * @brief isInsideDefenderKick determines whether a position is in the defender kick zone
	 * @param position a position in field coordinates
	 * @return true if the position is in the defender kick zone
	 */
	bool isInside(const Vector2f& position, const Rectangle<float>& zone) const
	{
		return position.x() >= zone.topLeft.x() && position.x() <= zone.bottomRight.x() &&
			position.y() >= zone.topLeft.y() && position.y() <= zone.bottomRight.y();
	}



	virtual void toValue(Uni::Value& value) const
	{
		value = Uni::Value(Uni::ValueType::OBJECT);
		value["defenderKick"] << defenderKick;
		value["defenderDribble"] << defenderDribble;
		value["keeper"] << keeper;
	}

	virtual void fromValue(const Uni::Value& value)
	{
		value["defenderKick"] >> defenderKick;
		value["defenderDribble"] >> defenderDribble;
		value["keeper"] >> keeper;
	}

	/**
	 * @brief init loads the field dimensions from a configuration file
	 * @param config a reference to the configuration provider
	 */
	void init(Configuration& config)
	{
		config.mount("tuhhSDK.FieldZones", "zones.json", ConfigurationType::HEAD);

		// read field parameters
		Vector2f from;
		Vector2f to;

		auto group = config.get("tuhhSDK.FieldZones", "defenderKick");

		group["from"] >> from;
		group["to"] >> to;
		defenderKick = Rectangle<float>(from, to);

		group = config.get("tuhhSDK.FieldZones", "defenderDribble");

		group["from"] >> from;
		group["to"] >> to;
		defenderDribble = Rectangle<float>(from, to);

		group = config.get("tuhhSDK.FieldZones", "keeper");

		group["from"] >> from;
		group["to"] >> to;
		keeper = Rectangle<float>(from, to);
	}
};


