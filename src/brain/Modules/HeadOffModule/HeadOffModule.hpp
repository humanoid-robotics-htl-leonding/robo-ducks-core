//
// Created by obyoxar on 04/09/2019.
//

#ifndef TUHHNAO_HEADOFFMODULE_H
#define TUHHNAO_HEADOFFMODULE_H


#include <Brain.hpp>
#include <Data/ButtonData.hpp>

class HeadOffModule : public Module<HeadOffModule, Brain> {
public:
    ModuleName name = "HeadOffModule";
    HeadOffModule(const ModuleManagerInterface& manager);
    void cycle();

private:
    const Dependency<ButtonData> buttonData_;
};


#endif //TUHHNAO_HEADOFFMODULE_H
