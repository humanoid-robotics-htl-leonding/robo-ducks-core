#pragma once

// the Defender uses the DefenderAction
DucksActionCommand roleDefender(const DucksDataSet &d)
{
    auto defenderAction = d.defendingPosition;

    //Specifies which mode the Defender should adopt (such as Defend or Walk)
    // A dataset (d) and mode must be passed => movement
    // the different action types are returned
    if(defenderAction.valid){

        WalkMode mode;
        switch (defenderAction.type) {
            case DucksDefenderAction::Type::DEFEND:
                return DucksActionCommand::stand();
            case DucksDefenderAction::Type::WALK:
                mode = WalkMode::PATH;
                return walkTo(defenderAction.targetPose, d, mode);
            case DucksDefenderAction::Type::WALK_WITH_ORIENTATION:
                mode = WalkMode::PATH_WITH_ORIENTATION;
                return walkTo(defenderAction.targetPose, d, mode);
            case DucksDefenderAction::Type::WALKDIRECT:
                mode = WalkMode::DIRECT;
                return walkTo(defenderAction.targetPose, d, mode);
            case DucksDefenderAction::Type::WALKDIRECT_WITH_ORIENTATION:
                mode = WalkMode::DIRECT_WITH_ORIENTATION;
                return walkTo(defenderAction.targetPose, d, mode);
            case DucksDefenderAction::Type::KICK:
                return kick(d, defenderAction.targetPose.position);
            default:
                return DucksActionCommand::stand().invalidate();
        }

    }

    return DucksActionCommand::stand().invalidate();
}
