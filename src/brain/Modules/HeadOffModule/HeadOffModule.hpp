//
// Created by obyoxar on 04/09/2019.
//

#ifndef TUHHNAO_HEADOFFMODULE_H
#define TUHHNAO_HEADOFFMODULE_H


#include <Brain.hpp>
#include <Data/ButtonData.hpp>
#include <Data/MotionRequest.hpp>
#include <Data/HeadOffData.hpp>
#include <Data/CycleInfo.hpp>

class HeadOffModule : public Module<HeadOffModule, Brain> {
public:
    ModuleName name = "HeadOffModule";
    HeadOffModule(const ModuleManagerInterface& manager);
    void cycle();

private:
    const Dependency<ButtonData> buttonData_;
    const Dependency<CycleInfo> cycleInfo_;
    Production<HeadOffData> headOffData_;

    TimePoint pressStarted = 0;
};


#endif //TUHHNAO_HEADOFFMODULE_H
