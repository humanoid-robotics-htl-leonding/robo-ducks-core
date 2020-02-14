#pragma once

#include "Tools/Math/Eigen.hpp"

#include "Data/CycleInfo.hpp"
#include "Data/IMUSensorData.hpp"
#include "Data/JointSensorData.hpp"
#include "Data/KickOutput.hpp"
#include "Data/MotionActivation.hpp"
#include "Framework/Module.hpp"
#include "Tools/Kinematics/KinematicMatrix.h"

#include "Utils/Interpolator/Interpolator.hpp"

class Motion;

/**
 * @brief execute a dynamic kick that adapts to the current ball position
 */
class Kick : public Module<Kick, Motion>
{
public:
  /// the name of this module
  ModuleName name = "Kick";
  /**
   * @brief the Kick class
   * @param manager a reference to motion
   */
  Kick(const ModuleManagerInterface& manager);

  void cycle();

private:
  /// a reference to the motion activation
  const Dependency<MotionActivation> motionActivation_;

  /// a reference to the motion request
  const Dependency<MotionRequest> motionRequest_;

  /// a reference to the cycle info
  const Dependency<CycleInfo> cycleInfo_;

  /// a reference to the imu sensor data
  const Dependency<IMUSensorData> imuSensorData_;

  /// a reference to the joint sensor data
  const Dependency<JointSensorData> jointSensorData_;

  /// a reference to the kick output
  Production<KickOutput> kickOutput_;

  /// whether the left or right foot is supposed to kick
  bool leftKicking_;

  /// torso offset for left kick
  const Parameter<Vector3f> torsoOffsetLeft_;
  /// tors ooffset for right kick
  const Parameter<Vector3f> torsoOffsetRight_;



  struct KickProperties{
      float distance;
      float angle;
      typedef enum {
          CENTER,
          SIDE,
      } KICK_DIRECTION;
      typedef enum {
          SHORT,
          MEDIUM,
          LONG,
          HAMMER
      } KICK_DISTANCE;
      KICK_DISTANCE kickDistance;
      KICK_DIRECTION kickDirection;
      static KickProperties getFromSourceAndDestination(Vector2f source,Vector2f destination){
          KickProperties properties = *new KickProperties();
            //initial calculations
          float xdist=destination.x() - source.x();
          float ydist=destination.y() - source.y();
          float dist = std::sqrt(xdist*xdist+ydist*ydist);
          float ang = std::atan2(ydist,xdist); //left positive - right negative

          std::cout<<"real Distance is: "<<dist<<std::endl;

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
              properties.kickDirection = KickProperties::SIDE;
          }
          else {
              properties.kickDirection = KickProperties::CENTER;
          }
          if(properties.distance <coneMeasurements_().shortDistanceBoundary){
              properties.kickDistance = KickProperties::SHORT;
          }
          else if(properties.distance<coneMeasurements_().mediumDistanceBoundary){
              properties.kickDistance = KickProperties::MEDIUM;
          }
          else if ( properties.distance <coneMeasurements_().hammerDistanceBoundary) {
              properties.kickDistance = KickProperties::LONG;
          }
          else {
              properties.kickDistance = KickProperties::HAMMER;
          }

