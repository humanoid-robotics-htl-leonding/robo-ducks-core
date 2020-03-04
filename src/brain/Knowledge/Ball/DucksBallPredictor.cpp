//
// Created by obyoxar on 18/02/2020.
//

#include "DucksBallPredictor.hpp"
DucksBallPredictor::DucksBallPredictor(const ModuleManagerInterface &manager)
	: Module(manager)
	, teamBallModel_(*this)
	, fieldDimensions_(*this)
	, desperation_(*this)

	, prediction_(*this)

	, ballPositionSmoothing_(50)

	, lastCycle_(std::chrono::steady_clock::now())
	, thisCycle_(std::chrono::steady_clock::now())
	, thisCycleModel_({Vector2f(), Vector2f(), false})
	, lastCycleModel_({Vector2f(), Vector2f(), false})
{

}
void DucksBallPredictor::cycle()
{
	thisCycle_ = std::chrono::steady_clock::now();
	auto dT = std::chrono::duration_cast<std::chrono::duration<double, std::chrono::seconds::period>>(thisCycle_ - lastCycle_);


	Vector2f predictedBallPos(0, 0);

	if(teamBallModel_->found) {
		ballPositionSmoothing_.push_front(teamBallModel_->position);
		for (const Vector2f &i : ballPositionSmoothing_) {
			predictedBallPos += i / 50;
		}
	}else{
		predictedBallPos = lastCycleModel_.position_ + lastCycleModel_.velocity_ * dT.count();
	}


	thisCycleModel_ = {
		predictedBallPos,
		Vector2f(),
		false
	};

	// Calculate Velocity
	if(lastCycleModel_.valid_){
		thisCycleModel_.velocity_ = (predictedBallPos - lastCycleModel_.position_)/dT.count();
	}

	this->forecast();

	lastCycleModel_ = thisCycleModel_;
	lastCycle_ = thisCycle_;
	lastCycleModel_.valid_ = true;
}

void DucksBallPredictor::forecast() {

	const int timeDifference = 10; //ms

	std::chrono::steady_clock::time_point cursor = thisCycle_;

	float certainty = 1-(desperation_->lookAtBallUrgency);
	certainty *= certainty;

	prediction_->forecast[0].position = thisCycleModel_.position_;
	prediction_->forecast[0].velocity = thisCycleModel_.velocity_;
	prediction_->forecast[0].time = TimePoint::from_std_time_point(thisCycle_);

	if(!fieldDimensions_->isInsideField(prediction_->forecast[0].position, 0.001)){
		prediction_->forecast[0].certainty = 0;
	}else{
		prediction_->forecast[0].certainty = certainty;
	}

	for(size_t i = 1; i < prediction_->forecast.size(); i++){
		cursor += std::chrono::milliseconds(timeDifference);
		certainty *= 0.5;

		prediction_->forecast[i].position = prediction_->forecast[i-1].position + prediction_->forecast[i-1].velocity * ((float)timeDifference)/1000.;
		prediction_->forecast[i].velocity = prediction_->forecast[i-1].velocity;
		prediction_->forecast[i].time = TimePoint::from_std_time_point(cursor);

		if(!fieldDimensions_->isInsideField(prediction_->forecast[i].position, 0.001)){
			prediction_->forecast[i].certainty = 0;
		}else{
			prediction_->forecast[i].certainty = certainty;
		}

	}
}