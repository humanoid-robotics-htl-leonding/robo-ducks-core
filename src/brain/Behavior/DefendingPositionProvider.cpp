
#include <cmath>
#include "Tools/Chronometer.hpp"
#include "Tools/Math/Angle.hpp"
#include "Tools/Math/Eigen.hpp"
#include "Tools/Math/Hysteresis.hpp"
#include "Tools/Math/Range.hpp"

#include "DefendingPositionProvider.hpp"


DefendingPositionProvider::DefendingPositionProvider(const ModuleManagerInterface& manager)
  : Module(manager)
  , fieldDimensions_(*this)
  , gameControllerState_(*this)
  , obstacleData_(*this)
  , playingRoles_(*this)
  , robotPosition_(*this)
  , teamBallModel_(*this)
  , teamPlayers_(*this)
  , worldState_(*this)
  , defendingPosition_(*this)
  , passiveDefenseLineX_(-fieldDimensions_->fieldLength / 2 +
                                 (float) std::fmax(fieldDimensions_->fieldPenaltyMarkerDistance - 0.3f, 0.5f)) // Little bit of a hacky solution to deal with small field sizes.
  , neutralDefenseLineX_(passiveDefenseLineX_ + 1.f)
  , aggressiveDefenseLineX_(neutralDefenseLineX_ + 1.f)
  , passiveDefenseLineY_(fieldDimensions_->fieldPenaltyAreaWidth / 2 + 0.4f)
  , iAmFar_(false)
  , otherIsFar_(false)
  , iAmLeftOfOther_(true)
  , ballCloseToOwnGoal_(true)
{
}

void DefendingPositionProvider::cycle()
{
  Chronometer time(debug(), mount_ + ".cycle_time");
  calculateDefendingPosition();
  considerSetPlay();
}

