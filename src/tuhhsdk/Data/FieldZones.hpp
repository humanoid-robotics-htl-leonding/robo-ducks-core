#pragma once

#include <cmath>

#include "Framework/DataType.hpp"
#include "Modules/Configuration/Configuration.h"
#include "Tools/Math/Eigen.hpp"

class FieldZones : public DataType<FieldZones>
{
public:
	/// the name of this DataType
	DataTypeName name = "FieldZones";
	/// the from coordinates of the defender kick zone
	Vector2f defenderKickFrom = {0, 0};
	/// the to coordinates of the defender kick zone
	Vector2f defenderKickTo = {0, 0};
	/// the from coordinates of the defender dribble zone
	Vector2f defenderDribbleFrom = {0, 0};
	/// the to coordinates of the defender dribble zone
	Vector2f defenderDribbleTo = {0, 0};

	Vector2f keeperFrom = {0, 0};
	Vector2f keeperTo = {0, 0};

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
	bool isInsideDefenderKick(const Vector2f& position) const
	{
		return ((position.x() >= defenderKickFrom.x() && position.x() <= defenderKickTo.x()) ||
				(position.x() <= defenderKickFrom.x() && position.x() >= defenderKickTo.x())) &&
			   ((position.y() >= defenderKickFrom.y() && position.y() <= defenderKickTo.y()) ||
				(position.y() <= defenderKickFrom.y() && position.y() >= defenderKickTo.y()));
	}

	/**
	 * @brief isInsideDefenderDribble determines whether a position is in the defender dribble zone
	 * @param position a position in field coordinates
	 * @return true if the position is in the defender dribble zone
	 */
	bool isInsideDefenderDribble(const Vector2f& position) const
	{
		return ((position.x() >= defenderDribbleFrom.x() && position.x() <= defenderDribbleTo.x()) ||
				(position.x() <= defenderDribbleFrom.x() && position.x() >= defenderDribbleTo.x())) &&
			   ((position.y() >= defenderDribbleFrom.y() && position.y() <= defenderDribbleTo.y()) ||
				(position.y() <= defenderDribbleFrom.y() && position.y() >= defenderDribbleTo.y()));
	}

	bool isInsideKeeper(const Vector2f& position) const
	{
		return ((position.x() >= keeperFrom.x() && position.x() <= keeperTo.x()) ||
				(position.x() <= keeperFrom.x() && position.x() >= keeperTo.x())) &&
			   ((position.y() >= keeperFrom.y() && position.y() <= keeperTo.y()) ||
				(position.y() <= keeperFrom.y() && position.y() >= keeperTo.y()));
	}



	virtual void toValue(Uni::Value& value) const
	{
		value = Uni::Value(Uni::ValueType::OBJECT);
		value["defenderKickFrom"] << defenderKickFrom;
		value["defenderKickTo"] << defenderKickTo;
		value["defenderDribbleFrom"] << defenderDribbleFrom;
		value["defenderDribbleTo"] << defenderDribbleTo;
		value["keeperFrom"] << keeperFrom;
		value["keeperTo"] << keeperTo;
	}

	virtual void fromValue(const Uni::Value& value)
	{
		value["defenderKickFrom"] >> defenderKickFrom;
		value["defenderKickTo"] >> defenderKickTo;
		value["defenderDribbleFrom"] >> defenderDribbleFrom;
		value["defenderDribbleTo"] >> defenderDribbleTo;
		value["keeperFrom"] >> keeperFrom;
		value["keeperTo"] >> keeperTo;
	}

	/**
	 * @brief init loads the field dimensions from a configuration file
	 * @param config a reference to the configuration provider
	 */
	void init(Configuration& config)
	{
		config.mount("tuhhSDK.FieldZones", "zones.json", ConfigurationType::HEAD);

		// read field parameters
		auto group = config.get("tuhhSDK.FieldZones", "defenderKick");

		group["from"] >> defenderKickFrom;
		group["to"] >> defenderKickTo;

		group = config.get("tuhhSDK.FieldZones", "defenderDribble");

		group["from"] >> defenderDribbleFrom;
		group["to"] >> defenderDribbleTo;

		group = config.get("tuhhSDK.FieldZones", "keeper");

		group["from"] >> keeperFrom;
		group["to"] >> keeperTo;
	}
};


