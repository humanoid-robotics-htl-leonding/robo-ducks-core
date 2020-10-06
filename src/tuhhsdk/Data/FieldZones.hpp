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
    /// the bishop patroling left zone
    Rectangle<float> bishopPatrolLeft;
/// the bishop patroling left zone
    Rectangle<float> bishopPatrolRight;
    ///the bishop shadowing ball zone
    Rectangle<float> bishopShadowBall;

    Rectangle<float> bishopPass;

    /**
	 * @brief reset does nothing
	 */
	void reset()
	{
	}

	/**
	 * @brief isInsideDefenderKick determines whether a position is in the defender kick zone
	 * @param position a position in field coordinates
	 * @param threshold "rounding error" countermeasure
	 * @return true if the position is in the defender kick zone
	 */
	bool isInside(const Vector2f& position, const Rectangle<float>& zone, const float threshold = 0.0) const
	{
		return position.x() + threshold >= zone.topLeft.x() && position.x() <= zone.bottomRight.x() + threshold&&
			position.y() + threshold >= zone.topLeft.y() && position.y() <= threshold + zone.bottomRight.y();
	}

	/**
	 * @brief gets the nearest corner of a zone to a given position
	 * @param position
	 * @param zone
	 * @return
	 */
    Vector2f nearestCorner(const Vector2f& position, const Rectangle<float>& zone) const
    {
	    Vector2f nearestCorner;
        float distanceToTopLeft = (position-zone.topLeft).norm();
        float distanceToBottomLeft = (position-zone.bottomLeft()).norm();
        float distanceToBottomRight = (position-zone.bottomRight).norm();
        float distanceToTopRight = (position-zone.topRight()).norm();

        float minDistance = std::min({distanceToBottomLeft, distanceToBottomRight, distanceToTopLeft,distanceToTopRight});
        if(minDistance == distanceToBottomLeft){
            return zone.bottomLeft();
        }
        else if(minDistance == distanceToBottomRight){
            return zone.bottomRight;
        }
        else if(minDistance == distanceToTopLeft){
            return zone.topLeft;
        }
        else {
            return zone.topRight();
        }
    }

	virtual void toValue(Uni::Value& value) const
	{
		value = Uni::Value(Uni::ValueType::OBJECT);
		value["defenderKick"] << defenderKick;
		value["defenderDribble"] << defenderDribble;
		value["keeper"] << keeper;
        value["bishopPass"] << bishopPass;
        value["bishopShadowBall"] << bishopShadowBall;
        value["bishopPatrolRight"] << bishopPatrolRight;
        value["bishopPatrolLeft"] << bishopPatrolLeft;

    }

	virtual void fromValue(const Uni::Value& value)
	{
		value["defenderKick"] >> defenderKick;
		value["defenderDribble"] >> defenderDribble;
		value["keeper"] >> keeper;
        value["bishopPass"] >> bishopPass;
        value["bishopShadowBall"] >> bishopShadowBall;
        value["bishopPatrolRight"] >> bishopPatrolRight;
        value["bishopPatrolLeft"] >> bishopPatrolLeft;
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

        group = config.get("tuhhSDK.FieldZones", "bishopPatrolRight");

        group["from"] >> from;
        group["to"] >> to;
        bishopPatrolRight = Rectangle<float>(from, to);

        group = config.get("tuhhSDK.FieldZones", "bishopPatrolLeft");

        group["from"] >> from;
        group["to"] >> to;
        bishopPatrolLeft = Rectangle<float>(from, to);

        group = config.get("tuhhSDK.FieldZones", "bishopShadowBall");

        group["from"] >> from;
        group["to"] >> to;
        bishopShadowBall = Rectangle<float>(from, to);

        group = config.get("tuhhSDK.FieldZones", "bishopPass");

        group["from"] >> from;
        group["to"] >> to;
        bishopPass = Rectangle<float>(from, to);
	}
};


