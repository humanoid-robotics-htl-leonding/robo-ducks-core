#pragma once

#include "Framework/Module.hpp"

#include "Data/CameraMatrix.hpp"
#include "Data/FilteredSegments.hpp"
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
	 * @brief sendImagesForDebug send debug information
	 */
	void sendImagesForDebug();

  /// a reference to the image
	const Dependency<ImageData> imageData_;
	/// a reference to the camera matrix
	const Dependency<CameraMatrix> cameraMatrix_;
  /// a reference to the filtered segments
	const Dependency<FilteredSegments> filteredSegments_;
	/// the detected goal posts for other modules
  Production<GoalData> goalData_;
  /// goal points for debug purposes
	VecVector2i debugGoalPoints_;
  /// candidate points on lines
	VecVector2i goalPoints_;
};
