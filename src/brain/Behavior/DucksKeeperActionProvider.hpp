#pragma once

#include <Data/PlayerConfiguration.hpp>
#include "Data/BallState.hpp"
#include "Data/CycleInfo.hpp"
#include "Data/FieldDimensions.hpp"
#include "Data/GameControllerState.hpp"
#include "Data/KeeperAction.hpp"
#include "Data/RobotPosition.hpp"
#include "Data/TeamBallModel.hpp"
#include "Data/TeamPlayers.hpp"
#include "Data/WorldState.hpp"
#include "Tools/Math/Line.hpp"

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
    const Parameter<float> shadowCastSpeed_;
    const Parameter<float> shadowResolveSpeed_;
    const Parameter<float> robotDiameter_;
    const Parameter<float> keeperMaxX_;
    const Parameter<float> keeperMinX_;
    const Parameter<int> segmentCount_;
    const Parameter<float> keeperBallKickDistance_;

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
    /// a reference to the world state
    const Dependency<PlayerConfiguration> playerConfiguration_;

    /// a reference to the striker action
    Production<KeeperAction> keeperAction_;

    /// default keeper position (0.5*penaltyAreaLength in front of the goal line)
    Vector2f keeperPosition_;

    struct ProposedPosition : public Uni::To{

        ProposedPosition(Vector2f position, float score): position(position), score(score) {}

        Vector2f position;
        float score;

        void toValue(Uni::Value &value) const override {
            value = Uni::Value(Uni::ValueType::OBJECT);
            value["position"] << position;
            value["score"] << score;
        }
    };

    std::vector<ProposedPosition> proposedPositions_;
    std::vector<float> goalShadow_;

    void calculateBestKeeperPositionFor(const Vector2f& segmentLowerPoint, const Vector2f& segmentMiddlePoint);

    bool robotIntersectsRayToSegment(const Vector2f& segment);

    bool aimingForMyGoal(float orientation);

    bool ballInKickRange();
};
