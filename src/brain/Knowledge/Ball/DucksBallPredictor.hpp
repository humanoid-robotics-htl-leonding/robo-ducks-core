//
// Created by obyoxar on 18/02/2020.
//

#ifndef DUCKSBALLPREDICTOR_HPP
#define DUCKSBALLPREDICTOR_HPP

#include <boost/circular_buffer.hpp>
#include <Framework/Module.hpp>
#include <Brain.hpp>
#include <Data/DucksBallPrediction.hpp>
#include <Data/TeamBallModel.hpp>
#include <Data/FieldDimensions.hpp>

class DucksBallPredictor : public Module<DucksBallPredictor, Brain>
{
public:
	ModuleName name = "DucksBallPredictor";

	explicit DucksBallPredictor(const ModuleManagerInterface& manager);

	void cycle() override;

private:
	const Dependency<TeamBallModel> teamBallModel_;
	const Dependency<FieldDimensions> fieldDimensions_;

	Production<DucksBallPrediction> prediction_;

	void forecast();

	struct BallModel {
		Vector2f position_;
		Vector2f velocity_;

		bool valid_;
	};

	boost::circular_buffer<Vector2f> ballPositionSmoothing_;

	std::chrono::steady_clock::time_point lastCycle_;
	std::chrono::steady_clock::time_point thisCycle_;

	BallModel thisCycleModel_;
	BallModel lastCycleModel_;
};


#endif //DUCKSBALLPREDICTOR_HPP
