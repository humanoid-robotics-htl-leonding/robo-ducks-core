#include "Tools/Chronometer.hpp"

#include "GoalDetection.hpp"

GoalDetection::GoalDetection(const ModuleManagerInterface& manager)
  : Module(manager)
  , imageData_(*this)
  , cameraMatrix_(*this)
  , filteredSegments_(*this)
  , goalData_(*this)
{
}

Vector2f GoalDetection::getGradient(const Vector2i& p) const
{
		std::function<uint8_t(const YCbCr422&)> y1 = [](const YCbCr422& c) { return c.y1_; };
		std::function<uint8_t(const YCbCr422&)> y2 = [](const YCbCr422& c) { return c.y2_; };
		const uint8_t one = 1;
		auto y = (p.x() & one) == 1 ? y2 : y1;
		Vector2f gradient = Vector2f::Zero();
		const Image422& im = imageData_->image422;
		if (p.x() < 1 || p.y() < 1 || p.x() + 1 >= im.size.x() || p.y() + 1 >= im.size.y())
		{
				return gradient;
		}
		gradient.x() = y(im.at(p.y() + 1, p.x() - 1)) + 2 * y(im.at(p.y() + 1, p.x())) +
									 y(im.at(p.y() + 1, p.x() + 1)) - y(im.at(p.y() - 1, p.x() - 1)) -
									 2 * y(im.at(p.y() - 1, p.x())) - y(im.at(p.y() - 1, p.x() + 1));
		gradient.y() = y(im.at(p.y() - 1, p.x() - 1)) + 2 * y(im.at(p.y(), p.x() - 1)) +
									 y(im.at(p.y() + 1, p.x() - 1)) - y(im.at(p.y() - 1, p.x() + 1)) -
									 2 * y(im.at(p.y(), p.x() + 1)) - y(im.at(p.y() + 1, p.x() + 1));
		return gradient.normalized();
}

void GoalDetection::detectGoalPoints()
{
		goalPoints_.clear();
		Vector2f g1, g2;
		auto shift = [](int c) { return c >> 1; };
		for (const auto& segment : filteredSegments_->horizontal)
		{
				if (segment->startEdgeType != EdgeType::RISING || segment->endEdgeType != EdgeType::FALLING)
				{
						continue;
				}
				g1 = getGradient(segment->start);
				g2 = getGradient(segment->end);
				if (g1.dot(g2) > -0.95)
				{
						continue;
				}
				goalPoints_.push_back((segment->start + segment->end).unaryExpr(shift));
		}
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
				debugGoalPoints_ = goalPoints_;
				//ransacHandler();
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
