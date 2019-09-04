//
// Created by obyoxar on 04/09/2019.
//

#include "HeadOffModule.hpp"

HeadOffModule::HeadOffModule(const ModuleManagerInterface &manager)
: Module(manager),
buttonData_(*this)
{
}

void HeadOffModule::cycle() {

  float headMiddle = buttonData_->buttons[keys::sensor::SWITCH_HEAD_MIDDLE];
  float headFront = buttonData_->buttons[keys::sensor::SWITCH_HEAD_FRONT];
  float headBack = buttonData_->buttons[keys::sensor::SWITCH_HEAD_REAR];

  assert(!(headBack >= 1.0 && headFront >= 1.0 && headMiddle >= 1.0));
}
