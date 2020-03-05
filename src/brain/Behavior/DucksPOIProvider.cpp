//
// Created by obyoxar on 03/03/2020.
//

#include <Knowledge/Position/FieldInfo.hpp>
#include "DucksPOIProvider.hpp"


DucksPOIProvider::DucksPOIProvider(const ModuleManagerInterface &manager)
	: Module(manager)
	, teamBallModel_(*this)
	, desperation_(*this)
	, robotPosition_(*this)
	, fieldDimensions_(*this)
	, ballSearchMap_(*this)

	, interestingPOI_(*this)

	, oldPOIPosition_()
	, startedLookingAtPOI_()

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
	FieldInfo info(*fieldDimensions_);
	pois_.clear();
	// == Integrate TeamBallModel ==
	if(teamBallModel_->found){
		proposePosition(teamBallModel_->position, DucksPOI::Type::BALL_CANDIDATE, POI_KNOWN_INTEREST(0.f));
	}

	// == Integrate BallMap
	auto list = std::list<ProbCell*>(ballSearchMap_->probabilityList_); //Copy
	list.sort(ProbCell::probability_comparator_desc);
	int minProbCells = 5;
	for(const auto& i : list){
		if(i->probability > 0.05 || minProbCells > 0){
			proposePosition(i->position, DucksPOI::Type::BALL_CANDIDATE, POI_GOOD_GUESS(i->probability*5.));
			minProbCells--;
		}
	}

	// ==
	//TODO FieldInfo Yeeten
	for(const auto& i : info.intersections){
		float importance = 0;
		if(i.type == LandmarkModel::Intersection::Type::T){
			importance = 10.0;
		}else if(i.type == LandmarkModel::Intersection::Type::X){
			importance = 20.0;
		}
		proposePosition(i.position, DucksPOI::Type::LANDMARK, POI_KNOWN_INTEREST(importance));
	}

	// ==




}
void DucksPOIProvider::votePOIs()
{

	std::sort(pois_.begin(), pois_.end(), [](const DucksPOI& a,const DucksPOI& b) -> bool {return a.score > b.score;});

	debug().update(mount_+".pois", pois_);
	debug().update(mount_+".cooldowns", cooldowns_);
	debug().update(mount_+".age", currentPOIDeath_.age());

	cooldowns_.erase(
	std::remove_if(cooldowns_.begin(), cooldowns_.end(), [](const POICooldown& a) -> bool {return a.doneAt.hasPassed();}),
	cooldowns_.end()
	);


	if(currentPOIDeath_.hasPassed()){
		for(const auto & p : pois_){
			if(p.type != currentPOI_.type){
				updateMostInterestingPOI(p);
				if(p.type == DucksPOI::Type::BALL_CANDIDATE){
					currentPOIDeath_ = TimePoint::getCurrentTime() + 2000.0;
				}else{
					currentPOIDeath_ = TimePoint::getCurrentTime() + 1000.0;
				}
				break;
			}
		}
	}else {
		auto meToCurrent = currentPOI_.position - robotPosition_->pose.position;
		float currentAngle = std::atan2(meToCurrent.y(), meToCurrent.x());

		for (const auto &p : pois_) {
			if (p.evaluation == DucksPOI::Evaluation::UNCOMFORATBLE) {
				continue;
			}

			auto meToNew = (p.position - robotPosition_->pose.position);
			float newAngle = std::atan2(meToNew.y(), meToNew.x());

			if (Angle::angleDiff(currentAngle, newAngle) < 10.0 * TO_RAD) {
				updateMostInterestingPOI(p);
				return;
			}
		}
		updateMostInterestingPOI(currentPOI_);
	}
}

void DucksPOIProvider::updateMostInterestingPOI(const DucksPOI &newMostInterestingPOI)
{
	currentPOI_ = newMostInterestingPOI;
	interestingPOI_->position = newMostInterestingPOI.position;
	interestingPOI_->score = newMostInterestingPOI.score;
	interestingPOI_->type = newMostInterestingPOI.type;

	if((oldPOIPosition_ - newMostInterestingPOI.position).norm() < 0.5){
//		oldPOIPosition_ = (2.0*oldPOIPosition_ + newMostInterestingPOI.position)/2;
	}else{
		oldPOIPosition_ = newMostInterestingPOI.position;
		startedLookingAtPOI_ = TimePoint::getCurrentTime();
	}

	debug().update(mount_+".oldPOIPosition", oldPOIPosition_);
	debug().update(mount_+".startedLookingAt", startedLookingAtPOI_);

}
void DucksPOIProvider::proposePosition(const Vector2f &position, DucksPOI::Type type, float proposedScore)
{
	auto val = DucksPOI::Evaluation::NONE;
	if(!robotPosition_->pose.frustrumContainsPoint(position, 30*TO_RAD)){
		proposedScore -= 301.0; //Move below prior class.
		val = DucksPOI::Evaluation::UNCOMFORATBLE;
	}
	pois_.emplace_back(position, type, proposedScore, val);
}
bool DucksPOIProvider::liesWithinCooldown(const Vector2f &position)
{
	for(const auto & cooldown : cooldowns_){
		if(!cooldown.doneAt.hasPassed() && (position - cooldown.poi.position).norm() < 0.3){
			return false;
		}
	}
	return true;
}

