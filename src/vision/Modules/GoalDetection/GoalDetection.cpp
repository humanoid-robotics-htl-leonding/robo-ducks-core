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
	, maxTilt_(*this, "maxTilt", [] {})
  , imageData_(*this)
  , cameraMatrix_(*this)
  , filteredSegments_(*this)
  , fieldDimensions_(*this)
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

bool GoalDetection::checkGroup(VecVector2i& group) {
	if (goalPostGroup_.size() < minPointsInGroup_()) {
		return false;
	}
	std::sort(group.begin(), group.end(),
			  [](const Vector2i& p1, const Vector2i& p2) { return (p1.y() < p2.y()); });
	float tilt = (group.front().x() - group.back().x()) / (double)(group.front().y() - group.back().y());
	return tilt < maxTilt_() && tilt > -maxTilt_();
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
		if (checkGroup(goalPostGroup_)) {
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
	goalData_->posts.clear();
	debugGoalPoints_.clear();
	std::sort(goalPostGroups_.begin(), goalPostGroups_.end(),
			  [](const VecVector2i& g1, const VecVector2i& g2) { return (g1.back().y() < g2.back().y()); });
	if (goalPostGroups_.size() == 1) {
		Vector2f goalPoint;
		if (cameraMatrix_->pixelToRobot(goalPostGroups_.front().back(), goalPoint)) {
			goalData_->posts.push_back(goalPoint);
			debugGoalPoints_.push_back(goalPostGroups_.front().back());
		}
	}
	else if (goalPostGroups_.size() > 1) {
		float goalPostDistance = fieldDimensions_->goalInnerWidth + fieldDimensions_->goalPostDiameter;
		float bestFit = -10;
		Vector2f goalPoint1;
		Vector2f goalPoint2;
		auto it = goalPostGroups_.begin();
		for (; std::next(it) != goalPostGroups_.end(); it++) {
			if (!cameraMatrix_->pixelToRobot((*it).back(), goalPoint1)) {
				continue;
			}
			auto ti = std::next(it);
			for (; ti != goalPostGroups_.end(); ti++) {
				if (!cameraMatrix_->pixelToRobot((*ti).back(), goalPoint2)) {
					continue;
				}
				float distance = (goalPoint1 - goalPoint2).norm();
				if (abs(distance - goalPostDistance) < abs(bestFit - goalPostDistance)) {
					debugGoalPoints_.clear();
					debugGoalPoints_.push_back((*it).back());
					debugGoalPoints_.push_back((*ti).back());
					bestFit = distance;
				}
			}
		}
		for (const auto& debugGoalPoint : debugGoalPoints_) {
			Vector2f goalPoint;
			if (cameraMatrix_->pixelToRobot(debugGoalPoint, goalPoint)) {
				goalData_->posts.push_back(goalPoint);
			}
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
