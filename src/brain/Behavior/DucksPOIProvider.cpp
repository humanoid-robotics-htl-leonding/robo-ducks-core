//
// Created by obyoxar on 03/03/2020.
//

#include "DucksPOIProvider.hpp"

DucksPOIProvider::DucksPOIProvider(const ModuleManagerInterface &manager)
	: Module(manager)
	, interestingPOI_(*this)
	, pois_()
{

}

void DucksPOIProvider::cycle()
{
	gatherPOIs();
	votePOIs();
}

void DucksPOIProvider::gatherPOIs()
{
	pois_.emplace_back();
}
void DucksPOIProvider::votePOIs()
{
	updateMostInterestingPOI(pois_[0]);
}

void DucksPOIProvider::updateMostInterestingPOI(const DucksPOI &newMostInterestingPOI)
{
	interestingPOI_->position = newMostInterestingPOI.position;
	interestingPOI_->score = newMostInterestingPOI.score;
	interestingPOI_->type = newMostInterestingPOI.type;
}


