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
      KICK_INTO_GOAL,
      /// passes the ball to coordinates
      PASS,
      /// waiting for the ball to reach the opposite side
      WAITING_FOR_BALL,
      // walks to ball
      WALK_TO_POS,
      // dribble to position
      DRIBBLE_TO_POS
  };
  enum KickType
  {
      /// kicks the ball
      KICK = 0,
      /// don't kick
      NONE = 1,
  };
  /// true if this struct is valid
  bool valid = false;
  /// the type of the action
  Action action = Action::WAITING_FOR_BALL;
  /// the field coordinates of the ball target
  Vector2f target = Vector2f::Zero();
  /// type of kick we want to do
  KickType kickType;
  /// the relative pose from where we want to kick from
  Pose kickPose;
  /// if ball is kickable at the moment and how
  BallUtils::Kickable kickable = BallUtils::Kickable::NOT;
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
    value["kickType"] << static_cast<int>(kickType);
    value["kickPose"] << kickPose;
    value["kickable"] << static_cast<int>(kickable);
  }
  void fromValue(const Uni::Value& value) override
  {
    value["valid"] >> valid;
    int readNumber = 0;
    value["action"] >> readNumber;
    action = static_cast<Action>(readNumber);
    value["target"] >> target;
    value["kickType"] >> readNumber;
    kickType = static_cast<KickType>(readNumber);
    value["kickPose"] >> kickPose;
    value["kickable"] >> readNumber;
    kickable = static_cast<BallUtils::Kickable>(readNumber);
  }
};
