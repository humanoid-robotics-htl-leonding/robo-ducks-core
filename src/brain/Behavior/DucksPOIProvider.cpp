//
// Created by obyoxar on 03/03/2020.
//

#include "DucksPOIProvider.hpp"


DucksPOIProvider::DucksPOIProvider(const ModuleManagerInterface &manager)
	: Module(manager)
	, teamBallModel_(*this)
	, desperation_(*this)
	, robotPosition_(*this)
	, ballSearchMap_(*this)

	, interestingPOI_(*this)
	, pois_(10)
{

}

void DucksPOIProvider::cycle()
{
	gatherPOIs();
	votePOIs();
}

void DucksPOIProvider::gatherPOIs()
{
	pois_.clear();
	// == Integrate TeamBallModel ==
	if(teamBallModel_->found){
		proposePosition(teamBallModel_->position, DucksPOI::Type::BALL_CANDIDATE, POI_KNOWN_INTEREST(0.f));
	}

	// == Integrate BallMap
//	auto list = std::list<ProbCell*>(ballSearchMap_->probabilityList_); //Copy
//	std::sort(list.begin(), list.end(), [](const ProbCell& a, const ProbCell& b) -> bool {return a.probability > b.probability;});
	for(const auto& i : ballSearchMap_->probabilityList_){
		if(i->probability > 0.05){
			proposePosition(i->position, DucksPOI::Type::BALL_CANDIDATE, POI_GOOD_GUESS(i->probability*5.));
		}
	}

	// == 


}
void DucksPOIProvider::votePOIs()
{
	std::sort(pois_.begin(), pois_.end(), [](const DucksPOI& a,const DucksPOI& b) -> bool {return a.score > b.score;});

	debug().update(mount_+".pois", pois_);

	updateMostInterestingPOI(pois_[0]);
}

void DucksPOIProvider::updateMostInterestingPOI(const DucksPOI &newMostInterestingPOI)
{
	interestingPOI_->position = newMostInterestingPOI.position;
	interestingPOI_->score = newMostInterestingPOI.score;
	interestingPOI_->type = newMostInterestingPOI.type;
}
void DucksPOIProvider::proposePosition(const Vector2f &position, DucksPOI::Type type, float proposedScore)
{
	auto val = DucksPOI::Evaluation::NONE;
	if(!robotPosition_->pose.frustrumContainsPoint(position, 40*TO_RAD)){
		proposedScore -= 101.0; //Move below prior class.
		val = DucksPOI::Evaluation::UNCOMFORATBLE;
	}
	pois_.emplace_back(position, type, proposedScore, val);
}


