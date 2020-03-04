#pragma once

#include <Brain.hpp>
#include <Framework/Module.hpp>

#include <Data/DucksPOI.hpp>
#include <Data/TeamBallModel.hpp>
#include <Data/RobotPosition.hpp>
#include <Data/Desperation.hpp>
#include <Data/BallSearchMap.hpp>
/***
 * @author Erik Mayrhofer
 */
class DucksPOIProvider: public Module<DucksPOIProvider, Brain>
{
public:
	ModuleName name = "DucksPOIProvider";

	explicit DucksPOIProvider(const ModuleManagerInterface& manager);

	void cycle() override;
private:
	void gatherPOIs();
	void votePOIs();
	void updateMostInterestingPOI(const DucksPOI& newMostInterestingPOI);
	void proposePosition(const Vector2f& position, DucksPOI::Type type, float proposedScore);

	const Dependency<TeamBallModel> teamBallModel_;
	const Dependency<Desperation> desperation_;
	const Dependency<RobotPosition> robotPosition_;
	const Dependency<BallSearchMap> ballSearchMap_;

	Production<DucksPOI> interestingPOI_;

	std::vector<DucksPOI> pois_;
};
