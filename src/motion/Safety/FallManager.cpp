#include "print.hpp"

#include "Modules/Poses.h"

#include "FallManager.hpp"


FallManager::FallManager(const ModuleManagerInterface& manager)
  : Module(manager)
  , kneeDownMotionFile_(*this, "kneeDownMotionFile")
  , enabled_(*this, "enabled")
  , motionActivation_(*this)
  , motionRequest_(*this)
  , bodyPose_(*this)
  , cycleInfo_(*this)
  , jointSensorData_(*this)
  , fallManagerOutput_(*this)
  , hot_(false)
  , catchFrontDuration_(*this, "catchFrontDuration", [] {})
  , catchFrontHipPitch_(*this, "catchFrontHipPitch", [this] { catchFrontHipPitch_() *= TO_RAD; })
  , kneeDown_(*cycleInfo_, *jointSensorData_)
{
  catchFrontHipPitch_() *= TO_RAD;

  std::string motionFileRoot = robotInterface().getFileRoot() + "motions/"; // motionFile path

  // Loading MotionFile
  kneeDown_.loadFromFile(motionFileRoot + kneeDownMotionFile_());

  lastAngles_ = Poses::getPose(Poses::READY);
}

void FallManager::cycle()
{
  hot_ = enabled_() && (motionRequest_->bodyMotion == MotionRequest::BodyMotion::WALK ||
      motionRequest_->bodyMotion == MotionRequest::BodyMotion::STAND);

  if (bodyPose_->fallDirection != FallDirection::NOT_FALLING)
  {
      prepareFalling(bodyPose_->fallDirection);
  }
  if (!catchFrontInterpolator_.finished())
  {
    fallManagerOutput_->wantToSend = true;
    fallManagerOutput_->safeExit = false;
    fallManagerOutput_->angles = catchFrontInterpolator_.step(10);
    fallManagerOutput_->stiffnesses = std::vector<float>(JOINTS::JOINTS_MAX, 0.7);

  }
  else if (kneeDown_.isPlaying())
  {
    MotionFilePlayer::JointValues values = kneeDown_.cycle();
    fallManagerOutput_->wantToSend = true;
    fallManagerOutput_->safeExit = false;
    fallManagerOutput_->angles = values.angles;
    fallManagerOutput_->stiffnesses = values.stiffnesses;
  }
  else
  {
    fallManagerOutput_->angles = lastAngles_;
    fallManagerOutput_->stiffnesses = std::vector<float>(JOINTS::JOINTS_MAX, 0.7f);
    fallManagerOutput_->wantToSend = false;
    fallManagerOutput_->safeExit = true;
  }
  lastAngles_ = fallManagerOutput_->angles;
}

/**
 * @brief Reacting on the falling detected by OnCycle
 * TODO: individual reaction on falling directions
 */
