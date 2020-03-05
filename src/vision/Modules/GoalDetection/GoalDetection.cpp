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
  , fieldBorder_(*this)
  , imageSegments_(*this)
  , goalData_(*this)
{
}

void GoalDetection::detectGoalPoints()
{
	goalPoints_.clear();
	debugPoints_.clear();
	auto shift = [](int c) { return c >> 1; };
	for (const auto& scanline : imageSegments_->horizontalScanlines)
	{
		for (const auto& segment : scanline.segments) {
			if (static_cast<unsigned int>(segment.scanPoints) <  minSegmentLength_() ||
				static_cast<unsigned int>(segment.scanPoints) >  maxSegmentLength_() ||
				segment.startEdgeType != EdgeType::RISING || segment.endEdgeType != EdgeType::FALLING)
			{
				continue;
			}
			goalPoints_.push_back((segment.start + segment.end).unaryExpr(shift));
		}
	}
	debugPoints_ = goalPoints_;
}

bool GoalDetection::checkGroup(VecVector2i& group) {
	if (goalPostGroup_.size() < minPointsInGroup_()) {
		return false;
	}
	std::sort(group.begin(), group.end(),
			  [](const Vector2i& p1, const Vector2i& p2) { return (p1.y() < p2.y()); });
	if (fieldBorder_->isInsideField(group.front()) || !fieldBorder_->isInsideField(group.back())) {
		return false;
	}
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
	Vector2f goalPost;
	for (auto& group : goalPostGroups_) {
		if (cameraMatrix_->pixelToRobot(group.back(), goalPost)) {
			debugGoalPoints_.push_back(group.back());
			goalData_->posts.push_back(goalPost);
		}
	}
	goalData_->timestamp = imageData_->timestamp;
	goalData_->valid = true;
}

void GoalDetection::cycle()
{
	if (!imageSegments_->valid)
	{
		return;
	}
	{
		Chronometer time(debug(), mount_ + "." + imageData_->identification + "_cycle_time");
		//detectGoalPoints();
		//bombermanMaxDistanceGrouping();
		//createGoalData();
	}
	//sendImagesForDebug();
	goalData_->valid = false;
}

void GoalDetection::sendImagesForDebug()
{
	auto mount = mount_ + "." + imageData_->identification + "_image_goals";
	if (debug().isSubscribed(mount))
	{
		Image image(imageData_->image422.to444Image());
		for (const auto& point : debugPoints_)
		{
			image.circle(Image422::get444From422Vector(point), 2, Color::RED);
		}
		for (const auto& point : debugGoalPoints_) {
			image.cross(Image422::get444From422Vector(point), 5, Color::BLUE);
		}
		debug().sendImage(mount_ + "." + imageData_->identification + "_image_goals", image);
	}
}
