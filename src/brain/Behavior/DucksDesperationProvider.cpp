#include "Tools/Chronometer.hpp"
#include "DucksDesperationProvider.hpp"


DucksDesperationProvider::DucksDesperationProvider(const ModuleManagerInterface &manager)
	: Module(manager)
	, teamBallModel_(*this)
	, cycleInfo_(*this)
	, desperation_(*this)
	, timeBallNotFound_(0)
{
}
void DucksDesperationProvider::cycle()
{
	Chronometer time(debug(), mount_ + ".cycleTime");

	if(!teamBallModel_->found){
		timeBallNotFound_ += cycleInfo_->cycleTime;
	}else{
		timeBallNotFound_ -= cycleInfo_->cycleTime * 10.f;
	}

	timeBallNotFound_ = std::max(0.f, std::min(timeBallNotFound_, 30.f));


	desperation_->lookAtBallUrgency = timeBallNotFound_ / 30.f;
}