void DefendingPositionProvider::calculateDefendingPosition()
{
  if (gameControllerState_->gameState != GameState::PLAYING ||
    playingRoles_->role != PlayingRole::DEFENDER || !teamBallModel_->seen)
  {
    return;
  }

  // find keeper, support striker and other defender if they exist
  const TeamPlayer *keeper = nullptr, *replacementKeeper = nullptr, *supportStriker = nullptr,
                   *otherDefender = nullptr;
  findRelevantTeamPlayers(keeper, replacementKeeper, supportStriker, otherDefender);

  const Vector2f absOwnGoalPosition = Vector2f(-fieldDimensions_->fieldLength / 2, 0.f);
  // the ball position is artificially limited
  Vector2f absBallPosition = teamBallModel_->position;
  const float minPositionX = -fieldDimensions_->fieldLength / 2 + 0.5f;
  absBallPosition.x() = std::max(absBallPosition.x(), minPositionX);
  const Vector2f ownGoalToBall = absBallPosition - absOwnGoalPosition;
  const Vector2f orthogonal = fieldDimensions_->goalInnerWidth / 4 *
    (worldState_->ballInLeftHalf ? 1 : -1) *
    Vector2f(-ownGoalToBall.y(), ownGoalToBall.x()).normalized();
  // the y position is computed from intersecting lines to make sure the defenders do not block the
  // sight of the keeper
  const Line<float> keeperSightLineAwayFromBall =
    Line<float>(absBallPosition - orthogonal, absOwnGoalPosition - orthogonal);
  const Line<float> keeperSightLineTowardsBall =
    Line<float>(absBallPosition + orthogonal, absOwnGoalPosition + orthogonal);

  ballCloseToOwnGoal_ = Hysteresis<float>::smallerThan(
    teamBallModel_->position.x(), aggressiveDefenseLineX_, hysteresis_, ballCloseToOwnGoal_);

  iAmFar_ = Hysteresis<float>::greaterThan(robotPosition_->pose.position.x(),
    aggressiveDefenseLineX_, hysteresis_, iAmFar_);
  otherIsFar_ = (otherDefender == nullptr) ||
    Hysteresis<float>::greaterThan(otherDefender->pose.position.x(),
      aggressiveDefenseLineX_, hysteresis_, otherIsFar_);
  // effectively one defender
  if (otherDefender == nullptr || (!iAmFar_ && otherIsFar_))
  {
    if (keeper == nullptr && replacementKeeper == nullptr)
    {
      // if there neither are a keeper and a replacement keeper, I replace the keeper
      const Line<float> ownGoalToBallLine = Line<float>(absBallPosition, absOwnGoalPosition);
      const float angleThreshold = std::atan2(
        passiveDefenseLineY_, fieldDimensions_->fieldLength / 2 + passiveDefenseLineX_);
      const float ownGoalToBallAngle = std::atan2(ownGoalToBall.y(), ownGoalToBall.x());
      if (std::fabs(ownGoalToBallAngle) > angleThreshold)
      {
        defendingPosition_->position.y() =
          worldState_->ballInLeftHalf ? passiveDefenseLineY_ : -passiveDefenseLineY_;
        print("Now I would have crashed if it wasn't for the nice Roboducks", LogLevel::INFO);
        if(minPositionX == passiveDefenseLineX_){
          print("PassiveDefenseRange has 0 go\n\n", LogLevel::INFO);
        }
        defendingPosition_->position.x() =
          Range<float>::clipToGivenRange(ownGoalToBallLine.getX(defendingPosition_->position.y()),
            minPositionX, passiveDefenseLineX_);
        defendingPosition_->valid = true;
        return;
      }
      else
      {
        defendingPosition_->position.x() = passiveDefenseLineX_;
        defendingPosition_->position.y() =
          Range<float>::clipToGivenRange(ownGoalToBallLine.getY(defendingPosition_->position.x()),
            -passiveDefenseLineY_, passiveDefenseLineY_);
        defendingPosition_->valid = true;
        return;
      }
    }
    else
    {
      // robot on passive or neutral defense line
      defendingPosition_->position.x() =
        worldState_->ballInOwnHalf ? passiveDefenseLineX_ : neutralDefenseLineX_;
      // position towards the ball and clipped if on passive defense line
      defendingPosition_->position.y() =
        defendingPosition_->position.x() == passiveDefenseLineX_
          ? Range<float>::clipToGivenRange(
          keeperSightLineTowardsBall.getY(defendingPosition_->position.x()),
          -passiveDefenseLineY_, passiveDefenseLineY_)
          : keeperSightLineTowardsBall.getY(defendingPosition_->position.x());
      defendingPosition_->valid = true;
      return;
    }
  }
    // two defenders
  else
  {
    iAmLeftOfOther_ = Hysteresis<float>::greaterThan(robotPosition_->pose.position.y(),
      otherDefender->pose.position.y(), hysteresis_,
      iAmLeftOfOther_);
    const bool ballInMySide = (worldState_->ballInLeftHalf && iAmLeftOfOther_) ||
      (!worldState_->ballInLeftHalf && !iAmLeftOfOther_);
    if (worldState_->ballInOwnHalf)
    {
      /* the ball is on our half, we need to defend.
       * one robot on neutral or passive defense line, one robot on passive defense line
       * aggressive defender should be on the same side as the ball
       * passive defender takes the other side
       */
      if (ballInMySide)
      {
        // if there is no keeper (and replacement keeper) but two defenders, the aggressive defender
        // should block the goal
        if (keeper == nullptr && replacementKeeper == nullptr)
        {
          const float angleThreshold = std::atan2(
            passiveDefenseLineY_, fieldDimensions_->fieldLength / 2 + passiveDefenseLineX_);
          const float ownGoalToBallAngle = std::atan2(ownGoalToBall.y(), ownGoalToBall.x());
          if (std::fabs(ownGoalToBallAngle) > angleThreshold)
          {
            const float minPositionX = -fieldDimensions_->fieldLength / 2 + 0.5f;
            defendingPosition_->position.y() =
              worldState_->ballInLeftHalf ? passiveDefenseLineY_ : -passiveDefenseLineY_;

            print("Now I would have crashed if it wasn't for the nice Roboducks", LogLevel::INFO);
            if(minPositionX == passiveDefenseLineX_){
              print("PassiveDefenseRange has 0 go\n\n", LogLevel::INFO);
            }
            defendingPosition_->position.x() = Range<float>::clipToGivenRange(
              keeperSightLineTowardsBall.getX(defendingPosition_->position.y()), minPositionX,
              passiveDefenseLineX_);
            defendingPosition_->valid = true;
            return;
          }
        }
        // Use neutral defense line if no supporter exists and ball is not close to own goal.
        // Otherwise, use passive defense line.
        defendingPosition_->position.x() =
          supportStriker != nullptr
            ? passiveDefenseLineX_
            : (ballCloseToOwnGoal_ ? passiveDefenseLineX_ : neutralDefenseLineX_);
        // position towards the ball and clipped if on passive defense line
        defendingPosition_->position.y() =
          defendingPosition_->position.x() == passiveDefenseLineX_
            ? Range<float>::clipToGivenRange(
            keeperSightLineTowardsBall.getY(defendingPosition_->position.x()),
            -passiveDefenseLineY_, passiveDefenseLineY_)
            : keeperSightLineTowardsBall.getY(defendingPosition_->position.x());
        defendingPosition_->valid = true;
        return;
      }
      else
      {
        // ball is not on my side; use passive defense line
        defendingPosition_->position.x() = passiveDefenseLineX_;
        // position away from ball and clip
        defendingPosition_->position.y() = Range<float>::clipToGivenRange(
          keeperSightLineAwayFromBall.getY(defendingPosition_->position.x()),
          -passiveDefenseLineY_ / 2, passiveDefenseLineY_ / 2);
        defendingPosition_->valid = true;
        return;
      }
    }
    else
    {
      /* the ball is not on our half, we can be more aggressive
       * one robot on aggressive or neutral defense line, one robot on passive defense line
       * aggressive defender should be on the same side as the ball
       * passive defender takes the other side
       */
      if (ballInMySide)
      {
        // if there is a support striker be less aggressive and use the neutral defense line
        defendingPosition_->position.x() =
          supportStriker != nullptr ? neutralDefenseLineX_ : aggressiveDefenseLineX_;
        // towards the ball
        defendingPosition_->position.y() =
          keeperSightLineTowardsBall.getY(defendingPosition_->position.x());
        defendingPosition_->valid = true;
        return;
      }
      else
      {
        // passive defender on passive defense line
        defendingPosition_->position.x() = passiveDefenseLineX_;
        // clip
        defendingPosition_->position.y() = Range<float>::clipToGivenRange(
          keeperSightLineAwayFromBall.getY(defendingPosition_->position.x()),
          -passiveDefenseLineY_ / 2, passiveDefenseLineY_);
        defendingPosition_->valid = true;
        return;
      }
    }
  }
}

