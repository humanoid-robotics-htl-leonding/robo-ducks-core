#pragma once
#include "Behavior/Units.hpp"


/**
 * @brief walkToBallAndKick creates an action command for walking to the ball and kick it somewhere
 * @pre The team ball has to be seen.
 * @param d a dataset
 * @param kickPose the relative (!!!) kick pose
 * @param kickable the type of kick that is currently executable (may be none)
 * @param ballDestination the position where the ball should end up
 * @param absolute true iff ballDestination is absolute
 * @param velocity the velocity
 * @param kickType the type of kick
 * @return an action command for walking to the ball and kick it somewhere
 */
ActionCommand walkToBallAndKick(const DataSet& d, const Pose& kickPose, const BallUtils::Kickable kickable, const Vector2f& ballDestination,
                                const bool absolute = false, const Velocity& velocity = Velocity())
{
  if (d.motionState.bodyMotion == MotionRequest::BodyMotion::KICK)
  {
    return ActionCommand::stand();
  }

  if (kickable != BallUtils::Kickable::NOT || d.lastActionCommand.body().type() == MotionRequest::BodyMotion::KICK)
  {
    const Vector2f relBallDestination = absolute ? d.robotPosition.fieldToRobot(ballDestination) : ballDestination;
    return ActionCommand::kick(d.ballState.position, relBallDestination);
  }
  return walkBehindBall(d, kickPose, velocity).combineHead(trackBall(d));
}

ActionCommand kickLeft(const DataSet& d)
{
  return ActionCommand::kick(Vector2f(0.17, 0.05), Vector2f(5, 0.05));
}

ActionCommand kickRight(const DataSet& d)
{
  return ActionCommand::kick(Vector2f(0.17, -0.05), Vector2f(5, -0.05));
}
