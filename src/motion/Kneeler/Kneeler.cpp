//
// Created by max on 12/3/19.
//

#include <Modules/NaoProvider.h>
#include <Modules/Poses.h>
#include "Kneeler.hpp"



Kneeler::Kneeler(const ModuleManagerInterface &manager) : Module(manager)
    ,motionActivation_(*this)
    ,motionRequest_(*this)
    ,bodyPose_(*this)
    ,cycleInfo_(*this)
    ,jointSensorData_(*this)
    ,motionState_(*this)
    ,kneelerOutput_(*this){
    kneelDownDuration = 200.0f;
    lastAngles_ = Poses::getPose(Poses::READY);
}


void Kneeler::cycle() {
    if (motionRequest_->bodyMotion == MotionRequest::BodyMotion::KNEEL  && motionState_->bodyMotion != MotionRequest::BodyMotion::KNEEL)
    {
        prepareKneeling();
    }
    if (!kneelInterpolator.finished())
    {
        kneelerOutput_->wantToSend = true;
        kneelerOutput_->safeExit = false;
        kneelerOutput_->angles = kneelInterpolator.step(10);
        kneelerOutput_->stiffnesses = std::vector<float>(JOINTS::JOINTS_MAX, 0.7);

    }
    else
    {
        kneelerOutput_->angles = lastAngles_;
        kneelerOutput_->stiffnesses = std::vector<float>(JOINTS::JOINTS_MAX, 0.7f);
        kneelerOutput_->wantToSend = false;
        kneelerOutput_->safeExit = true;
    }
    lastAngles_ = kneelerOutput_->angles;
}

void Kneeler::prepareKneeling() {
    std::vector<float> kneelingAngles = Poses::getPose(Poses::READY);

    //head
    kneelingAngles[JOINTS::HEAD_YAW]= 0.0f;
    kneelingAngles[JOINTS::HEAD_PITCH]= 28.0f * TO_RAD;
    //leftarm
    kneelingAngles[JOINTS::L_SHOULDER_PITCH]= 90.0f*TO_RAD;
    kneelingAngles[JOINTS::L_SHOULDER_ROLL]= 0.0f*TO_RAD;
    kneelingAngles[JOINTS::L_ELBOW_ROLL]= -2.0*TO_RAD;
    kneelingAngles[JOINTS::L_ELBOW_YAW]= 0.0f;
    kneelingAngles[JOINTS::L_WRIST_YAW]= 0.0f;

    //rightarm
    kneelingAngles[JOINTS::R_SHOULDER_PITCH]= 90.0f *TO_RAD;
    kneelingAngles[JOINTS::R_SHOULDER_ROLL]= 0.0f * TO_RAD;
    kneelingAngles[JOINTS::R_ELBOW_ROLL]= 2.0f*TO_RAD;
    kneelingAngles[JOINTS::R_ELBOW_YAW]= 0.0f;
    kneelingAngles[JOINTS::R_WRIST_YAW]= 0.0f;

    //hips
    kneelingAngles[JOINTS::L_HIP_YAW_PITCH]= 0.0f;
    kneelingAngles[JOINTS::R_HIP_YAW_PITCH]= 0.0f;

    //leftfoot
    kneelingAngles[JOINTS::L_HIP_PITCH]= -50.0f*TO_RAD;
    kneelingAngles[JOINTS::L_HIP_ROLL]= 0.0f;
    kneelingAngles[JOINTS::L_KNEE_PITCH]= 120.0f*TO_RAD;
    kneelingAngles[JOINTS::L_ANKLE_PITCH]= -65.0f*TO_RAD;
    kneelingAngles[JOINTS::L_ANKLE_ROLL]= 0.0f;

    //rightfoot
    kneelingAngles[JOINTS::R_HIP_PITCH]=-50.0f*TO_RAD;
    kneelingAngles[JOINTS::R_HIP_ROLL]= 0.0f;
    kneelingAngles[JOINTS::R_KNEE_PITCH]= 120.0f*TO_RAD;
    kneelingAngles[JOINTS::R_ANKLE_PITCH]= -65.0f*TO_RAD;
    kneelingAngles[JOINTS::R_ANKLE_ROLL]= 0.0f;

    kneelInterpolator.reset(jointSensorData_->getBodyAngles(), kneelingAngles,
                                  kneelDownDuration);
}
