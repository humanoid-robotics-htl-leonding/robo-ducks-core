#pragma once

#include "Framework/Module.hpp"

#include "Data/CameraMatrix.hpp"
#include "Data/FieldBorder.hpp"
#include "Data/ImageSegments.hpp"
#include "Data/ImageData.hpp"
#include "Data/GoalData.hpp"

class Brain;

class GoalDetection : public Module<GoalDetection, Brain>
{
public:
  ModuleName name = "GoalDetection";
  GoalDetection(const ModuleManagerInterface& manager);
  void cycle();

private:
	/**
 	 * @brief getGradient calculates the normalized gradient in the y channel
 	 * @param p a point in pixel coordinates at which the gradient is computed
 	 * @return the gradient
 	 */
	Vector2f getGradient(const Vector2i& p) const;
  	/**
  	 * @brief detectGoalPoints uses the scanline segments and detects points which could belong to a
  	 * goal post
  	 */
  	void detectGoalPoints();
  	/**
  	 * @brief checkGroup returns true if the points match the minimum requited amount and if the points form a vertical
  	 * line that is tilted less than maxTilt
  	 */
  	bool checkGroup(VecVector2i& group);
  	/**
   	 * @brief bombermanMaxDistanceGrouping is a grouping algorithm that groups points that are close to any existing
   	 * member of a group.
   	 *
   	 * See www.github.com/humanoid-robotics-htl-leonding/robo-ducks-documentation/blob/master/docs/bombermanGrouping.md
   	 * for a more detailed description.
   	 */
   	void bombermanMaxDistanceGrouping();
  	/**
  	 * @brief the recursive part of the bomberman max distance grouping
  	 */
  	void bombermanExplodeRecursive(Vector2i point);
  	/**
  	 * @brief takes the two best matching goal post candidates and converts them to points in robot coordinates
  	 */
  	void createGoalData();

  	/**
	 * @brief sendImagesForDebug send debug information
	 */
	void sendImagesForDebug();
	/// the maximum distance between two neighbors
	const Parameter<unsigned int> maxDistanceOfNeighbors_;
	/// the minimum number of points in a group
	const Parameter<unsigned int> minPointsInGroup_;
	/// the minimum number of points in a goal post segment
	const Parameter<unsigned int> minSegmentLength_;
	/// the maximum number of points in a goal post segment
	const Parameter<unsigned int> maxSegmentLength_;
	/// the maximum tilt from of a goal post group in x / y
	const Parameter<float> maxTilt_;
  	/// a reference to the image
	const Dependency<ImageData> imageData_;
	/// a reference to the camera matrix
	const Dependency<CameraMatrix> cameraMatrix_;
	/// a reference to the field border
	const Dependency<FieldBorder> fieldBorder_;
  	/// a reference to the filtered segments
	const Dependency<ImageSegments> imageSegments_;
	/// the detected goal posts for other modules
  	Production<GoalData> goalData_;
  	/// goal post candidates for debug purposes
	VecVector2i debugPoints_;
	/// goal post points for debug purposes
	VecVector2i debugGoalPoints_;
  	/// candidate points
	VecVector2i goalPoints_;
	/// goal post point groups
	std::vector<VecVector2i> goalPostGroups_;
	/// current goal post group
	VecVector2i goalPostGroup_;
};
