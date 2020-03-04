#include <csignal>
#include "Tools/Chronometer.hpp"

#include "ActionCommand.hpp"
#include "BehaviorModule.hpp"
#include "Units.hpp"

BehaviorModule::BehaviorModule(const ModuleManagerInterface& manager)
  : Module(manager)
  , remoteMotionRequest_(*this, "remoteMotionRequest",
                         [this] {
                           std::lock_guard<std::mutex> lg(actualRemoteMotionRequestLock_);
                           actualRemoteMotionRequest_ = remoteMotionRequest_();
                         })
  , useRemoteMotionRequest_(*this, "useRemoteMotionRequest", [] {})
  , gameControllerState_(*this)
  , ballState_(*this)
  , robotPosition_(*this)
  , bodyPose_(*this)
  , playerConfiguration_(*this)
  , playingRoles_(*this)
  , motionState_(*this)
  , headPositionData_(*this)
  , headMotionOutput_(*this)
  , teamBallModel_(*this)
  , teamPlayers_(*this)
  , ballSearchPosition_(*this)
  , fieldDimensions_(*this)
  , strikerAction_(*this)
  , penaltyStrikerAction_(*this)
  , kickConfigurationData_(*this)
  , keeperAction_(*this)
  , penaltyKeeperAction_(*this)
  , cycleInfo_(*this)
  , setPosition_(*this)
  , defendingPosition_(*this)
  , bishopPosition_(*this)
  , supportingPosition_(*this)
  , replacementKeeperAction_(*this)
  , buttonData_(*this)
  , worldState_(*this)
  , headOffData_(*this)
  , motionRequest_(*this)
  , audioRequest_(*this)
  , playbackData_(*this)
  , ledRequest_(*this)
  , thoughtControlRequest_(*this)
  , actionCommand_(ActionCommand::dead())
  , dataSet_(*this, *gameControllerState_, *ballState_, *robotPosition_, *bodyPose_,
             *playerConfiguration_, *playingRoles_, *motionState_, *headMotionOutput_,
             *teamBallModel_, *teamPlayers_, *fieldDimensions_, *strikerAction_,
             *penaltyStrikerAction_, *keeperAction_, *penaltyKeeperAction_, *cycleInfo_,
             *setPosition_, *defendingPosition_, *bishopPosition_, *supportingPosition_,
             *replacementKeeperAction_, *buttonData_, *worldState_, *kickConfigurationData_,
             *ballSearchPosition_, *headPositionData_, actionCommand_)
{
  {
    // This is needed because callbacks are called asynchronously and a MotionRequest is large
    // enough that it is too dangerous.
    std::lock_guard<std::mutex> lg(actualRemoteMotionRequestLock_);
    actualRemoteMotionRequest_ = remoteMotionRequest_();
  }
  useRemoteMotionRequest_() = false;
  print("Behaviour - Init: ", LogLevel::INFO);
  print(" ==== Behaviour is using HULKs BehaviourModule ==== ", LogLevel::INFO);
}

void BehaviorModule::cycle() {
  Chronometer time(debug(), mount_ + ".cycle_time");

  if (
          useRemoteMotionRequest_() &&
          gameControllerState_->gameState == GameState::PLAYING &&
          gameControllerState_->penalty == Penalty::NONE &&
          !bodyPose_->fallen
          ) {
    std::lock_guard<std::mutex> lg(actualRemoteMotionRequestLock_);
    *motionRequest_ = actualRemoteMotionRequest_;
  } else {
//    thoughts_->pushState(gameControllerState_->gameState)

	  if (headOffData_->shouldDie) {
		  actionCommand_ = ActionCommand::dead().combineChestLED(ActionCommand::ChestLED::rainbow());
	  }
	  else {
		  actionCommand_ = hulks::rootBehavior(dataSet_);
	  }
	  if (headOffData_->shouldDieSignal) {
		  actionCommand_ = ActionCommand::dead()
			  .combineChestLED(ActionCommand::ChestLED::red())
			  .combineAudio(ActionCommand::Audio::audioC5());
	  }
	  if(headOffData_->lastCycle) {
		  actionCommand_ = ActionCommand::dead();
	  }
    actionCommand_.toMotionRequest(*motionRequest_);
//    actionCommand_.toEyeLEDRequest(*eyeLEDRequest_);
    actionCommand_.toAudioRequest(*audioRequest_);
//    actionCommand_.toEarLEDRequest(*earLEDRequest_);
    actionCommand_.toThoughtControlRequest(*thoughtControlRequest_);
//    actionCommand_.toChestLEDRequest(*chestLEDRequest_);
    actionCommand_.toLEDRequest(*ledRequest_);
    //actionCommand_.toPlaybackData(*playbackData_);
  }
}
