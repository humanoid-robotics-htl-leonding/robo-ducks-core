#include <Modules/Log/Log.h>
#include "Modules/NaoProvider.h"
#include "Modules/Poses.h"
#include "Tools/Kinematics/Com.h"
#include "Tools/Kinematics/ForwardKinematics.h"
#include "Tools/Kinematics/InverseKinematics.h"
#include "Tools/Math/Angle.hpp"
#include "../print.hpp"
#include "Kick.hpp"



Kick::Kick(const ModuleManagerInterface& manager)
  : Module(manager)
  , motionActivation_(*this)
  , motionRequest_(*this)
  , cycleInfo_(*this)
  , imuSensorData_(*this)
  , jointSensorData_(*this)
  , kickOutput_(*this)
  , leftKicking_(true)
  , torsoOffsetLeft_(*this, "torsoOffsetLeft", [] {})
  , torsoOffsetRight_(*this, "torsoOffsetRight", [] {})
  , forwardKickParameters_(*this, "forwardKickParameters",
                           [this] {
                             forwardKickParameters_().yawLeft2right *= TO_RAD;
                             forwardKickParameters_().shoulderRoll *= TO_RAD;
                             forwardKickParameters_().shoulderPitchAdjustment *= TO_RAD;
                             forwardKickParameters_().ankleRoll *= TO_RAD;
                             forwardKickParameters_().anklePitch *= TO_RAD;
                           })
  , sideKickParameters_(*this, "sideKickParameters",
                        [this] {
                            sideKickParameters_().yawLeft2right *= TO_RAD;
                            sideKickParameters_().shoulderRoll *= TO_RAD;
                            sideKickParameters_().shoulderPitchAdjustment *= TO_RAD;
                            sideKickParameters_().ankleRoll *= TO_RAD;
                            sideKickParameters_().anklePitch *= TO_RAD;
                            })
  , coneMeasurements_(*this, "coneMeasurements",
                        [this] {
                            coneMeasurements_().maximumAngle *= TO_RAD;
                            coneMeasurements_().sideDirectionBoundary *= TO_RAD;
                            coneMeasurements_().minimalAngle *= TO_RAD;
                            coneMeasurements_().centerKickAngle *= TO_RAD;
                            coneMeasurements_().sideKickAngle *= TO_RAD;
                        })
  , kickAdjustments_(*this, "kickAdjustments",
                        [this] {
                            kickAdjustments_().longDistanceStraightLeftAnklePitch *= TO_RAD;
                            kickAdjustments_().mediumDistanceStraightLeftAnklePitch *= TO_RAD;
                            kickAdjustments_().shortDistanceStraightLeftHipPitch *= TO_RAD;
                            kickAdjustments_().shortDistanceStraightLeftKneePitch *= TO_RAD;
                            kickAdjustments_().shortDistanceStraightLeftAnklePitch *= TO_RAD;
                        })
  , retractAdjustments_(*this, "retractAdjustments",
                       [this] {
                           retractAdjustments_().longDistanceStraightLeftAnklePitch *= TO_RAD;
                           retractAdjustments_().longDistanceStraightLeftKneePitch *= TO_RAD;
                           retractAdjustments_().longDistanceStraightLeftHipPitch *= TO_RAD;
                           retractAdjustments_().mediumDistanceStraightLeftAnklePitch *= TO_RAD;
                           retractAdjustments_().mediumDistanceStraightLeftHipPitch *= TO_RAD;
                           retractAdjustments_().mediumDistanceStraightLeftKneePitch *= TO_RAD;
                           retractAdjustments_().shortDistanceStraightLeftHipPitch *= TO_RAD;
                           retractAdjustments_().shortDistanceStraightLeftKneePitch *= TO_RAD;
                           retractAdjustments_().shortDistanceStraightLeftAnklePitch *= TO_RAD;
                       })
  , currentInterpolatorID_(interpolators_.size())
  , gyroLowPassRatio_(*this, "gyroLowPassRatio", [] {})
  , gyroForwardBalanceFactor_(*this, "gyroForwardBalanceFactor", [] {})
  , gyroSidewaysBalanceFactor_(*this, "gyroSidewaysBalanceFactor", [] {})
  , filteredGyro_(Vector2f::Zero())
{
  forwardKickParameters_().yawLeft2right *= TO_RAD;
  forwardKickParameters_().shoulderRoll *= TO_RAD;
  forwardKickParameters_().shoulderPitchAdjustment *= TO_RAD;
  forwardKickParameters_().ankleRoll *= TO_RAD;
  forwardKickParameters_().anklePitch *= TO_RAD;
  sideKickParameters_().yawLeft2right *= TO_RAD;
  sideKickParameters_().shoulderRoll *= TO_RAD;
  sideKickParameters_().shoulderPitchAdjustment *= TO_RAD;
  sideKickParameters_().ankleRoll *= TO_RAD;
  sideKickParameters_().anklePitch *= TO_RAD;
  coneMeasurements_().maximumAngle *= TO_RAD;
  coneMeasurements_().sideDirectionBoundary *= TO_RAD;
  coneMeasurements_().minimalAngle *= TO_RAD;
  coneMeasurements_().centerKickAngle *= TO_RAD;
  coneMeasurements_().sideKickAngle *= TO_RAD;
  kickAdjustments_().longDistanceStraightLeftAnklePitch *= TO_RAD;
  kickAdjustments_().mediumDistanceStraightLeftAnklePitch *= TO_RAD;
  kickAdjustments_().shortDistanceStraightLeftHipPitch *= TO_RAD;
  kickAdjustments_().shortDistanceStraightLeftKneePitch *= TO_RAD;
  kickAdjustments_().shortDistanceStraightLeftAnklePitch *= TO_RAD;
  retractAdjustments_().longDistanceStraightLeftAnklePitch *= TO_RAD;
  retractAdjustments_().longDistanceStraightLeftKneePitch *= TO_RAD;
  retractAdjustments_().longDistanceStraightLeftHipPitch *= TO_RAD;
  retractAdjustments_().mediumDistanceStraightLeftAnklePitch *= TO_RAD;
  retractAdjustments_().mediumDistanceStraightLeftHipPitch *= TO_RAD;
  retractAdjustments_().mediumDistanceStraightLeftKneePitch *= TO_RAD;
  retractAdjustments_().shortDistanceStraightLeftHipPitch *= TO_RAD;
  retractAdjustments_().shortDistanceStraightLeftKneePitch *= TO_RAD;
  retractAdjustments_().shortDistanceStraightLeftAnklePitch *= TO_RAD;
}

