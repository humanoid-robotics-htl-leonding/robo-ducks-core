#pragma once

ActionCommand roleStriker(const DataSet& d){
    ActionCommand cmd = ActionCommand::walk(Pose(0, 0, 45));
    return cmd;
}