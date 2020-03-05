#pragma once

#include <mutex>

#include "Hardware/CameraInterface.hpp"

class QtWebsocketCamera : public CameraInterface {
public:
  /**
   */
  QtWebsocketCamera();
  /**
   * @brief setImage sets the data that will be returned in subsequent calls to readImage
   * @param image an image
   * @param timestamp the timestamp of the image
   */
  void setImage(const Image422& image, const TimePoint timestamp);
  /**
   * @brief waitForImage waits until there is a new image available to be processed
   * @return the number of seconds that have been waited
   */
  float waitForImage();
  /**
   * @brief readImage copies the next image
   * @param image is filled with the new image
   * @return the time point at which the first pixel of the image was recorded
   */
  TimePoint readImage(Image422& image);
  /**
   * @brief releaseImage is used to possible release the image of a camera
   */
  void releaseImage();
  /**
   * @brief startCapture starts capturing images
   */
  void startCapture();
  /**
   * @brief stopCapture stops capturing images
   */
  void stopCapture();
  /**
   * @brief getCamera queries if it represents a TOP or BOTTOM camera
   * @return the camera type
   */
  Camera getCameraType();
private:
  /// the current image that the camera would return
  Image422 image_;
  /// the timestamp of the current image
  TimePoint timestamp_;
  /// lock to prevent races between setImageData and readImage
  std::mutex lock_;
};