void Kick::cycle()
{

  filteredGyro_.x() = gyroLowPassRatio_() * filteredGyro_.x() +
                      (1.f - gyroLowPassRatio_()) * imuSensorData_->gyroscope.x();
  filteredGyro_.y() = gyroLowPassRatio_() * filteredGyro_.y() +
                      (1.f - gyroLowPassRatio_()) * imuSensorData_->gyroscope.y();

  // check if a kick is requested
  const bool incomingKickRequest =
      motionActivation_->activations[static_cast<unsigned int>(MotionRequest::BodyMotion::KICK)] ==
          1 &&
      motionRequest_->bodyMotion == MotionRequest::BodyMotion::KICK;
  if (currentInterpolatorID_ == interpolators_.size() && incomingKickRequest)
  {
    // select kick parameters based on requested kick type

    // check whether left or right foot is to be used
    leftKicking_ = motionRequest_->kickData.ballSource.y() > 0;
    // select appropriate torso offset
    const Vector3f torsoOffset = leftKicking_ ? torsoOffsetLeft_() : torsoOffsetRight_();
      KickProperties kickProperties = getFromSourceAndDestination(motionRequest_->kickData.ballSource,motionRequest_->kickData.ballDestination,motionRequest_->kickData.forceHammer);
      KickParameters kickParameters;
      switch (kickProperties.kickDirection)
      {
          case KickProperties::KICK_DIRECTION::CENTER:
              kickParameters = forwardKickParameters_();
              break;
          case KickProperties::KICK_DIRECTION::SIDE:
              kickParameters = sideKickParameters_();
              break;
          default:
              kickParameters = forwardKickParameters_();
              break;
      }



      // reset interpolators
      resetInterpolators(kickParameters, torsoOffset, kickProperties);
    // initialize kick
    currentInterpolatorID_ = 0;
  }

  // check whether kick if active
  if (currentInterpolatorID_ < interpolators_.size())
  {
    // do not move this check unless you want a segmentation fault
    if (interpolators_[currentInterpolatorID_]->finished())
    {
      // advance kick phase
      currentInterpolatorID_++;
    }
  }

  // check whether kick if active
  if (currentInterpolatorID_ < interpolators_.size())
  {
    // convert seconds to milliseconds to get time step
    const float timeStep = cycleInfo_->cycleTime * 1000;
    // get output angles from current interpolator
    std::vector<float> outputAngles = interpolators_[currentInterpolatorID_]->step(timeStep);
    // apply gyroscope feedback
    gyroFeedback(outputAngles);
    kickOutput_->angles = outputAngles;
    kickOutput_->stiffnesses = std::vector<float>(JOINTS::JOINTS_MAX, 1.f);
    kickOutput_->safeExit = false;

    // mirror output angles if right foot is used
    if (!leftKicking_)
    {
      kickOutput_->mirrorAngles();
    }
  }
  else
  {
    // default kick output
    kickOutput_->angles = Poses::getPose(Poses::READY);
    kickOutput_->stiffnesses = std::vector<float>(JOINTS::JOINTS_MAX, 0.7f);
    kickOutput_->safeExit = true;
  }
}

