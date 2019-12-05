//
// Created by obyoxar on 26/11/2019.
//
#pragma once

ActionCommand rootBehavior(const DataSet& d) {
//  ActionCommand state = chooseState(d);
  ActionCommand state = ActionCommand::dead().combineChestLED(ActionCommand::ChestLED::blue());
  return state;
}