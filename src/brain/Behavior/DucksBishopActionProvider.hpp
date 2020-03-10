//
// Created by max on 3/9/20.
//

#pragma once
#include <Data/FieldZones.hpp>
#include <Data/RobotPosition.hpp>
#include <Data/TeamBallModel.hpp>
#include <Data/Desperation.hpp>
#include <Data/FieldDimensions.hpp>
#include <Data/ObstacleData.hpp>
#include "Framework/Module.hpp"
#include "Data/DucksBishopAction.hpp"

class Brain;

class DucksBishopActionProvider: public Module<DucksBishopActionProvider, Brain>
{
public:
    /// the name of this module
    ModuleName name = "DucksBishopActionProvider";
    /**
     * @brief DucksBishopActionProvider initializes members
     * @param manager a reference to brain
     */
    DucksBishopActionProvider(const ModuleManagerInterface &manager);
    /**
     * @brief cycle calculates the defending position
     */
    void cycle();

private:

    const Parameter<float> strikerIsOffensiveLine_;
    const Parameter<Vector2f> passOffset_;
    const Parameter<float> maxPassDistance_;

    const Dependency<FieldZones> fieldZones_;
    const Dependency<GameControllerState> gameControllerState_;
    const Dependency<RobotPosition> robotPosition_;
    const Dependency<TeamBallModel> teamBallModel_;
    const Dependency<Desperation> desperation_;
    const Dependency<TeamPlayers> teamPlayers_;
    const Dependency<FieldDimensions> fieldDimensions_;
    const Dependency<ObstacleData> obstacleData_;


    Production<DucksBishopAction> bishopAction_;

    bool shouldPatrol();
    void patrol();
    bool shouldEvadeLiberationStrike();
    void evadeLiberationStrike();
    bool shouldPass();
    void pass();
    bool shouldShadowBall();
    void shadowBall();
    bool shouldStrike();
    void strike();
    float getZoneCornerPatrolOrientation(Vector2f corner,Rectangle<float> zone);
    Pose patrolTarget;
    void findStriker(const TeamPlayer *&pPlayer);
    bool positionToPositionIsIntersected(const Vector2f &segment,Vector2f position);
    bool currentlyDoingPassingAction;
    Vector2f passBallSource;
    Vector2f passBallTarget;

};