#pragma once

#include "Framework/DataType.hpp"
#include "Tools/Math/Eigen.hpp"
#include "Tools/Math/Pose.hpp"

class DucksDefendingPosition : public DataType<DucksDefendingPosition>
{
public:
  /// the name of this DataType
  DataTypeName name = "DucksDefendingPosition";
  /// whether the playing position is valid

  bool valid = false;
  Pose targetPose = Pose(Vector2f(0, 0), 0);

  /**
   * @brief invalidates the position
   */
  void reset()
  {
    valid = false;
  }

  virtual void toValue(Uni::Value& value) const
  {
    value = Uni::Value(Uni::ValueType::OBJECT);
    value["valid"] << valid;
    value["targetPose"] << targetPose;
  }

  virtual void fromValue(const Uni::Value& value)
  {
    value["valid"] >> valid;
    value["targetPose"] >> targetPose;
  }
};
