#include <cmath>

#include "FieldInfo.hpp"
#include <Definitions/windows_definition_fix.hpp>

FieldInfo::FieldInfo(const FieldDimensions& fieldDimensions)
{
  const float fieldLength = fieldDimensions.fieldLength;
  const float fieldWidth = fieldDimensions.fieldWidth;
  const float penaltyAreaWidth = fieldDimensions.fieldPenaltyAreaWidth;
  const float penaltyAreaLength = fieldDimensions.fieldPenaltyAreaLength;
  const float goalBoxWidth = fieldDimensions.fieldGoalBoxWidth;
  const float goalBoxLength = fieldDimensions.fieldGoalBoxLength;
  const float centerCircleRadius = fieldDimensions.fieldCenterCircleDiameter * 0.5f;
  // The goal post position is assumed to be in the center of the post, thus two goal post radii have to be added (a.k.a one diameter).
  const float goalPostDistance = fieldDimensions.goalInnerWidth + fieldDimensions.goalPostDiameter;
  const float goalBoxBorder = fieldLength * 0.5f - goalBoxLength;
  const float penaltyAreaBorder = fieldLength * 0.5f - penaltyAreaLength;

  // field border
  lines.push_back({{-fieldLength / 2, fieldWidth / 2}, {fieldLength / 2, fieldWidth / 2}}); // top, ID: 1
  lines.push_back({{-fieldLength / 2, -fieldWidth / 2}, {fieldLength / 2, -fieldWidth / 2}}); // bottom, ID: 2
  lines.push_back({{-fieldLength / 2, fieldWidth / 2}, {-fieldLength / 2, -fieldWidth / 2}}); // left,  ID: 3
  lines.push_back({{fieldLength / 2, fieldWidth / 2}, {fieldLength / 2, -fieldWidth / 2}}); // right, ID: 4

  // center line
  lines.push_back({{0, fieldWidth / 2}, {0, -fieldWidth / 2}}); // ID: 5

  // center circle as polygon
  for (int i = 0; i < 16; i++)// phi < 2 * M_PI; phi += M_PI / 8)
  {
  	float phi = M_PI * i / 8;
    lines.emplace_back(polar2cartesian({centerCircleRadius, phi}), polar2cartesian({centerCircleRadius, phi + static_cast<float>(M_PI / 8)})); // ID: 0
  }

  // own goal box
  lines.push_back({{-fieldLength / 2, goalBoxWidth / 2}, {-goalBoxBorder, goalBoxWidth / 2}}); // top, ID: 6
  lines.push_back({{-fieldLength / 2, -goalBoxWidth / 2}, {-goalBoxBorder, -goalBoxWidth / 2}}); // bottom, ID: 7
  lines.push_back({{-goalBoxBorder, goalBoxWidth / 2}, {-goalBoxBorder, -goalBoxWidth / 2}}); // right, ID: 8

  // opponent goal box
  lines.push_back({{fieldLength / 2, goalBoxWidth / 2}, {goalBoxBorder, goalBoxWidth / 2}}); // top, ID: 9
  lines.push_back({{fieldLength / 2, -goalBoxWidth / 2}, {goalBoxBorder, -goalBoxWidth / 2}}); // bottom, ID 10
  lines.push_back({{goalBoxBorder, goalBoxWidth / 2}, {goalBoxBorder, -goalBoxWidth / 2}}); // left, ID: 11

  // own penalty area
  lines.push_back({{-fieldLength / 2, penaltyAreaWidth / 2}, {-penaltyAreaBorder, penaltyAreaWidth / 2}}); // top, ID: 12
  lines.push_back({{-fieldLength / 2, -penaltyAreaWidth / 2}, {-penaltyAreaBorder, -penaltyAreaWidth / 2}}); // bottom, ID: 13
  lines.push_back({{-penaltyAreaBorder, penaltyAreaWidth / 2}, {-penaltyAreaBorder, -penaltyAreaWidth / 2}}); // right, ID: 14

  // opponent penalty area
  lines.push_back({{fieldLength / 2, penaltyAreaWidth / 2}, {penaltyAreaBorder, penaltyAreaWidth / 2}}); // top, ID: 15
  lines.push_back({{fieldLength / 2, -penaltyAreaWidth / 2}, {penaltyAreaBorder, -penaltyAreaWidth / 2}}); // bottom, ID 16
  lines.push_back({{penaltyAreaBorder, penaltyAreaWidth / 2}, {penaltyAreaBorder, -penaltyAreaWidth / 2}}); // left, ID: 17


	// intersections
	LandmarkModel::Intersection intersection;

	// center circle
	// top
	intersection.type = LandmarkModel::Intersection::Type::X;
	intersection.position = Vector2f(0, centerCircleRadius);
	intersection.hasOrientation = false;
	intersection.onLine1 = true;
	intersection.onLine2 = true;
	intersection.usedLineIds = {0, 5};
	intersections.push_back(intersection);

	//bottom
	intersection.type = LandmarkModel::Intersection::Type::X;
	intersection.position = Vector2f(0, -centerCircleRadius);
	intersection.hasOrientation = false;
	intersection.onLine1 = true;
	intersection.onLine2 = true;
	intersection.usedLineIds = {0, 5};
	intersections.push_back(intersection);

	// middle line
	// top
	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(0, fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = -M_PI / 2;
	intersection.onLine1 = true;
	intersection.onLine2 = false;
	intersection.usedLineIds = {1, 5};
	intersections.push_back(intersection);

	// bottom
	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(0, -fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI / 2;
	intersection.onLine1 = true;
	intersection.onLine2 = false;
	intersection.usedLineIds = {2, 5};
	intersections.push_back(intersection);

	// corners
	// top left
	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(-fieldLength / 2, fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = -M_PI * 1 / 4;
	intersection.onLine1 = false;
	intersection.onLine2 = false;
	intersection.usedLineIds = {1, 3};
	intersections.push_back(intersection);

	// top right
	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(fieldLength / 2, fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = -M_PI * 3 / 4;
	intersection.onLine1 = false;
	intersection.onLine2 = false;
	intersection.usedLineIds = {1, 4};
	intersections.push_back(intersection);

	// bottom right
	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(fieldLength / 2, -fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI * 3 / 4;
	intersection.onLine1 = false;
	intersection.onLine2 = false;
	intersection.usedLineIds = {2, 4};
	intersections.push_back(intersection);

	// bottom left
	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(-fieldLength / 2, -fieldWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI * 1 / 4;
	intersection.onLine1 = false;
	intersection.onLine2 = false;
	intersection.usedLineIds = {2, 3};
	intersections.push_back(intersection);

	// own penalty area
	// top left
	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(-fieldLength / 2, penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = 0;
	intersection.onLine1 = true;
	intersection.onLine2 = false;
	intersection.usedLineIds = {3, 12};
	intersections.push_back(intersection);

	// bottom left
	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(-fieldLength / 2, -penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = 0;
	intersection.onLine1 = true;
	intersection.onLine2 = false;
	intersection.usedLineIds = {3, 13};
	intersections.push_back(intersection);

	// top right
	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(-penaltyAreaBorder, penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = -M_PI * 3 / 4;
	intersection.onLine1 = false;
	intersection.onLine2 = false;
	intersection.usedLineIds = {12, 14};
	intersections.push_back(intersection);

	// bottom right
	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(-penaltyAreaBorder, -penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI * 3 / 4;
	intersection.onLine1 = false;
	intersection.onLine2 = false;
	intersection.usedLineIds = {13, 14};
	intersections.push_back(intersection);

	// opponent penalty area
	// top right
	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(fieldLength / 2, penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI;
	intersection.onLine1 = true;
	intersection.onLine2 = false;
	intersection.usedLineIds = {4, 15};
	intersections.push_back(intersection);

	// bottom right
	intersection.type = LandmarkModel::Intersection::Type::T;
	intersection.position = Vector2f(fieldLength / 2, -penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI;
	intersection.onLine1 = true;
	intersection.onLine2 = false;
	intersection.usedLineIds = {4, 16};
	intersections.push_back(intersection);

	// top left
	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(penaltyAreaBorder, penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = -M_PI * 1 / 4;
	intersection.onLine1 = false;
	intersection.onLine2 = false;
	intersection.usedLineIds = {15, 17};
	intersections.push_back(intersection);

	// bottom left
	intersection.type = LandmarkModel::Intersection::Type::L;
	intersection.position = Vector2f(penaltyAreaBorder, -penaltyAreaWidth / 2);
	intersection.hasOrientation = true;
	intersection.orientation = M_PI * 1 / 4;
	intersection.onLine1 = false;
	intersection.onLine2 = false;
	intersection.usedLineIds = {16, 17};
	intersections.push_back(intersection);

	// own goal box
    // top left
    intersection.type = LandmarkModel::Intersection::Type::T;
    intersection.position = Vector2f(-fieldLength / 2, goalBoxWidth / 2);
    intersection.hasOrientation = true;
    intersection.orientation = 0;
    intersection.onLine1 = true;
    intersection.onLine2 = false;
    intersection.usedLineIds = {3, 6};
    intersections.push_back(intersection);

    // bottom left
    intersection.type = LandmarkModel::Intersection::Type::T;
    intersection.position = Vector2f(-fieldLength / 2, -goalBoxWidth / 2);
    intersection.hasOrientation = true;
    intersection.orientation = 0;
    intersection.onLine1 = true;
    intersection.onLine2 = false;
    intersection.usedLineIds = {3, 7};
    intersections.push_back(intersection);

    // top right
    intersection.type = LandmarkModel::Intersection::Type::L;
    intersection.position = Vector2f(-goalBoxBorder, goalBoxWidth / 2);
    intersection.hasOrientation = true;
    intersection.orientation = -M_PI * 3 / 4;
    intersection.onLine1 = false;
    intersection.onLine2 = false;
    intersection.usedLineIds = {6, 8};
    intersections.push_back(intersection);

    // bottom right
    intersection.type = LandmarkModel::Intersection::Type::L;
    intersection.position = Vector2f(-goalBoxBorder, -goalBoxWidth / 2);
    intersection.hasOrientation = true;
    intersection.orientation = M_PI * 3 / 4;
    intersection.onLine1 = false;
    intersection.onLine2 = false;
    intersection.usedLineIds = {7, 8};
    intersections.push_back(intersection);

    // opponent goal box
    // top right
    intersection.type = LandmarkModel::Intersection::Type::T;
    intersection.position = Vector2f(fieldLength / 2, goalBoxWidth / 2);
    intersection.hasOrientation = true;
    intersection.orientation = M_PI;
    intersection.onLine1 = true;
    intersection.onLine2 = false;
    intersection.usedLineIds = {4, 9};
    intersections.push_back(intersection);

    // bottom right
    intersection.type = LandmarkModel::Intersection::Type::T;
    intersection.position = Vector2f(fieldLength / 2, -goalBoxWidth / 2);
    intersection.hasOrientation = true;
    intersection.orientation = M_PI;
    intersection.onLine1 = true;
    intersection.onLine2 = false;
    intersection.usedLineIds = {4, 10};
    intersections.push_back(intersection);

    // top left
    intersection.type = LandmarkModel::Intersection::Type::L;
    intersection.position = Vector2f(goalBoxBorder, goalBoxWidth / 2);
    intersection.hasOrientation = true;
    intersection.orientation = -M_PI * 1 / 4;
    intersection.onLine1 = false;
    intersection.onLine2 = false;
    intersection.usedLineIds = {9, 11};
    intersections.push_back(intersection);

    // bottom left
    intersection.type = LandmarkModel::Intersection::Type::L;
    intersection.position = Vector2f(goalBoxBorder, -goalBoxWidth / 2);
    intersection.hasOrientation = true;
    intersection.orientation = M_PI * 1 / 4;
    intersection.onLine1 = false;
    intersection.onLine2 = false;
    intersection.usedLineIds = {10, 11};
    intersections.push_back(intersection);

  // own goal
  ownGoal.left = Vector2f(-fieldLength / 2, -goalPostDistance / 2);
  ownGoal.right = Vector2f(-fieldLength / 2, goalPostDistance / 2);
  ownGoal.hasOrientation = true;
  ownGoal.orientation = 0;

  // opponent goal
	opponentGoal.left = Vector2f(fieldLength / 2, goalPostDistance / 2);
  opponentGoal.right = Vector2f(fieldLength / 2, -goalPostDistance / 2);
  ownGoal.hasOrientation = true;
  opponentGoal.orientation = M_PI;

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
