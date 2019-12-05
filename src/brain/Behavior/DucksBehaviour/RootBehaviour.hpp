//
// Created by obyoxar on 26/11/2019.
//
#pragma once

ActionCommand rootBehavior(const DataSet& d) {
  ActionCommand state = chooseState(d);
  state.combineLeftFootLED(ActionCommand::FootLED::green());
  state.combineRightFootLED(ActionCommand::FootLED::blue());
//  ActionCommand state = ActionCommand::dead().combineChestLED(ActionCommand::ChestLED::blue());
  return state;
}