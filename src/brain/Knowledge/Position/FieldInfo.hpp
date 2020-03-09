#pragma once

#include <vector>

#include "Data/FieldDimensions.hpp"
#include "Data/LandmarkModel.hpp"
#include "Tools/Math/Eigen.hpp"
#include "Tools/Math/Line.hpp"
#include "Tools/Math/Pose.hpp"

class FieldInfo
{
public:
  /**
   * @brief FieldInfo constructs an array of lines and goalPosts from the dimensions stored in the FieldDimensions
   * @param fieldDimensions a reference to the field dimensions
   */
  FieldInfo(const FieldDimensions& fieldDimensions);
  /// contains all the lines on the field
  std::vector<Line<float>> lines;
  /// contains all the intersections on the field
  std::vector<LandmarkModel::Intersection> intersections;
  /// contains the own goal
  LandmarkModel::Goal ownGoal;
  /// contains the opponent goal
  LandmarkModel::Goal opponentGoal;
  /// contains the own penalty area
  LandmarkModel::PenaltyArea ownPenaltyArea;
  /// contains the opponent penalty area
  LandmarkModel::PenaltyArea opponentPenaltyArea;

private:
  /**
   * @brief polar2cartesian converts a vector in polar coordinates to cartesian coordinates
   * @param polar a vector containing the radius and the angle (r, phi)
   * @return a vector in cartesian coordinates (x, y)
   */
  Vector2f polar2cartesian(const Vector2f& polar) const;
};
