#pragma once

#include "Data/BallState.hpp"
#include "Data/CycleInfo.hpp"
#include "Data/FieldDimensions.hpp"
#include "Data/GameControllerState.hpp"
#include "Data/KeeperAction.hpp"
#include "Data/RobotPosition.hpp"
#include "Data/TeamBallModel.hpp"
#include "Data/TeamPlayers.hpp"
#include "Data/WorldState.hpp"

#include "Framework/Module.hpp"

class Brain;

class DucksKeeperActionProvider : public Module<DucksKeeperActionProvider, Brain> {
public:
    /// the name of this module
    ModuleName name = "DucksKeeperActionProvider";

    /**
     * @brief DucksKeeperActionProvider initializes members
     * @param manager a reference to brain
     */
    explicit DucksKeeperActionProvider(const ModuleManagerInterface &manager);

    /**
     * @brief cycle updates the Keeper action
     */
    void cycle() override;

private:
    /// cycle info needed to check if team mate is aleady near the ball
    const Dependency<CycleInfo> cycleInfo_;
    /// a reference to the ball state
    const Dependency<BallState> ballState_;
    /// a reference to the field dimensions
    const Dependency<FieldDimensions> fieldDimensions_;
    /// a reference to the game controller state
    const Dependency<GameControllerState> gameControllerState_;
    /// a reference to the robot position
    const Dependency<RobotPosition> robotPosition_;
    /// a reference to the team ball state
    const Dependency<TeamBallModel> teamBallModel_;
    /// a reference to the team players
    const Dependency<TeamPlayers> teamPlayers_;
    /// a reference to the world state
    const Dependency<WorldState> worldState_;

    /// a reference to the striker action
    Production<KeeperAction> keeperAction_;

    /// default keeper position (0.5*penaltyAreaLength in front of the goal line)
    Vector2f keeperPosition_;
};