void FallManager::prepareFalling(const FallDirection fallDirection)
{
  // Only react if hot
  if (!hot_)
  {
    print("Falling - but FallManager disabled", LogLevel::DEBUG);
    return;
  }

  // disable protection
  hot_ = false;
  // accomplish reaction move depenting on tendency of falling
    std::vector<float> catchFallAngles = Poses::getPose(Poses::READY);
  if (fallDirection == FallDirection::FRONT)
  {
      catchFallAngles[JOINTS::HEAD_PITCH] = -38.5 * TO_RAD; // set the head pitch to the minimum
    // set hip pitches
      catchFallAngles[JOINTS::L_HIP_PITCH] = catchFrontHipPitch_();
      catchFallAngles[JOINTS::R_HIP_PITCH] = catchFrontHipPitch_();
  }
  else if(fallDirection == FallDirection::BACK){
      catchFallAngles[JOINTS::HEAD_PITCH] = 25 * TO_RAD; // set the head pitch to the minimum
  }
  else if(fallDirection == FallDirection::LEFT){
      float elbowYaw = jointSensorData_->getBodyAngles()[JOINTS::L_ELBOW_YAW];
      float elbowRoll = jointSensorData_->getBodyAngles()[JOINTS::L_ELBOW_ROLL];
      if(elbowYaw >=1.3 &&elbowRoll <=- 1.4 ){
          catchFallAngles[JOINTS::L_SHOULDER_PITCH] = 115*TO_RAD;
          catchFallAngles[JOINTS::L_SHOULDER_ROLL]= 20*TO_RAD;
          catchFallAngles[JOINTS::L_ELBOW_ROLL] =elbowRoll;

          catchFallAngles[JOINTS::L_WRIST_YAW] = -90*TO_RAD;
          catchFallAngles[JOINTS::L_ELBOW_YAW] = 115 * TO_RAD;

          catchFallAngles[JOINTS::R_HIP_YAW_PITCH] = 35*TO_RAD;


          catchFallAngles[JOINTS::L_HIP_PITCH] = -40.0 * TO_RAD;
          catchFallAngles[JOINTS::L_KNEE_PITCH] = -5.0 * TO_RAD;

          catchFallAngles[JOINTS::R_HIP_PITCH] = -20.0 * TO_RAD;
          catchFallAngles[JOINTS::R_KNEE_PITCH] = -5 * TO_RAD;
          catchFallAngles[JOINTS::R_HIP_ROLL] = 30*TO_RAD;

          catchFallAngles[JOINTS::R_SHOULDER_ROLL]=-40*TO_RAD;

      } else{
          catchFallAngles[JOINTS::L_SHOULDER_PITCH] = 35*TO_RAD;
          catchFallAngles[JOINTS::L_SHOULDER_ROLL]= 20*TO_RAD;
          catchFallAngles[JOINTS::L_ELBOW_ROLL] =-30*TO_RAD;
          catchFallAngles[JOINTS::L_WRIST_YAW] = -90*TO_RAD;
          catchFallAngles[JOINTS::L_ELBOW_YAW] = 0 * TO_RAD;

          catchFallAngles[JOINTS::L_HIP_PITCH] = -80.0 * TO_RAD;
          catchFallAngles[JOINTS::L_KNEE_PITCH] = 110.0 * TO_RAD;

          catchFallAngles[JOINTS::R_HIP_PITCH] = -20.0 * TO_RAD;
          catchFallAngles[JOINTS::R_KNEE_PITCH] = -5 * TO_RAD;
          catchFallAngles[JOINTS::R_HIP_ROLL] = -30*TO_RAD;

      }
  }
  else if(fallDirection == FallDirection::RIGHT){


      float elbowYaw = jointSensorData_->getBodyAngles()[JOINTS::R_ELBOW_YAW];
      float elbowRoll = jointSensorData_->getBodyAngles()[JOINTS::R_ELBOW_ROLL];
      if(elbowYaw <=-1.3 &&elbowRoll >= 1.4 ){
          catchFallAngles[JOINTS::R_SHOULDER_PITCH] = 115*TO_RAD;
          catchFallAngles[JOINTS::R_SHOULDER_ROLL]= -20*TO_RAD;
          catchFallAngles[JOINTS::R_ELBOW_ROLL] =elbowRoll;

          catchFallAngles[JOINTS::R_WRIST_YAW] = -90*TO_RAD;
          catchFallAngles[JOINTS::R_ELBOW_YAW] = -115 * TO_RAD;

          catchFallAngles[JOINTS::R_HIP_YAW_PITCH] = 35*TO_RAD;


          catchFallAngles[JOINTS::R_HIP_PITCH] = -40.0 * TO_RAD;
          catchFallAngles[JOINTS::R_KNEE_PITCH] = -5.0 * TO_RAD;
          catchFallAngles[JOINTS::L_HIP_PITCH] = -20.0 * TO_RAD;
          catchFallAngles[JOINTS::L_KNEE_PITCH] = -5 * TO_RAD;
          catchFallAngles[JOINTS::L_HIP_ROLL] = 30*TO_RAD;

          catchFallAngles[JOINTS::L_SHOULDER_ROLL]=40*TO_RAD;
      } else{
          catchFallAngles[JOINTS::R_SHOULDER_PITCH] = 35*TO_RAD;
          catchFallAngles[JOINTS::R_SHOULDER_ROLL]= -20*TO_RAD;
          catchFallAngles[JOINTS::R_ELBOW_ROLL] =30*TO_RAD;
          catchFallAngles[JOINTS::R_WRIST_YAW] = -90*TO_RAD;
          catchFallAngles[JOINTS::R_ELBOW_YAW] = 0 * TO_RAD;

          catchFallAngles[JOINTS::R_HIP_PITCH] = -80.0 * TO_RAD;
          catchFallAngles[JOINTS::R_KNEE_PITCH] = 110.0 * TO_RAD;

          catchFallAngles[JOINTS::L_HIP_PITCH] = -20.0 * TO_RAD;
          catchFallAngles[JOINTS::L_KNEE_PITCH] = -5 * TO_RAD;
          catchFallAngles[JOINTS::L_HIP_ROLL] = 30*TO_RAD;

      }


  }

    catchFrontInterpolator_.reset(jointSensorData_->getBodyAngles(), catchFallAngles,
                                  fallPreparationMovementDuration_);
}
