#include <cmath>

#include "FieldInfo.hpp"
#include <Definitions/windows_definition_fix.hpp>

FieldInfo::FieldInfo(const FieldDimensions& fieldDimensions)
{
  const float fieldLength = fieldDimensions.fieldLength;
  const float fieldWidth = fieldDimensions.fieldWidth;
  const float penaltyAreaWidth = fieldDimensions.fieldPenaltyAreaWidth;
  const float penaltyAreaLength = fieldDimensions.fieldPenaltyAreaLength;
  const float centerCircleRadius = fieldDimensions.fieldCenterCircleDiameter * 0.5f;
  // The goal post position is assumed to be in the center of the post, thus two goal post radii have to be added (a.k.a one diameter).
  const float goalPostDistance = fieldDimensions.goalInnerWidth + fieldDimensions.goalPostDiameter;
  const float penaltyAreaBorder = fieldLength * 0.5f - penaltyAreaLength;

  // field border
  lines.push_back({{-fieldLength / 2, fieldWidth / 2}, {fieldLength / 2, fieldWidth / 2}});
  lines.push_back({{-fieldLength / 2, -fieldWidth / 2}, {fieldLength / 2, -fieldWidth / 2}});
  lines.push_back({{-fieldLength / 2, fieldWidth / 2}, {-fieldLength / 2, -fieldWidth / 2}});
  lines.push_back({{fieldLength / 2, fieldWidth / 2}, {fieldLength / 2, -fieldWidth / 2}});

  // center line
  lines.push_back({{0, fieldWidth / 2}, {0, -fieldWidth / 2}});

  // center circle as polygon
  for (int i = 0; i < 16; i++)// phi < 2 * M_PI; phi += M_PI / 8)
  {
  	float phi = M_PI * i / 8;
    lines.emplace_back(polar2cartesian({centerCircleRadius, phi}), polar2cartesian({centerCircleRadius, phi + static_cast<float>(M_PI / 8)}));
  }

  // own penalty area
  lines.push_back({{-fieldLength / 2, penaltyAreaWidth / 2}, {-penaltyAreaBorder, penaltyAreaWidth / 2}});
  lines.push_back({{-fieldLength / 2, -penaltyAreaWidth / 2}, {-penaltyAreaBorder, -penaltyAreaWidth / 2}});
  lines.push_back({{-penaltyAreaBorder, penaltyAreaWidth / 2}, {-penaltyAreaBorder, -penaltyAreaWidth / 2}});

  // opponent penalty area
  lines.push_back({{fieldLength / 2, penaltyAreaWidth / 2}, {penaltyAreaBorder, penaltyAreaWidth / 2}});
  lines.push_back({{fieldLength / 2, -penaltyAreaWidth / 2}, {penaltyAreaBorder, -penaltyAreaWidth / 2}});
  lines.push_back({{penaltyAreaBorder, penaltyAreaWidth / 2}, {penaltyAreaBorder, -penaltyAreaWidth / 2}});

	// intersections
	LandmarkModel::Intersection intersection;

	// center circle
	intersection.type = LandmarkModel::Intersection::Type::X;
	intersection.position = Vector2f(0, centerCircleRadius);
	intersection.hasOrientation = false;
	intersections.push_back(intersection);

	intersection.type = LandmarkModel::Intersection::Type::X;
	intersection.position = Vector2f(0, -centerCircleRadius);
	intersection.hasOrientation = false;
	intersections.push_back(intersection);

	// middle line
	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(0, fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = -M_PI / 2;
	intersections.push_back(intersection);

	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(0, -fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI / 2;
	intersections.push_back(intersection);

	// corners
	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(-fieldLength / 2, fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = -M_PI * 1 / 4;
	intersections.push_back(intersection);

	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(fieldLength / 2, fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = -M_PI * 3 / 4;
	intersections.push_back(intersection);

	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(fieldLength / 2, -fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI * 3 / 4;
	intersections.push_back(intersection);

	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(-fieldLength / 2, -fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI * 1 / 4;
	intersections.push_back(intersection);

	// own penalty area
	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(-fieldLength / 2, penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = 0;
	intersections.push_back(intersection);

	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(-fieldLength / 2, -penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = 0;
	intersections.push_back(intersection);

	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(-penaltyAreaBorder, penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = -M_PI * 3 / 4;
	intersections.push_back(intersection);

	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(-penaltyAreaBorder, -penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI * 3 / 4;
	intersections.push_back(intersection);

	// opponent penalty area
	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(fieldLength / 2, penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI;
	intersections.push_back(intersection);

	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(fieldLength / 2, -penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI;
	intersections.push_back(intersection);

	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(penaltyAreaBorder, penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = -M_PI * 1 / 4;
	intersections.push_back(intersection);

	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(penaltyAreaBorder, -penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI * 1 / 4;
	intersections.push_back(intersection);

  // own goal
  ownGoal.left = Vector2f(-fieldLength / 2, -goalPostDistance / 2);
  ownGoal.right = Vector2f(-fieldLength / 2, goalPostDistance / 2);
  ownGoal.hasOrientation = true;
  ownGoal.orientation = M_PI;

  // opponent goal
	opponentGoal.left = Vector2f(fieldLength / 2, goalPostDistance / 2);
  opponentGoal.right = Vector2f(fieldLength / 2, -goalPostDistance / 2);
  ownGoal.hasOrientation = true;
  opponentGoal.orientation = 0;

  // own penalty area
  ownPenaltyArea.position = Vector2f(-fieldLength / 2 + fieldDimensions.fieldPenaltyMarkerDistance, 0.f);
  ownPenaltyArea.hasOrientation = true;
	ownPenaltyArea.orientation = M_PI;

  // opponent penalty area
  opponentPenaltyArea.position = Vector2f(fieldLength / 2 - fieldDimensions.fieldPenaltyMarkerDistance, 0.f);
  opponentPenaltyArea.hasOrientation = true;
  opponentPenaltyArea.orientation = 0;
}

Vector2f FieldInfo::polar2cartesian(const Vector2f& polar) const
{
  float r = polar.x();
  float phi = polar.y();
  return {r * std::cos(phi), r * std::sin(phi)};
}