void Kick::resetInterpolators(const KickParameters &kickParameters, const Vector3f &torsoOffset, KickProperties properties)
{
    float kickDistance;
    double kickAngle;

    switch(properties.kickDistance){
        case KickProperties::KICK_DISTANCE::SHORT:
            kickDistance = coneMeasurements_().shortKickDistance;
            break;
        case KickProperties::KICK_DISTANCE::MEDIUM:
            kickDistance = coneMeasurements_().mediumKickDistance;
            break;
        case KickProperties::KICK_DISTANCE::LONG:
            kickDistance = coneMeasurements_().longKickDistance;
            break;
        default:
            kickDistance = coneMeasurements_().hammerKickDistance;
    }
    switch(properties.kickDirection){
        case KickProperties::KICK_DIRECTION::SIDE:
            kickAngle = coneMeasurements_().sideKickAngle;
            break;
        case KickProperties::KICK_DIRECTION::CENTER:
            kickAngle = coneMeasurements_().centerKickAngle;
            break;
        default:
            kickAngle = coneMeasurements_().centerKickAngle;
    }
  /*
   * wait before start
   */
  const std::vector<float> anglesAtKickRequest = jointSensorData_->getBodyAngles();
  const std::vector<float> readyPoseAngles = Poses::getPose(Poses::READY);
  waitBeforeStartInterpolator_.reset(anglesAtKickRequest, readyPoseAngles,
                                     kickParameters.waitBeforeStartDuration);

  /*
   * weight shift
   */
  const Vector3f weightShiftCom = kickParameters.weightShiftCom + torsoOffset;
  std::vector<float> weightShiftAngles(JOINTS::JOINTS_MAX);
  computeWeightShiftAnglesFromReferenceCom(readyPoseAngles, weightShiftCom, weightShiftAngles);
  weightShiftAngles[JOINTS::L_SHOULDER_ROLL] = kickParameters.shoulderRoll;
  weightShiftAngles[JOINTS::R_SHOULDER_ROLL] = -kickParameters.shoulderRoll;
  weightShiftInterpolator_.reset(readyPoseAngles, weightShiftAngles,
                                 kickParameters.weightShiftDuration);

  /*
   * lift foot
   */
  const float yawLeft2right = kickParameters.yawLeft2right;
  const KinematicMatrix liftFootPose = KinematicMatrix(AngleAxisf(yawLeft2right, Vector3f::UnitZ()),
                                                       kickParameters.liftFootPosition);
  std::vector<float> liftFootAngles(JOINTS::JOINTS_MAX);
  computeLegAnglesFromFootPose(weightShiftAngles, liftFootPose, liftFootAngles);
  liftFootAngles[JOINTS::L_SHOULDER_PITCH] -= kickParameters.shoulderPitchAdjustment;
  liftFootAngles[JOINTS::R_SHOULDER_PITCH] += kickParameters.shoulderPitchAdjustment;
  liftFootAngles[JOINTS::L_ANKLE_ROLL] = kickParameters.ankleRoll;
  liftFootInterpolator_.reset(weightShiftAngles, liftFootAngles, kickParameters.liftFootDuration);

  /*
   * swing foot
   */
  const KinematicMatrix swingFootPose = KinematicMatrix(
      AngleAxisf(yawLeft2right, Vector3f::UnitZ()), kickParameters.swingFootPosition);
  std::vector<float> swingFootAngles(JOINTS::JOINTS_MAX);
  computeLegAnglesFromFootPose(liftFootAngles, swingFootPose, swingFootAngles);
  swingFootAngles[JOINTS::L_SHOULDER_PITCH] += kickParameters.shoulderPitchAdjustment;
  swingFootAngles[JOINTS::R_SHOULDER_PITCH] -= kickParameters.shoulderPitchAdjustment;
  swingFootAngles[JOINTS::L_ANKLE_PITCH] += kickParameters.anklePitch;
  swingFootAngles[JOINTS::L_ANKLE_ROLL] = kickParameters.ankleRoll;
  swingFootInterpolator_.reset(liftFootAngles, swingFootAngles, kickParameters.swingFootDuration);

  /*
   * kick ball
   */
  const KinematicMatrix kickBallPose = KinematicMatrix(AngleAxisf(yawLeft2right, Vector3f::UnitZ()),
                                                       kickParameters.kickBallPosition);
  std::vector<float> kickBallAngles(JOINTS::JOINTS_MAX);
  computeLegAnglesFromFootPose(swingFootAngles, kickBallPose, kickBallAngles);
  kickBallAngles[JOINTS::L_SHOULDER_PITCH] += kickParameters.shoulderPitchAdjustment;
  kickBallAngles[JOINTS::R_SHOULDER_PITCH] -= kickParameters.shoulderPitchAdjustment;
  kickBallAngles[JOINTS::L_ANKLE_ROLL] = kickParameters.ankleRoll;


    if( kickDistance == coneMeasurements_().longKickDistance){
        kickBallAngles[JOINTS::L_ANKLE_PITCH] = kickAdjustments_().longDistanceStraightLeftAnklePitch;

        if (kickAngle == coneMeasurements_().sideKickAngle){
        }
    }
    else if (kickDistance == coneMeasurements_().mediumKickDistance){
        kickBallAngles[JOINTS::L_ANKLE_PITCH] = kickAdjustments_().mediumDistanceStraightLeftAnklePitch;

        if(kickAngle == coneMeasurements_().sideKickAngle){

        }

    }
    else {
            kickBallAngles[JOINTS::L_ANKLE_PITCH] = kickAdjustments_().shortDistanceStraightLeftAnklePitch;
            kickBallAngles[JOINTS::L_KNEE_PITCH] =kickAdjustments_().shortDistanceStraightLeftKneePitch;
            kickBallAngles[JOINTS::L_HIP_PITCH] = kickAdjustments_().shortDistanceStraightLeftHipPitch;

        if (kickAngle == coneMeasurements_().sideKickAngle){

        }



    }



  kickBallInterpolator_.reset(swingFootAngles, kickBallAngles, kickParameters.kickBallDuration);

  //kickBallInterpolator_.reset(swingFootAngles, kickBallAngles, kickTime);
  /*
     * pause
     */
  pauseInterpolator_.reset(kickBallAngles, kickBallAngles, kickParameters.pauseDuration);

  /*
   * retract foot
   */
  const KinematicMatrix retractFootPose = KinematicMatrix(
      AngleAxisf(yawLeft2right, Vector3f::UnitZ()), kickParameters.retractFootPosition);
  std::vector<float> retractFootAngles(JOINTS::JOINTS_MAX);
  computeLegAnglesFromFootPose(kickBallAngles, retractFootPose, retractFootAngles);
  retractFootAngles[JOINTS::L_SHOULDER_PITCH] -= kickParameters.shoulderPitchAdjustment;
  retractFootAngles[JOINTS::R_SHOULDER_PITCH] += kickParameters.shoulderPitchAdjustment;
  retractFootAngles[JOINTS::L_ANKLE_ROLL] = kickParameters.ankleRoll;


    if( kickDistance == coneMeasurements_().longKickDistance){ //these are currently the same, but could be changed to improve kick timings
            retractFootAngles[JOINTS::L_KNEE_PITCH]=retractAdjustments_().longDistanceStraightLeftKneePitch;
            retractFootAngles[JOINTS::L_ANKLE_PITCH]=retractAdjustments_().longDistanceStraightLeftAnklePitch;
            retractFootAngles[JOINTS::L_HIP_PITCH] =retractAdjustments_().longDistanceStraightLeftHipPitch;
        if (kickAngle == coneMeasurements_().sideKickAngle){

        }
    }
    else if (kickDistance == coneMeasurements_().mediumKickDistance){
        retractFootAngles[JOINTS::L_KNEE_PITCH]=retractAdjustments_().mediumDistanceStraightLeftKneePitch;
        retractFootAngles[JOINTS::L_ANKLE_PITCH]=retractAdjustments_().mediumDistanceStraightLeftAnklePitch;
        retractFootAngles[JOINTS::L_HIP_PITCH] =retractAdjustments_().mediumDistanceStraightLeftHipPitch;

        if (kickAngle == coneMeasurements_().sideKickAngle){
        }
    }
    else {
        retractFootAngles[JOINTS::L_KNEE_PITCH]=retractAdjustments_().shortDistanceStraightLeftKneePitch;
        retractFootAngles[JOINTS::L_ANKLE_PITCH]=retractAdjustments_().shortDistanceStraightLeftAnklePitch;
        retractFootAngles[JOINTS::L_HIP_PITCH] =retractAdjustments_().shortDistanceStraightLeftHipPitch;
        if (kickAngle == coneMeasurements_().sideKickAngle){

        }
    }


    retractFootInterpolator_.reset(kickBallAngles, retractFootAngles,
                                 kickParameters.retractFootDuration);

  /*
   * extend foot and center torso
   */
  extendFootAndCenterTorsoInterpolator_.reset(retractFootAngles, readyPoseAngles,
                                              kickParameters.extendFootAndCenterTorsoDuration);

  /*
   * wait before exit
   */
  waitBeforeExitInterpolator_.reset(readyPoseAngles, readyPoseAngles,
                                    kickParameters.waitBeforeExitDuration);
}

