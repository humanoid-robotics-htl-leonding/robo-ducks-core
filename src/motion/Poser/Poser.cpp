#include "Modules/Poses.h"

#include "Poser.hpp"

Poser::Poser(const ModuleManagerInterface& manager)
  : Module(manager)
  , motionActivation_(*this)
  , poserOutput_(*this)
{
}

void Poser::cycle()
{
    if(motionActivation_->activeMotion ==MotionRequest::BodyMotion::PENALIZED){
        poserOutput_->angles = Poses::getPose(Poses::PENALIZED);
        poserOutput_->stiffnesses = std::vector<float>(
                poserOutput_->angles.size(), motionActivation_->activations[static_cast<unsigned int>(MotionRequest::BodyMotion::PENALIZED)] < 0.9f ? 0.7f : 0.2f);
    }
    if(motionActivation_->activeMotion ==MotionRequest::BodyMotion::KNEEL){
        poserOutput_->angles = Poses::getPose(Poses::KNEEL);
        poserOutput_->stiffnesses = std::vector<float>(
                poserOutput_->angles.size(), 0.7f);
    }

}
