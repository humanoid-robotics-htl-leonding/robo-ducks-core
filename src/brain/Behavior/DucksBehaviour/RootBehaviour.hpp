//
// Created by obyoxar on 26/11/2019.
//
#pragma once

ActionCommand rootBehavior(const DataSet& d) {
  ActionCommand state = chooseState(d);
  if(d.cycleInfo.getTimeDiff(d.thoughts.stateSince, TDT::MILS) < 0.2){
    state.combineAudio(ActionCommand::Audio::audioC3());
  }
  return state;
}