#pragma once
#include <Data/TeamBallModel.hpp>
#include <Data/CycleInfo.hpp>
#include "Tools/Math/Eigen.hpp"

#include "Data/Desperation.hpp"
#include "Framework/Module.hpp"

class Brain;

class DucksDesperationProvider: public Module<DucksDesperationProvider, Brain>
{
public:
	ModuleName name = "DucksDesperationProvider";
	DucksDesperationProvider(const ModuleManagerInterface &manager);
	void cycle();

private:
	const Dependency<TeamBallModel> teamBallModel_;
	const Dependency<CycleInfo> cycleInfo_;

	Production<Desperation> desperation_;

	float timeBallNotFound_ = 0;
};
