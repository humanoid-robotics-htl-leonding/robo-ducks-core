//
// Created by obyoxar on 04/09/2019.
//

#include "HeadOffModule.hpp"
#include "Behavior/Units.hpp"

HeadOffModule::HeadOffModule(const ModuleManagerInterface &manager)
: Module(manager),
buttonData_(*this),
motionRequest_(*this)
{
}

void HeadOffModule::cycle() {

  float headMiddle = buttonData_->buttons[keys::sensor::SWITCH_HEAD_MIDDLE];
  float headFront = buttonData_->buttons[keys::sensor::SWITCH_HEAD_FRONT];
  float headBack = buttonData_->buttons[keys::sensor::SWITCH_HEAD_REAR];


  ActionCommand behaviour = ActionCommand::dead();
  behaviour.toMotionRequest(*motionRequest_);

  assert(!(headBack >= 1.0 && headFront >= 1.0 && headMiddle >= 1.0));
}