void Kick::computeWeightShiftAnglesFromReferenceCom(const std::vector<float>& currentAngles,
                                                    const Vector3f& weightShiftCom,
                                                    std::vector<float>& weightShiftAngles) const
{
  weightShiftAngles = currentAngles;
  // iteratively move the torso to achieve the desired CoM
  for (unsigned int i = 0; i < 5; i++)
  {
    std::vector<float> leftLegAngles(JOINTS_L_LEG::L_LEG_MAX);
    std::vector<float> rightLegAngles(JOINTS_R_LEG::R_LEG_MAX);
    separateAngles(leftLegAngles, rightLegAngles, weightShiftAngles);

    KinematicMatrix com2torso = Com::getCom(weightShiftAngles);
    const KinematicMatrix right2torso = ForwardKinematics::getRFoot(rightLegAngles);
    const KinematicMatrix com2right = right2torso.invert() * com2torso;
    const KinematicMatrix left2torso = ForwardKinematics::getLFoot(leftLegAngles);
    const KinematicMatrix com2left = left2torso.invert() * com2torso;

    const Vector3f comError = com2right.posV - weightShiftCom;

    com2torso.posV += comError;

    leftLegAngles = InverseKinematics::getLLegAngles(com2torso * com2left.invert());
    rightLegAngles = InverseKinematics::getFixedRLegAngles(
        com2torso * com2right.invert(), leftLegAngles[JOINTS_L_LEG::L_HIP_YAW_PITCH]);
    combineAngles(weightShiftAngles, currentAngles, leftLegAngles, rightLegAngles);
  }
}

