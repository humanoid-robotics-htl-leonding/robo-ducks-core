#pragma once

#include "Framework/DataType.hpp"
#include "Tools/BallUtils.hpp"
#include "Tools/Math/Pose.hpp"
#include "Tools/Math/Eigen.hpp"


class DucksStrikerAction : public DataType<DucksStrikerAction>
{
public:
  /// the name of this DataType
  DataTypeName name = "DucksStrikerAction";
  /**
   * @enum Type enumerates the possible types of action for a striker
   */
  enum Action
  {
      /// kicks the ball into the goal
      KICK_TO_POS,
      /// passes the ball to coordinates
      PASS,
      /// waiting for the ball to reach the opposite side
      WAITING_FOR_BALL,
      // walks to ball
      WALK_TO_POS,
      // dribble to position
      DRIBBLE_TO_POS
  };

  /// true if this struct is valid
  bool valid = false;
  /// the type of the action
  Action action = Action::WAITING_FOR_BALL;
  /// the field coordinates of the ball target
  Vector2f target = Vector2f::Zero();
  /// target where we want the robot to be
  Pose walkTarget;

  /**
   * @brief reset does nothing
   */
  void reset() override
  {
    valid = false;
  }

  void toValue(Uni::Value& value) const override
  {
    value = Uni::Value(Uni::ValueType::OBJECT);
    value["valid"] << valid;
    value["action"] << static_cast<int>(action);
    value["target"] << target;
    value["walkTarget"] << walkTarget;
  }
  void fromValue(const Uni::Value& value) override
  {
    value["valid"] >> valid;
    int readNumber = 0;
    value["action"] >> readNumber;
    action = static_cast<Action>(readNumber);
    value["target"] >> target;
    value["walkTarget"] >> walkTarget;
  }
};
