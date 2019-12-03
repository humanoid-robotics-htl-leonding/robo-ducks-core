#include "GoalDetection.hpp"

#include "Tools/Chronometer.hpp"
#include "Tools/Math/Geometry.hpp"
#include "Tools/Math/Random.hpp"

#include "print.h"

GoalDetection::GoalDetection(const ModuleManagerInterface& manager)
  : Module(manager)
	, minNumberOfPointsOnGoal_(*this, "minNumberOfPointsOnGoal", [] {})
	, maxNumberOfPointsOnGoal_(*this, "maxNumberOfPointsOnGoal", [] {})
	, maxDistanceOfGroupPoints_(*this, "maxDistanceOfGroupPoints", [] {})
  , imageData_(*this)
  , cameraMatrix_(*this)
  , imageSegments_(*this)
  , goalData_(*this)
{
}

void GoalDetection::detectGoalPoints()
{
		goalPoints_.clear();
		Vector2f g1, g2;
		for (const auto& scanLine : imageSegments_->verticalScanlines)
		{
				const auto& segment = &scanLine.segments[0];
				if (segment->startEdgeType != EdgeType::BORDER || segment->endEdgeType != EdgeType::FALLING)
				{
						continue;
				}
				goalPoints_.push_back(segment->end);
		}
}

void GoalDetection::bombermanMaxDistanceGrouping()
{
		std::cerr << "A" << std::endl;
		goalPostGroups_.clear();
		auto it = goalPoints_.begin();
		std::cerr << "B" << std::endl;
		for (; it != goalPoints_.end(); it++) {
				std::cerr << "C" << std::endl;
				bombermanExplodeRecursive(it);
				std::cerr << "D" << std::endl;
				if (goalPostGroup_.size() >= minNumberOfPointsOnGoal_() && goalPostGroup_.size() <= maxNumberOfPointsOnGoal_()) {
						goalPostGroups_.emplace_back(goalPostGroup_);
				}
		}
		std::cerr << goalPostGroups_.size() << std::endl;
}

void GoalDetection::bombermanExplodeRecursive(VecVector2i::iterator column) {
		std::cerr << "E" << std::endl;
		goalPostGroup_.emplace_back(*column);
		std::cerr << "F" << std::endl;
		if (column == goalPoints_.end()) {
				std::cerr << "G" << std::endl;
				return;
		}
		std::cerr << "H" << std::endl;
		auto it = std::next(column);
		for (; it != goalPoints_.end(); it++) {
				std::cerr << "I" << std::endl;
				float distance = ((*column) - (*it)).norm();
				std::cerr << "J" << std::endl;
				if (distance > maxDistanceOfGroupPoints_())
				{
						std::cerr << "K" << std::endl;
						bombermanExplodeRecursive(it);
						std::cerr << "L" << std::endl;
						goalPoints_.erase(it);
						std::cerr << "M" << std::endl;
				}
				std::cerr << "N" << std::endl;
		}
}

void GoalDetection::cycle()
{
		if (!imageSegments_->valid)
		{
				return;
		}
		{
				Chronometer time(debug(), mount_ + "." + imageData_->identification + "_cycle_time");
				detectGoalPoints();
				debugGoalPoints_ = goalPoints_;
				//bombermanMaxDistanceGrouping();
				//createGoalData();
		}
		sendImagesForDebug();
		goalData_->valid = false;
}

void GoalDetection::sendImagesForDebug()
{
		auto mount = mount_ + "." + imageData_->identification + "_image_goals";
		if (debug().isSubscribed(mount))
		{
				Image image(imageData_->image422.to444Image());
				for (const auto& point : debugGoalPoints_)
				{
						image.circle(Image422::get444From422Vector(point), 2, Color::RED);
				}
				debug().sendImage(mount_ + "." + imageData_->identification + "_image_goals", image);
		}
}
