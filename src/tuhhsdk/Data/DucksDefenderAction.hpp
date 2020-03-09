#pragma once

#include "Framework/DataType.hpp"
#include "Tools/Math/Eigen.hpp"
#include "Tools/Math/Pose.hpp"

class DucksDefenderAction: public DataType<DucksDefenderAction>
{
public:
	/// the name of this DataType
	DataTypeName name = "DucksDefenderAction";
	/// whether the playing position is valid

	enum Type
	{
		WALK,
		WALK_WITH_ORIENTATION,
		WALKDIRECT,
		WALKDIRECT_WITH_ORIENTATION,
		DEFEND,
		KICK
	};

	bool valid = false;
	Pose targetPose = Pose(Vector2f(0, 0), 0);
	Type type = Type::WALK;

	/**
	 * @brief invalidates the position
	 */
	void reset()
	{
		valid = false;
	}

	virtual void toValue(Uni::Value &value) const
	{
		value = Uni::Value(Uni::ValueType::OBJECT);
		value["valid"] << valid;
		value["targetPose"] << targetPose;
		value["type"] << static_cast<int>(type);
	}

	virtual void fromValue(const Uni::Value &value)
	{
		value["valid"] >> valid;
		value["targetPose"] >> targetPose;
		int readNumber = 0;
		value["type"] >> readNumber;
		type = static_cast<Type>(readNumber);
	}
};
