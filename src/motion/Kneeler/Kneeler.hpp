//
// Created by max on 12/3/19.
//

#include <Data/MotionActivation.hpp>
#include <Data/BodyPose.hpp>
#include <Data/KneelerOutput.hpp>
#include <Data/CycleInfo.hpp>
#include <Utils/Interpolator/Interpolator.hpp>
#include <Data/JointSensorData.hpp>
#include "Framework/Module.hpp"


class Motion;

class Kneeler: public Module<Kneeler,Motion> {
public:
    /// the name of this module
    ModuleName name = "Kneeler";
    /**
     * @brief FallManager initializes members and loads motion files
     * @param manager a reference to motion
     */
    Kneeler(const ModuleManagerInterface& manager);
    /**
     * @brief cycle checks if the robot is falling and initializes a motion to prevent it
     */
    void cycle();

private:
    const Dependency<MotionActivation> motionActivation_;
    const Dependency<MotionRequest> motionRequest_;
    const Dependency<BodyPose> bodyPose_;
    const Dependency<CycleInfo> cycleInfo_;
    const Dependency<JointSensorData> jointSensorData_;
    Production<KneelerOutput> kneelerOutput_;
    Interpolator kneelInterpolator;

    float kneelDownDuration;

    void prepareKneeling();

    std::vector<float> lastAngles_;
};


