#include "Modules/Configuration/Configuration.h"
#include "Tools/Chronometer.hpp"
#include "Tools/Storage/Image.hpp"
#include "print.hpp"

#include "CameraCalibration.hpp"


CameraCalibration::CameraCalibration(const ModuleManagerInterface& manager)
  : Module(manager)
  , image_data_(*this)
  , camera_matrix_(*this)
  , field_dimensions_(*this)
  , head_matrix_buffer_(*this)
{
}

void CameraCalibration::cycle()
{
  Chronometer time(debug(), mount_ + ".cycle_time");
  /**
   * Put all calibration code AFTER this if block
   */

  /// torso2ground and head2torso are needed to construct the transformation chain.
  if (!head_matrix_buffer_->buffer.empty())
  {
    const HeadMatrixWithTimestamp& bufferEntry =
        head_matrix_buffer_->getBestMatch(image_data_->timestamp);
    debug().update(mount_ + ".Torso2Ground", bufferEntry.torso2ground);
    debug().update(mount_ + ".Head2Torso", bufferEntry.head2torso);
  }
  // send cam2ground via debug. ex: mount_Camera2Ground
  debug().update(mount_ + ".Camera2Ground_" + image_data_->identification,
                 camera_matrix_->camera2ground);

  const std::string syncImageMount = mount_ + "." + image_data_->identification + "_image";
  if (debug().isSubscribed(syncImageMount))
  {
    /// send raw image. Explicit purpose is to ensure synchronization.
    debug().sendImage(syncImageMount, image_data_->image422.to444Image());
  }

  /**
   * If and only if calibration image is requested for penalty area based calibration
   */
  if (!debug().isSubscribed(mount_ + "." + image_data_->identification + "_penalty_project_image"))
  {
    return;
  }

  calibImage_ = image_data_->image422.to444Image();
  if (image_data_->camera == Camera::TOP) {
	  projectTopCamera();
  }
  else {
	  projectBottomCamera();
  }
  sendImageForCalibration();
}