void DefendingPositionProvider::findRelevantTeamPlayers(const TeamPlayer*& keeper,
                                                        const TeamPlayer*& replacementKeeper,
                                                        const TeamPlayer*& supportStriker,
                                                        const TeamPlayer*& otherDefender) const
{
  for (auto& player : teamPlayers_->players)
  {
    if (player.penalized)
    {
      continue;
    }
    if (player.currentlyPerformingRole == PlayingRole::KEEPER)
    {
      keeper = &player;
    }
    else if (player.currentlyPerformingRole == PlayingRole::REPLACEMENT_KEEPER)
    {
      replacementKeeper = &player;
    }
    else if (player.currentlyPerformingRole == PlayingRole::SUPPORT_STRIKER)
    {
      supportStriker = &player;
    }
    if (player.currentlyPerformingRole == PlayingRole::DEFENDER)
    {
      otherDefender = &player;
    }
  }
}

void DefendingPositionProvider::considerSetPlay()
{
  const bool enemyHasFreeKick =
      gameControllerState_->setPlay != SetPlay::NONE && !gameControllerState_->kickingTeam;

  // If there is no free kick there is nothing to do
  if (!enemyHasFreeKick)
  {
    return;
  }

  // If the team ball was not found, we are not able to go away from it
  if (!teamBallModel_->found)
  {
    return;
  }

  if (worldState_->ballInOwnHalf && gameControllerState_->setPlay == SetPlay::GOAL_FREE_KICK &&
      !gameControllerState_->kickingTeam)
  {
    // ref made a mistake: The ball cannot be in our own half without us being the kicking team
    // during a GOALFreeKick
    return;
  }

  // vector to the ball (relative to the robot)
  const Vector2f ballToRobot = robotPosition_->pose.position - teamBallModel_->position;
  // vector from the target position to the ball
  const Vector2f ballToTarget = defendingPosition_->position - teamBallModel_->position ;
  // set the defending position's y coord to something that is not illegal.
  if (ballToRobot.norm() < 0.9f || ballToTarget.norm() < 0.9f)
  {
    const float side = ballToRobot.y() < 0.f ? -1.f : 1.f;
    const float newYCoord = teamBallModel_->position.y() +
                            (side * std::sqrt(abs(0.9f * 0.9f - ballToRobot.x() * ballToRobot.x())));

    defendingPosition_->position.y() = newYCoord;

    debug().update(mount_ + ".modifiedDefPos", defendingPosition_->position);
  }
}