void Kick::computeLegAnglesFromFootPose(const std::vector<float>& currentAngles,
                                        const KinematicMatrix& nextLeft2right,
                                        std::vector<float>& nextAngles) const
{
  std::vector<float> leftLegAngles(JOINTS_L_LEG::L_LEG_MAX);
  std::vector<float> rightLegAngles(JOINTS_R_LEG::R_LEG_MAX);
  separateAngles(leftLegAngles, rightLegAngles, currentAngles);

  // compute left and right foot pose relative to torso
  const KinematicMatrix right2torso = ForwardKinematics::getRFoot(rightLegAngles);
  const KinematicMatrix left2torso = right2torso * nextLeft2right;

  // compute left and right leg angles
  leftLegAngles = InverseKinematics::getLLegAngles(left2torso);
  rightLegAngles = InverseKinematics::getFixedRLegAngles(
      right2torso, leftLegAngles[JOINTS_L_LEG::L_HIP_YAW_PITCH]);

  combineAngles(nextAngles, currentAngles, leftLegAngles, rightLegAngles);
}

void Kick::separateAngles(std::vector<float>& left, std::vector<float>& right,
                          const std::vector<float>& body) const
{
  left.resize(JOINTS_L_LEG::L_LEG_MAX);
  right.resize(JOINTS_R_LEG::R_LEG_MAX);
  for (unsigned int i = 0; i < JOINTS_L_LEG::L_LEG_MAX; i++)
  {
    left[i] = body[JOINTS::L_HIP_YAW_PITCH + i];
    right[i] = body[JOINTS::R_HIP_YAW_PITCH + i];
  }
}