void CameraCalibration::projectTopCamera()
{
  Vector2f penaltyAreaTopLeft, penaltyAreaTopRight, penaltyAreaBottomLeft, penaltyAreaBottomRight,
  	goalBoxTopLeft, goalBoxTopRight, goalBoxBottomLeft, goalBoxBottomRight,
      cornerLeft, cornerRight;
  // Retrieve the field dimensions in meters
  float fieldLength = field_dimensions_->fieldLength;
  float fieldWidth = field_dimensions_->fieldWidth;
  float penaltyAreaLength = field_dimensions_->fieldPenaltyAreaLength;
  float penaltyAreaWidth = field_dimensions_->fieldPenaltyAreaWidth;
  float goalBoxLength = field_dimensions_->fieldGoalBoxLength;
  float goalBoxWidth = field_dimensions_->fieldGoalBoxWidth;

  // Calculate positions of the penalty area corner points first
  // Top left penalty area point
  penaltyAreaTopLeft.x() = fieldLength / 2;
  penaltyAreaTopLeft.y() = penaltyAreaWidth / 2;
  // Top right penalty area point
  penaltyAreaTopRight.x() = fieldLength / 2;
	penaltyAreaTopRight.y() = -penaltyAreaWidth / 2;
  // bottom left penalty area point
	penaltyAreaBottomLeft.x() = fieldLength / 2 - penaltyAreaLength;
	penaltyAreaBottomLeft.y() = penaltyAreaWidth / 2;
  // bottom right penalty area point
	penaltyAreaBottomRight.x() = fieldLength / 2 - penaltyAreaLength;
	penaltyAreaBottomRight.y() = -penaltyAreaWidth / 2;
	// Calculate positions of the goal box corner points first
	// Top left penalty area point
	goalBoxTopLeft.x() = fieldLength / 2;
	goalBoxTopLeft.y() = goalBoxWidth / 2;
	// Top right penalty area point
	goalBoxTopRight.x() = fieldLength / 2;
	goalBoxTopRight.y() = -goalBoxWidth / 2;
	// bottom left penalty area point
	goalBoxBottomLeft.x() = fieldLength / 2 - goalBoxLength;
	goalBoxBottomLeft.y() = goalBoxWidth / 2;
	// bottom right penalty area point
	goalBoxBottomRight.x() = fieldLength / 2 - goalBoxLength;
	goalBoxBottomRight.y() = -goalBoxWidth / 2;
  // Calculate positions of the field corners
  // Top left field corner
	cornerLeft.x() = fieldLength / 2;
	cornerLeft.y() = fieldWidth / 2;
  // Top right field corner
	cornerRight.x() = fieldLength / 2;
	cornerRight.y() = -fieldWidth / 2;

  // Get the pixel positions of the points on the 2D camera image
  Vector2i ptl, ptr, pbl, pbr, gtl, gtr, gbl, gbr, cl, cr;
  // Check if all projection points lie outside of the image frame.
  if (!camera_matrix_->robotToPixel(penaltyAreaTopLeft, ptl) ||
      !camera_matrix_->robotToPixel(penaltyAreaTopRight, ptr) ||
      !camera_matrix_->robotToPixel(penaltyAreaBottomLeft, pbl) ||
      !camera_matrix_->robotToPixel(penaltyAreaBottomRight, pbr) ||
		  !camera_matrix_->robotToPixel(goalBoxTopLeft, gtl) ||
		  !camera_matrix_->robotToPixel(goalBoxTopRight, gtr) ||
		  !camera_matrix_->robotToPixel(goalBoxBottomLeft, gbl) ||
		  !camera_matrix_->robotToPixel(goalBoxBottomRight, gbr) ||
      !camera_matrix_->robotToPixel(cornerLeft, cl) ||
      !camera_matrix_->robotToPixel(cornerRight, cr))
  {
    Log(LogLevel::WARNING) << "The penalty area projection is outside of the observable image!";
    return;
  }

  ptl = image_data_->image422.get444From422Vector(ptl);
  ptr = image_data_->image422.get444From422Vector(ptr);
  pbl = image_data_->image422.get444From422Vector(pbl);
  pbr = image_data_->image422.get444From422Vector(pbr);
	gtl = image_data_->image422.get444From422Vector(gtl);
	gtr = image_data_->image422.get444From422Vector(gtr);
	gbl = image_data_->image422.get444From422Vector(gbl);
	gbr = image_data_->image422.get444From422Vector(gbr);
  cl = image_data_->image422.get444From422Vector(cl);
  cr = image_data_->image422.get444From422Vector(cr);

  // Draw lines for the penalty area on the camera image.
  calibImage_.cross((ptl + ptr) / 2, 8, Color::RED); // middle of penalty area line.
  calibImage_.cross((pbl + pbr) / 2, 8, Color::RED); // middle of penalty area line.
  calibImage_.cross(ptl, 8, Color::RED);
  calibImage_.cross(ptr, 8, Color::RED);
  calibImage_.cross(pbl, 8, Color::RED);
  calibImage_.cross(pbr, 8, Color::RED);
  calibImage_.line(ptl, ptr, Color::PINK);
  calibImage_.line(pbl, pbr, Color::PINK);
  calibImage_.line(pbl, ptl, Color::PINK);
  calibImage_.line(pbr, ptr, Color::PINK);
	// Draw lines for the goal box on the camera image.
	calibImage_.cross((gtl + gtr) / 2, 8, Color::RED); // middle of goal box line.
	calibImage_.cross((gbl + gbr) / 2, 8, Color::RED); // middle of goal box line.
	calibImage_.cross(gtl, 8, Color::RED);
	calibImage_.cross(gtr, 8, Color::RED);
	calibImage_.cross(gbl, 8, Color::RED);
	calibImage_.cross(gbr, 8, Color::RED);
	calibImage_.line(gtl, gtr, Color::PINK);
	calibImage_.line(gbl, gbr, Color::PINK);
	calibImage_.line(gbl, gtl, Color::PINK);
	calibImage_.line(gbr, gtr, Color::PINK);
  // Draw the line between the field corners and mark them with crosses
  calibImage_.line(cl, cr, Color::PINK);
  calibImage_.cross(cl, 8, Color::RED);
  calibImage_.cross(cr, 8, Color::RED);
}

void CameraCalibration::projectBottomCamera() {
	Vector2f goalBoxTopLeft, goalBoxTopRight, penaltySpot;

	// Retrieve the field dimensions in meters
	float goalBoxLength = field_dimensions_->fieldGoalBoxLength;
	float goalBoxWidth = field_dimensions_->fieldGoalBoxWidth;
	float penaltySpotDistance = field_dimensions_->fieldPenaltyMarkerDistance;

	goalBoxTopLeft.x() = goalBoxLength;
	goalBoxTopLeft.y() = goalBoxWidth;

	goalBoxTopRight.x() = goalBoxLength;
	goalBoxTopRight.y() = -goalBoxWidth;

	penaltySpot.x() = penaltySpotDistance;
	penaltySpot.y() = 0;

	Vector2i gtl, gtr, ps;
	if (!camera_matrix_->robotToPixel(goalBoxTopLeft, gtl) ||
		!camera_matrix_->robotToPixel(goalBoxTopRight, gtr) ||
		!camera_matrix_->robotToPixel(penaltySpot, ps))
	{
		Log(LogLevel::WARNING) << "The projection is outside of the observable image!";
		return;
	}

	gtl = image_data_->image422.get444From422Vector(gtl);
	gtr = image_data_->image422.get444From422Vector(gtr);
	ps = image_data_->image422.get444From422Vector(ps);

	calibImage_.cross(gtl, 8, Color::RED);
	calibImage_.cross(gtr, 8, Color::RED);
	calibImage_.line(gtl, gtr, Color::PINK);

	calibImage_.cross(ps, 16, Color::RED);
}

void CameraCalibration::sendImageForCalibration() {
	debug().sendImage(mount_ + "." + image_data_->identification + "_penalty_project_image",
					  calibImage_);
}
