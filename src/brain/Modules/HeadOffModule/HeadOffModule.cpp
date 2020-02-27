//
// Created by obyoxar on 04/09/2019.
//

#include <Modules/Log/Log.h>
#include "HeadOffModule.hpp"
#include "print.h"


HeadOffModule::HeadOffModule(const ModuleManagerInterface &manager)
	: Module(manager),
	  buttonData_(*this),
	  cycleInfo_(*this),
	  headOffData_(*this)
{
}

void HeadOffModule::cycle()
{
//	assert(!headOffModule_shutDown);
	if(headOffModule_shutDown){
		std::cout << "HeadoffModule is giving the Nao a stroke." << std::endl;
		std::cout.flush();
		exit(0);
	}

	if (lastCycle) {
		headOffModule_shutDown = true;
	}

	float headMiddle = buttonData_->buttons[keys::sensor::SWITCH_HEAD_MIDDLE];
	float headFront = buttonData_->buttons[keys::sensor::SWITCH_HEAD_FRONT];
	float headBack = buttonData_->buttons[keys::sensor::SWITCH_HEAD_REAR];

	bool dying = (headBack >= 1.0 && headFront >= 1.0 && headMiddle >= 1.0);

	if (dying) {
		if (pressStarted == 0) {
			pressStarted = cycleInfo_->startTime;
		}
		int delta = cycleInfo_->startTime - pressStarted;
		if (delta > shutdownTime) {
			headOffData_->lastCycle = true;
			lastCycle = true;
		}
		else if (delta > (shutdownTime-signalLength)) {
			headOffData_->shouldDieSignal = true;
		}
	}
	else {
		headOffData_->shouldDieSignal = false;
		pressStarted = 0;
	}

	headOffData_->shouldDie = dying;

}
