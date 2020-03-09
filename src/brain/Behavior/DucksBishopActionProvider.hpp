//
// Created by max on 3/9/20.
//

#pragma once
#include <Data/FieldZones.hpp>
#include <Data/RobotPosition.hpp>
#include <Data/TeamBallModel.hpp>
#include <Data/Desperation.hpp>
#include "Framework/Module.hpp"
#include "Data/DucksBishopAction.hpp"

class Brain;

class DucksBishopActionProvider: public Module<DucksBishopActionProvider, Brain>
{
public:
    /// the name of this module
    ModuleName name = "DucksDefenderActionProvider";
    /**
     * @brief DucksDefenderActionProvider initializes members
     * @param manager a reference to brain
     */
    DucksBishopActionProvider(const ModuleManagerInterface &manager);
    /**
     * @brief cycle calculates the defending position
     */
    void cycle();

private:
    const Dependency<FieldZones> fieldZones_;
    const Dependency<GameControllerState> gameControllerState_;
    const Dependency<RobotPosition> robotPosition_;
    const Dependency<TeamBallModel> teamBallModel_;
    const Dependency<Desperation> desperation_;
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
};