          return properties;
      }
  };

  /// The KickParameters struct contains all information for a kick. A kick is divided into nine
  /// phases. Each phase interpolates from one set of joint angles to another in a given duration.
  struct KickParameters : public Uni::To, public Uni::From
  {
    unsigned int waitBeforeStartDuration;
    unsigned int weightShiftDuration;
    unsigned int liftFootDuration;
    unsigned int swingFootDuration;
    unsigned int kickBallDuration;
    unsigned int pauseDuration;
    unsigned int retractFootDuration;
    unsigned int extendFootAndCenterTorsoDuration;
    unsigned int waitBeforeExitDuration;
    /// position of CoM after weight shift
    Vector3f weightShiftCom;
    /// position of kick foot after lifting it
    Vector3f liftFootPosition;
    /// position of kick foot after swinging it
    Vector3f swingFootPosition;
    /// position of kick foot after kicking the ball
    Vector3f kickBallPosition;
    /// position of kick foot after retracting it it
    Vector3f retractFootPosition;
    /// yawLeft2Right is the only joint angle that affects the yaw between the feet
    float yawLeft2right;
    /// shoulder roll prevents collision of arms with body
    float shoulderRoll;
    /// shoulderPitchAdjustement is added to shoulder pitch for momentum compnsation
    float shoulderPitchAdjustment;
    float ankleRoll;
    float anklePitch;

    virtual void toValue(Uni::Value& value) const
    {
      value = Uni::Value(Uni::ValueType::OBJECT);
      value["waitBeforeExitDuration"] << waitBeforeStartDuration;
      value["weightShiftDuration"] << weightShiftDuration;
      value["liftFootDuration"] << liftFootDuration;
      value["swingFootDuration"] << swingFootDuration;
      value["kickBallDuration"] << kickBallDuration;
      value["pauseDuration"] << pauseDuration;
      value["retractFootDuration"] << retractFootDuration;
      value["extendFootAndCenterTorsoDuration"] << extendFootAndCenterTorsoDuration;
      value["waitBeforeExitDuration"] << waitBeforeExitDuration;
      value["weightShiftCom"] << weightShiftCom;
      value["liftFootPosition"] << liftFootPosition;
      value["swingFootPosition"] << swingFootPosition;
      value["kickBallPosition"] << kickBallPosition;
      value["retractFootPosition"] << retractFootPosition;
      value["yawLeft2right"] << yawLeft2right;
      value["shoulderRoll"] << shoulderRoll;
      value["shoulderPitchAdjustment"] << shoulderPitchAdjustment;
      value["ankleRoll"] << ankleRoll;
      value["anklePitch"] << anklePitch;
    }

    virtual void fromValue(const Uni::Value& value)
    {
      value["waitBeforeStartDuration"] >> waitBeforeStartDuration;
      value["weightShiftDuration"] >> weightShiftDuration;
      value["liftFootDuration"] >> liftFootDuration;
      value["swingFootDuration"] >> swingFootDuration;
      value["kickBallDuration"] >> kickBallDuration;
      value["pauseDuration"] >> pauseDuration;
      value["retractFootDuration"] >> retractFootDuration;
      value["extendFootAndCenterTorsoDuration"] >> extendFootAndCenterTorsoDuration;
      value["waitBeforeExitDuration"] >> waitBeforeExitDuration;
      value["weightShiftCom"] >> weightShiftCom;
      value["liftFootPosition"] >> liftFootPosition;
      value["swingFootPosition"] >> swingFootPosition;
      value["kickBallPosition"] >> kickBallPosition;
      value["retractFootPosition"] >> retractFootPosition;
      value["yawLeft2right"] >> yawLeft2right;
      value["shoulderRoll"] >> shoulderRoll;
      value["shoulderPitchAdjustment"] >> shoulderPitchAdjustment;
      value["ankleRoll"] >> ankleRoll;
      value["anklePitch"] >> anklePitch;
    }
  };

  Parameter<KickParameters> forwardKickParameters_;
  Parameter<KickParameters> sideKickParameters_;

    struct ConeMeasurements : public Uni::To, public Uni::From
    {
        float minimalRadius;
        float shortDistanceBoundary;
        float mediumDistanceBoundary;
        float maximumRadius;
        float hammerDistanceBoundary;
        float maximumAngle;
        float sideDirectionBoundary;
        float minimalAngle;


        float shortKickDistance;
        float mediumKickDistance;
        float longKickDistance;
        float hammerKickDistance;

        float centerKickAngle;
        float sideKickAngle;

        virtual void toValue(Uni::Value& value) const
        {
            value = Uni::Value(Uni::ValueType::OBJECT);
            value["minimalRadius"] << minimalRadius;
            value["shortDistanceBoundary"] << shortDistanceBoundary;
            value["mediumDistanceBoundary"] << mediumDistanceBoundary;
            value["maximumRadius"] << maximumRadius;
            value["hammerDistanceBoundary"] << hammerDistanceBoundary;
            value["maximumAngle"] << maximumAngle;
            value["sideDirectionBoundary"] << sideDirectionBoundary;
            value["minimalAngle"] << minimalAngle;
            value["shortKickDistance"] << shortKickDistance;
            value["mediumKickDistance"] << mediumKickDistance;
            value["longKickDistance"] << longKickDistance;
            value["hammerKickDistance"] << hammerKickDistance;
            value["centerKickAngle"] << centerKickAngle;
            value["sideKickAngle"] << sideKickAngle;

        }

        virtual void fromValue(const Uni::Value& value)
        {
            value["minimalRadius"] >> minimalRadius;
            value["shortDistanceBoundary"] >> shortDistanceBoundary;
            value["mediumDistanceBoundary"] >> mediumDistanceBoundary;
            value["maximumRadius"] >> maximumRadius;
            value["hammerDistanceBoundary"] >>  hammerDistanceBoundary;
            value["maximumAngle"] >> maximumAngle;
            value["sideDirectionBoundary"] >> sideDirectionBoundary;
            value["minimalAngle"] >> minimalAngle;
            value["shortKickDistance"] >> shortKickDistance;
            value["mediumKickDistance"] >> mediumKickDistance;
            value["longKickDistance"] >> longKickDistance;
            value["hammerKickDistance"] >> hammerKickDistance;
            value["centerKickAngle"] >> centerKickAngle;
            value["sideKickAngle"] >> sideKickAngle;
        }
    };
    static Parameter<ConeMeasurements> coneMeasurements_;


    /// interpolators for all kick phases
  Interpolator waitBeforeStartInterpolator_;
  Interpolator weightShiftInterpolator_;
  Interpolator liftFootInterpolator_;
  Interpolator swingFootInterpolator_;
  Interpolator kickBallInterpolator_;
  Interpolator pauseInterpolator_;
  Interpolator retractFootInterpolator_;
  Interpolator extendFootAndCenterTorsoInterpolator_;
  Interpolator waitBeforeExitInterpolator_;

  /// an array containing all inteprolators
  const std::array<Interpolator*, 9> interpolators_ = {
      {&waitBeforeStartInterpolator_, &weightShiftInterpolator_, &liftFootInterpolator_,
       &swingFootInterpolator_, &kickBallInterpolator_, &pauseInterpolator_,
       &retractFootInterpolator_, &extendFootAndCenterTorsoInterpolator_,
       &waitBeforeExitInterpolator_}};

  /// the current interpolator id
  unsigned int currentInterpolatorID_;

  /// gyroscope filter coefficient and feedback gains
  const Parameter<float> gyroLowPassRatio_;
  const Parameter<float> gyroForwardBalanceFactor_;
  const Parameter<float> gyroSidewaysBalanceFactor_;

  // filtered gyroscope values
  Vector2f filteredGyro_;

  /**
   * @brief resetInterpolators resets all interpolators
   * @param kickParameters the parameters that determine the kick
   * @param torsoOffset the torso offset used for the kick
   */
  void resetInterpolators(const KickParameters &kickParameters, const Vector3f &torsoOffset, KickProperties d);

  /**
   * @brief computeWeightShiftAnglesFromReferenceCom computes angles from a reference CoM
   * @param currentAngles the current angles
   * @param weightShiftCom the desired CoM
   * @weightShiftAngles output parameter containing joint angles
   */
  void computeWeightShiftAnglesFromReferenceCom(const std::vector<float>& currentAngles,
                                                const Vector3f& weightShiftCom,
                                                std::vector<float>& weightShiftAngles) const;

  /**
   * @brief computeLegAnglesFromFootPose computes leg angles from foot pose
   * @param currentAngles the current angles
   * @param nextLeft2right the desired foot pose
   * @param nextAngles output parameter containing joint angles
   */
  void computeLegAnglesFromFootPose(const std::vector<float>& currentAngles,
                                    const KinematicMatrix& nextLeft2right,
                                    std::vector<float>& nextAngles) const;

  /**
   * @brief separateAngles separates left and right leg angles
   * @param left output parameter containing the left left angles
   * @param right output parameter containing the right leg angles
   * @param body angles of the whole body
   */
  void separateAngles(std::vector<float>& left, std::vector<float>& right,
                      const std::vector<float>& body) const;

  /**
   * @brief combineAngles combines left and right leg angles
   * @param result output parameter containing whole body angles
   * @param body angles of the whole body
   * @param left the left leg angles
   * @param right the right leg angles
   */
  void combineAngles(std::vector<float>& result, const std::vector<float>& body,
                     const std::vector<float>& left, const std::vector<float>& right) const;

  /**
   * @brief gyroFeedback applies gyroscope feedback to ankle roll and pitch
   * @param outputAngles output parameter containing whole body angles
   */
  void gyroFeedback(std::vector<float>& outputAngles) const;
};
