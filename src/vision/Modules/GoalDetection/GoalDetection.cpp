#include "GoalDetection.hpp"

#include "Tools/Chronometer.hpp"
#include "Tools/Math/Geometry.hpp"
#include "Tools/Math/Random.hpp"

#include "print.h"

GoalDetection::GoalDetection(const ModuleManagerInterface& manager)
  : Module(manager)
  	, maxDistanceOfNeighbors_(*this, "maxDistanceOfNeighbors", [] {})
  	, minPointsInGroup_(*this, "minPointsInGroup", [] {})
	, minSegmentLength_(*this, "minSegmentLength", [] {})
	, maxSegmentLength_(*this, "maxSegmentLength", [] {})
  , imageData_(*this)
  , cameraMatrix_(*this)
  , filteredSegments_(*this)
  , goalData_(*this)
{
}

void GoalDetection::detectGoalPoints()
{
	goalPoints_.clear();
	auto shift = [](int c) { return c >> 1; };
	for (const auto& segment : filteredSegments_->horizontal)
	{
		if (static_cast<unsigned int>(segment->scanPoints) <  minSegmentLength_ () ||
		static_cast<unsigned int>(segment->scanPoints) >  maxSegmentLength_ () ||
		segment->startEdgeType != EdgeType::RISING || segment->endEdgeType != EdgeType::FALLING)
		{
			continue;
		}
		goalPoints_.push_back((segment->start + segment->end).unaryExpr(shift));
	}
}

void GoalDetection::bombermanMaxDistanceGrouping() {
	goalPostGroups_.clear();
	auto it = goalPoints_.begin();
	for (; it != goalPoints_.end(); it++) {
		goalPostGroup_.clear();
		goalPostGroup_.push_back(*it);
		Vector2i point = *it;
		it--;
		goalPoints_.erase(std::next(it));
		bombermanExplodeRecursive(point);
		if (goalPostGroup_.size() >= minPointsInGroup_()) {
			goalPostGroups_.push_back(goalPostGroup_);
		}
		if (goalPoints_.size() == 0) {
			break;
		}
	}
}

void GoalDetection::bombermanExplodeRecursive(Vector2i point) {
	if (goalPoints_.size() == 0) {
		return;
	}
	auto it = goalPoints_.begin();
	for (; it != goalPoints_.end(); it++) {
		float distance = ((*it) - (point)).norm();
		if (distance <= maxDistanceOfNeighbors_()) {
			goalPostGroup_.push_back(*it);
			Vector2i next = *it;
			it--;
			goalPoints_.erase(std::next(it));
			bombermanExplodeRecursive(next);
			if (goalPoints_.size() == 0) {
				break;
			}
		}
	}
}

void GoalDetection::createGoalData() {
	debugGoalPoints_.clear();
	for (const auto& group : goalPostGroups_) {
		Vector2i bestFit = group.front();
		Vector2f goalPoint;
		for (const auto& point : group) {
			if (point.y() > bestFit.y()) {
				bestFit = point;
			}
		}
		if (cameraMatrix_->pixelToRobot(bestFit, goalPoint)) {
			goalData_->posts.push_back(goalPoint);
			debugGoalPoints_.push_back(bestFit);
		}
	}
	goalData_->timestamp = imageData_->timestamp;
	goalData_->valid = true;
}

void GoalDetection::cycle()
{
	if (!filteredSegments_->valid)
	{
		return;
	}
	{
		Chronometer time(debug(), mount_ + "." + imageData_->identification + "_cycle_time");
		detectGoalPoints();
		bombermanMaxDistanceGrouping();
		debugGoalPostGroups_ = goalPostGroups_;
		createGoalData();
	}
	sendImagesForDebug();
}

void GoalDetection::sendImagesForDebug()
{
	auto mount = mount_ + "." + imageData_->identification + "_image_goals";
	if (debug().isSubscribed(mount))
	{
		Image image(imageData_->image422.to444Image());
		for (const auto& group : debugGoalPostGroups_)
		{
			for (const auto& point : group) {
				image.circle(Image422::get444From422Vector(point), 2, Color::RED);
			}
		}
		for (const auto& point : debugGoalPoints_) {
			image.cross(Image422::get444From422Vector(point), 3, Color::BLUE);
		}
		debug().sendImage(mount_ + "." + imageData_->identification + "_image_goals", image);
	}
}
