#pragma once

ActionCommand roleStriker(const DataSet& d){
//    d.robotPosition.robotToField()
    ActionCommand cmd = ActionCommand::walk(d.robotPosition.robotToField(Pose(0, 0, 45*TO_RAD)));
    return cmd;
}