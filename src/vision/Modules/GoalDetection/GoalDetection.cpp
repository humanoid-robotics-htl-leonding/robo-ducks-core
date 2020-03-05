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
  , imageData_(*this)
  , cameraMatrix_(*this)
  , imageSegments_(*this)
  , fieldDimensions_(*this)
  , fieldBorder_(*this)
  , goalData_(*this)
{
}

void GoalDetection::detectGoalPoints()
{
	goalPoints_.clear();
    for(const auto& scanline : imageSegments_->verticalScanlines){
        for (const auto& segment : scanline.segments) {
			if(segment.startEdgeType == EdgeType::BORDER && segment.endEdgeType == EdgeType::FALLING &&
			    static_cast<unsigned int>(segment.scanPoints) > minSegmentLength_()){
			    if(fieldBorder_->isInsideField(segment.end) && !fieldBorder_->isInsideField(segment.start)){
                    goalPoints_.push_back(segment.end);
			    }
			}
        }
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
		if (goalPoints_.empty()) {
			break;
		}
	}
}

void GoalDetection::bombermanExplodeRecursive(Vector2i point) {
	if (goalPoints_.empty()) {
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
			if (goalPoints_.empty()) {
				break;
			}
		}
	}
}

void GoalDetection::createGoalData() {
	goalData_->posts.clear();
	debugGoalPoints_.clear();
	Vector2f goalPoint;
	for (auto& group : goalPostGroups_) {
	    Vector2f sum (0,0);
	    for(auto& groupPiece : group){
            if (cameraMatrix_->pixelToRobot(groupPiece, goalPoint)) {
                sum += goalPoint;
            }
	    }
	    sum /= group.size();
	    goalData_->posts.push_back(sum);
	    sum = sum/sum.norm()*(sum.norm()+ fieldDimensions_->goalPostDiameter/2);
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
		detectGoalPoints();
		bombermanMaxDistanceGrouping();
		debugGoalPostGroups_ = goalPostGroups_;
		createGoalData();
	}
	sendImagesForDebug();
	goalData_->valid = false;
}

void GoalDetection::sendImagesForDebug()
{
    Vector2i goalPixel;
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

        for (const auto& post : goalData_->posts) {
            if (cameraMatrix_->robotToPixel(post, goalPixel)) {
                image.cross(Image422::get444From422Vector(goalPixel), 5, Color::BLUE);
            }
        }
		debug().sendImage(mount_ + "." + imageData_->identification + "_image_goals", image);
	}
}


