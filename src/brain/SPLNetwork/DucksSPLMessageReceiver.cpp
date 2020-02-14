#include "Definitions/DucksStandardMessage.h"
#include "Definitions/SPLStandardMessage.h"

#include "DucksMessage.hpp"
#include "DucksSPLMessageReceiver.hpp"
#include "print.h"

DucksSPLMessageReceiver::DucksSPLMessageReceiver(const ModuleManagerInterface& manager)
  : Module(manager)
  , playerConfiguration_(*this)
  , splNetworkData_(*this)
  , cycleInfo_(*this)
  , rawGameControllerState_(*this)
  , rawTeamPlayers_(*this)
  , ntpData_(*this)
{
}

void DucksSPLMessageReceiver::cycle()
{
  float dt = cycleInfo_->getTimeDiff(lastTime_);
  lastTime_ = cycleInfo_->startTime;
  // increase age and remove too old players
  for (auto it = internalPlayers_.rawPlayers.begin(); it != internalPlayers_.rawPlayers.end();)
  {
    it->age += dt;
    if (it->age > 3.f)
    {
      it = internalPlayers_.rawPlayers.erase(it);
    }
    else
    {
      it++;
    }
  }
  // integrate incoming messages
  for (auto& it : splNetworkData_->messages)
  {
    const SPLStandardMessage& msg = it.first;
    // do not handle own messages and messages from penalized robots
    if (static_cast<unsigned int>(msg.playerNum) == playerConfiguration_->playerNumber)
    {
      continue;
    }
	  //TODO Why exactly isn't this check already made.
	  //Maybe hack by Erik Mayrhofer
	  std::cout << "I am " << playerConfiguration_->teamNumber << " received " << static_cast<unsigned int>(msg.teamNum) << std::endl;
	  if(static_cast<unsigned int>(msg.teamNum) != playerConfiguration_->teamNumber){
		  continue;
	  }

    RawTeamPlayer p;
    p.age = 0.0f;
    p.playerNumber = msg.playerNum;
    p.pose = Pose(msg.pose[0] * 0.001f, msg.pose[1] * 0.001f, msg.pose[2]);
    p.ballPosition = Vector2f(msg.ball[0], msg.ball[1]) * 0.001f;
    if (msg.ballAge < 0.f || msg.ballAge * 1000 >= cycleInfo_->startTime.getSystemTime())
    {
      p.timeWhenBallWasSeen = 0;
    }
    else
    {
      p.timeWhenBallWasSeen = cycleInfo_->startTime - msg.ballAge * 1000;
    }
    p.fallen = (msg.fallen > 0);
    p.penalized = p.playerNumber <= rawGameControllerState_->penalties.size() && rawGameControllerState_->penalties[p.playerNumber - 1] != Penalty::NONE;

    Ducks::DucksStandardMessage bhmsg;
    // This check is not completely safe. bhmsg.sizeOfBHULKsMessage returns the size of a message with no obstacles and no NTP messages.
    // If a malformatted message is received, bhmsg.read could read more bytes than allowed.
    if (msg.numOfDataBytes >= bhmsg.sizeOfDucksMessage() && bhmsg.read(msg.data))
    {
      const unsigned int receiveTime = it.second.getSystemTime();
      if (bhmsg.requestsNTPMessage)
      {
        NTPData::NTPRequest request;
        request.sender = msg.playerNum;
        request.origination = bhmsg.timestamp;
        request.receipt = receiveTime;
        ntpData_->ntpRequests.push_back(request);
      }
      for (auto& ntpMsg : bhmsg.ntpMessages)
      {
        if (ntpMsg.receiver == playerConfiguration_->playerNumber)
        {
          if (static_cast<unsigned int>(msg.playerNum) > ntpRobots_.size())
          {
            ntpRobots_.resize(msg.playerNum);
          }
          ntpRobots_[msg.playerNum - 1].offset = static_cast<int>(ntpMsg.requestReceipt - ntpMsg.requestOrigination + bhmsg.timestamp - receiveTime) / 2;
          ntpRobots_[msg.playerNum - 1].valid = true;
        }
      }
      // figure out whether robot is a HULK
      p.isHULK = (bhmsg.member == HULKS_MEMBER);
      // add local obstacles of the robot to the RawTeamPlayer
      p.localObstacles = std::vector<B_HULKs::Obstacle>(bhmsg.obstacles.cbegin(), bhmsg.obstacles.cend()); //Cast DucksObstacle to BHUlks
      // convert obstacle centers back to meters because the B-HULKs message is based on millimeters
      for (auto& playerObstacle : p.localObstacles)
      {
        playerObstacle.center[0] *= 0.001f;
        playerObstacle.center[1] *= 0.001f;
      }
      // override with info from BHULKs message because it might be that the GameController does not know about a manually penalized robot
      p.penalized = bhmsg.isPenalized;
      p.keeperWantsToPlayBall = bhmsg.kingIsPlayingBall;
      p.currentPassTarget = bhmsg.passTarget;
      p.currentlyPerformingRole = bhmsg.currentlyPerfomingRole;
      p.roleAssignments.resize(BHULKS_STANDARD_MESSAGE_MAX_NUM_OF_PLAYERS);
      for (unsigned int i = 0; i < BHULKS_STANDARD_MESSAGE_MAX_NUM_OF_PLAYERS; i++)
      {
        p.roleAssignments[i] = bhmsg.roleAssignments[i];
      }
      p.headYaw = bhmsg.headYawAngle;
      if (ntpRobots_.size() >= static_cast<unsigned int>(msg.playerNum) && ntpRobots_[msg.playerNum - 1].valid)
      {
        p.timeWhenReachBall = std::max<int>(0, (bhmsg.timeWhenReachBall - ntpRobots_[msg.playerNum - 1].offset));
        p.timeWhenReachBallStriker = std::max<int>(0, (bhmsg.timeWhenReachBallQueen - ntpRobots_[msg.playerNum - 1].offset));
        p.timestampLastJumped = std::max<int>(0, (bhmsg.timestampLastJumped - ntpRobots_[msg.playerNum - 1].offset));
        p.lastTimeWhistleHeard = std::max<int>(0, (bhmsg.lastTimeWhistleDetected - ntpRobots_[msg.playerNum - 1].offset));
        p.timeWhenBallWasSeen = std::max<int>(0, (bhmsg.ballTimeWhenLastSeen - ntpRobots_[msg.playerNum - 1].offset));
      }
      else
      {
        p.timeWhenReachBall = cycleInfo_->startTime + 600000;
        p.timeWhenReachBallStriker = cycleInfo_->startTime + 600000;
        p.timestampLastJumped = 0;
        p.lastTimeWhistleHeard = 0;
      }

      // Here the hulks message is being processed if there is any.
      Ducks::DucksMessage hulksMessage;
      if (msg.numOfDataBytes >= bhmsg.sizeOfDucksMessage() + hulksMessage.sizeOfDucksMessage() && hulksMessage.read(msg.data + bhmsg.sizeOfDucksMessage()))
      {
        p.isPoseValid = hulksMessage.isPoseValid;
        p.walkingTo = hulksMessage.walkingTo;
        p.ballVelocity = Vector2f(hulksMessage.ballVel[0], hulksMessage.ballVel[1]);
        p.currentSearchPosition = hulksMessage.ballSearchData.currentSearchPosition;
        p.isAvailableForBallSearch = hulksMessage.ballSearchData.availableForSearch;

        for (uint8_t i = 0; i < MAX_NUM_PLAYERS; i++)
        {
          p.suggestedSearchPositionsValidity[i] = static_cast<bool>(hulksMessage.ballSearchData.positionSuggestionsValidity & (1 << i));
          p.suggestedSearchPositions[i] = hulksMessage.ballSearchData.positionSuggestions[i];
        }

        if (ntpRobots_.size() >= static_cast<unsigned int>(msg.playerNum) && ntpRobots_[msg.playerNum - 1].valid)
        {
          p.timestampBallSearchMapUnreliable = std::max<int>(0,
            (hulksMessage.ballSearchData.timestampBallSearchMapUnreliable - ntpRobots_[msg.playerNum - 1].offset));
        }

        p.mostWisePlayerNumber = hulksMessage.ballSearchData.mostWisePlayerNumber;
      }
    }
    else
    {
      p.isHULK = false;
      p.currentlyPerformingRole = PlayingRole::DEFENDER;
      p.headYaw = 0;
      p.timeWhenReachBall = cycleInfo_->startTime + 600000;
      p.timeWhenReachBallStriker = cycleInfo_->startTime + 600000;
      p.lastTimeWhistleHeard = 0;
      p.currentPassTarget = -1;
      p.keeperWantsToPlayBall = false;
    }
    bool merged = false;
    for (auto& it2 : internalPlayers_.rawPlayers)
    {
      if (it2.playerNumber == p.playerNumber)
      {
        it2 = p;
        merged = true;
        break;
      }
    }
    if (!merged)
    {
      internalPlayers_.rawPlayers.push_back(p);
    }
  }
  internalPlayers_.activePlayers = 0;
  internalPlayers_.activeHULKPlayers = 0;
  for (auto& player : internalPlayers_.rawPlayers)
  {
    if (!player.penalized)
    {
      internalPlayers_.activePlayers++;
      if (player.isHULK)
      {
        internalPlayers_.activeHULKPlayers++;
      }
    }
  }
  *rawTeamPlayers_ = internalPlayers_;

  debug().update(mount_ + ".RawTeamPlayers", *rawTeamPlayers_);
}