void Kick::combineAngles(std::vector<float>& result, const std::vector<float>& body,
                         const std::vector<float>& left, const std::vector<float>& right) const
{
  result = body;
  for (unsigned int i = 0; i < JOINTS_L_LEG::L_LEG_MAX; i++)
  {
    result[JOINTS::L_HIP_YAW_PITCH + i] = left[i];
    result[JOINTS::R_HIP_YAW_PITCH + i] = right[i];
  }
}

void Kick::gyroFeedback(std::vector<float>& outputAngles) const
{
  // add filtered gyroscope x and y values multiplied by gain to ankle roll and pitch, respectively
  outputAngles[JOINTS::R_ANKLE_ROLL] +=
      (leftKicking_ ? 1 : -1) * gyroSidewaysBalanceFactor_() * filteredGyro_.x();
  outputAngles[JOINTS::R_ANKLE_PITCH] += gyroForwardBalanceFactor_() * filteredGyro_.y();
}


KickProperties Kick::getFromSourceAndDestination(Vector2f source,Vector2f destination,bool forceHammer) const {
    KickProperties properties = *new KickProperties();
    //initial calculations
    float xdist=destination.x() - source.x();
    float ydist=destination.y() - source.y();
    float dist = std::sqrt(xdist*xdist+ydist*ydist);
    float ang = std::atan2(ydist,xdist); //left positive - right negative


    //corrections
    if(dist>coneMeasurements_().hammerDistanceBoundary){
        print("KickTarget-Distance is away more than "+std::to_string(coneMeasurements_().hammerDistanceBoundary)+"m, commencing HAMMER-shot",LogLevel::WARNING);
        dist= coneMeasurements_().hammerDistanceBoundary;
    }
    else if(dist >coneMeasurements_().maximumRadius){
        print("Exceeding max kick distance of "+std::to_string(coneMeasurements_().maximumRadius)+"m, kickDistance is forced onto "+std::to_string(coneMeasurements_().maximumRadius)+"m",LogLevel::WARNING);
        dist =coneMeasurements_().maximumRadius;
    }
    else if(dist <coneMeasurements_().minimalRadius){
        print("Subceeding min kick distance of "+std::to_string(coneMeasurements_().minimalRadius)+"m, kickDistance is forced onto "+std::to_string(coneMeasurements_().minimalRadius)+"m",LogLevel::WARNING);
        dist=coneMeasurements_().minimalRadius;
    }
    if (ang >coneMeasurements_().maximumAngle){
        ang =coneMeasurements_().maximumAngle;
        print("Exceeding max kick angle  of "+std::to_string(coneMeasurements_().maximumAngle)+", kickAngle is forced onto "+std::to_string(coneMeasurements_().maximumAngle),LogLevel::WARNING);
    }
    if (ang <coneMeasurements_().minimalAngle){
        ang = coneMeasurements_().minimalAngle;
        print("Subceeding min kick angle  of "+std::to_string(coneMeasurements_().minimalAngle)+", kickAngle is forced onto "+std::to_string(coneMeasurements_().minimalAngle),LogLevel::WARNING);
    }


    properties.distance = dist;
    properties.angle =ang;
    //enums
    if(std::abs(properties.angle) > coneMeasurements_().sideKickAngle){
        properties.kickDirection = KickProperties::KICK_DIRECTION::SIDE;
    }
    else {
        properties.kickDirection = KickProperties::KICK_DIRECTION::CENTER;
    }
    if(properties.distance <coneMeasurements_().shortDistanceBoundary){
        properties.kickDistance = KickProperties::KICK_DISTANCE::SHORT;
    }
    else if(properties.distance<coneMeasurements_().mediumDistanceBoundary){
        properties.kickDistance = KickProperties::KICK_DISTANCE::MEDIUM;
    }
    else if ( properties.distance <coneMeasurements_().hammerDistanceBoundary) {
        properties.kickDistance = KickProperties::KICK_DISTANCE::LONG;
    }
    else {
        properties.kickDistance = KickProperties::KICK_DISTANCE::HAMMER;
    }

    if(forceHammer){
        properties.kickDistance = KickProperties::KICK_DISTANCE::HAMMER;
    }
    return properties;
}
bool Kick::isValidKick(Vector2f source, Vector2f destination)
{
    //ballsource.y positiv und ang negativ - sonst??
    float xdist=destination.x() - source.x();
    float ydist=destination.y() - source.y();
    float ang = std::atan2(ydist,xdist); //left positive - right negative
    if(source.y() >0.0 && ang <=0.0){
        return true;
    }
    if(source.y() <0.0 && ang >=0.0){
        return true;
    }
    return false;
}

