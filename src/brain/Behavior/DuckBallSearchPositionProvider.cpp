#include <Eigen/Dense>

#include "Tools/Chronometer.hpp"
#include "Tools/Math/HungarianMethod.hpp"
#include "Tools/Math/Hysteresis.hpp"
#include "Tools/Math/Range.hpp"

#include "print.h"

#include "DuckBallSearchPositionProvider.hpp"


DuckBallSearchPositionProvider::DuckBallSearchPositionProvider(const ModuleManagerInterface& manager)
  : Module(manager)
  , ballSearchMap_(*this)
  , gameControllerState_(*this)
  , playerConfiguration_(*this)
  , playingRoles_(*this)
  , teamPlayers_(*this)
  , ballState_(*this)
  , robotPosition_(*this)
  , bodyPose_(*this)
  , teamBallModel_(*this)
  , fieldDimensions_(*this)
  , jointSensorData_(*this)
  , cycleInfo_(*this)
  , minBallDetectionRange_(*this, "minBallDetectionRange", [] {})
  , maxBallDetectionRange_(*this, "maxBallDetectionRange", [] {})
  , maxAgeValueContribution_(*this, "maxAgeValueContribution", [] {})
  , probabilityWeight_(*this, "probabilityWeight", [] {})
  , voronoiSeeds_(*this, "voronoiSeeds", [this] { rebuildSearchAreas(); })
  , searchPosition_(*this)
  , fieldLength_(fieldDimensions_->fieldLength)
  , fieldWidth_(fieldDimensions_->fieldWidth)
{
}

void DuckBallSearchPositionProvider::cycle()
{
  {
	  Chronometer time(debug(), mount_ + ".cycle_time");

		auto distVec = teamBallModel_->position - robotPosition_->pose.position;
		auto dist = distVec.norm();

	  debug().update(mount_ + ".distance", dist);

  }
  sendDebug();
}

float DuckBallSearchPositionProvider::timeToReachPosition(const TeamPlayer& player,
                                                      const Vector2f position) const
{
  const Vector2f relPosition = position - player.pose.position;
  // TODO: Is 15cm per second a good approximation?
  const float walkTimeDistance = relPosition.norm() / 0.18f;
  // TODO: Is 10s per 180° a good approximation?
  const float cellOrientation = std::atan2(relPosition.y(), relPosition.x());
  const auto rotateTimeDistance =
      static_cast<float>(Angle::angleDiff(cellOrientation, player.pose.orientation) * 10.f / M_PI);
  // TODO: Is 10s a good approximation?
  const float fallenPenalty = player.fallen ? 10.0f : 0.0f;

  return walkTimeDistance + rotateTimeDistance + fallenPenalty;
}

float DuckBallSearchPositionProvider::timeToReachCell(const TeamPlayer& player,
                                                  const ProbCell& cell) const
{
  return timeToReachPosition(player, cell.position);
}

float DuckBallSearchPositionProvider::getValue(const ProbCell& cell) const
{
  return cell.probability * probabilityWeight_() +
         std::min(maxAgeValueContribution_(), static_cast<float>(cell.age));
}

void DuckBallSearchPositionProvider::sendDebug()
{
  debug().update(mount_ + ".explorerCount", explorers_.size());
  if (!explorers_.empty())
  {
    if (debug().isSubscribed(mount_ + ".voronoiSeeds"))
    {
      VecVector2f seeds;
      for (auto& seed : voronoiSeeds_()[explorers_.size() - 1])
      {
        seeds.emplace_back(seed.x() * fieldLength_ * 0.5f, seed.y() * fieldWidth_ * 0.5f);
      }
      debug().update(mount_ + ".voronoiSeeds", seeds);
    }
  }
}

float DuckBallSearchPositionProvider::getCosts(const TeamPlayer& player, const ProbCell& cellToExplore)
{
  return (timeToReachCell(player, cellToExplore) + 2.f) / getValue(cellToExplore);
}

void DuckBallSearchPositionProvider::rebuildSearchAreas()
{
  searchAreas_.clear();
  searchAreas_.reserve(explorers_.size());

  if (explorers_.empty())
  {
    return;
  }

  for (auto& seed : voronoiSeeds_()[explorers_.size() - 1])
  {
    SearchArea area;
    area.voronoiSeed = {seed.x() * fieldLength_ / 2.f, seed.y() * fieldWidth_ / 2.f};
    area.defaultPosition = area.voronoiSeed;
    area.cellToExplore = &ballSearchMap_->cellFromPositionConst(area.defaultPosition);
    searchAreas_.emplace_back(area);
  }

  // voronoi (https://en.wikipedia.org/wiki/Voronoi_diagram)
  // The field is divided into so called searchAreas. Division is done by reading
  // the seeds (aka generators) coming from the config and do some voronoi on them.
  for (auto& cell : ballSearchMap_->probabilityList_)
  {
    SearchArea* minimumDistanceArea = &(searchAreas_[0]);
    float minimumDistance = std::numeric_limits<float>::max();
    int areaNum = 0;

    for (auto& area : searchAreas_)
    {
      Vector2f relDistance = area.voronoiSeed - cell->position;
      float areaDistance = relDistance.squaredNorm();
      if (areaDistance < minimumDistance)
      {
        minimumDistance = areaDistance;
        minimumDistanceArea = &area;
      }
      areaNum++;
    }
    minimumDistanceArea->cells.push_back(cell);
  }
}
