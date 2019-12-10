/**
 * @file LEDHandler.hpp
 * @brief File providing handler for LEDs
 * @author <a href="mailto:oliver.tretau@tuhh.de">Oliver Tretau</a>
 *
 * This file should be used whenever some LED are addressed. Never try to access
 * a LED by using your bare hands (e.g. calling the DcmConnector).
 *
 * Further information on NAOs hardware can be found <a href="http://doc.aldebaran.com/2-1/family/nao_h25/index_h25.html">here</a>.
 */

#pragma once

#include <array>
#include <vector>
#include <Data/LEDRequest.hpp>

#include "Data/CycleInfo.hpp"

#include "Definitions/keys.h"
#include "Framework/Module.hpp"


class Motion;

class LEDHandler : public Module<LEDHandler, Motion>
{
public:
  /// the name of this module
  ModuleName name = "LEDHandler";
  explicit LEDHandler(const ModuleManagerInterface& manager);
  void cycle() override;


private:

  void setChestLEDs(float red, float green, float blue);

  void setEyeLeftLEDsColor(float red, float green, float blue);

  void setEyeRightLEDsColor(float red, float green, float blue);

  void setEarLeftLEDs(const float earSegmentBrightnesses[keys::led::EAR_MAX]);

  void setEarRightLEDs(const float earSegmentBrightnesses[keys::led::EAR_MAX]);

  void setEyeLeftRainbow();

  void setEyeRightRainbow();

  void setRightEarBrightness(float brightness);

  void setRightEarContinueLoading();

  void setRightEarProgress(short progress);

  void resetRightEarPulsating(uint8_t right);

  void setRightEarPulsating(uint8_t speed);

  void setLeftEarBrightness(float brightness);

  void setLeftEarContinueLoading();

  void setLeftEarProgress(short progress);

  void setLeftEarPulsating(uint8_t speed);

  void resetLeftEarPulsating(uint8_t right);

  void setChestRainbowColors();

  void setFootLeftLEDs(float red, float green, float blue);

  void setFootRightLEDs(float red, float green, float blue);

  void setFootRightRainbowColors();

  void setFootLeftRainbowColors();

  const Dependency<CycleInfo> cycleInfo_;
  const Dependency<LEDRequest> ledRequest_;
  std::vector<float> cmd_;
  unsigned int cycleCount_, rainbowCycle_;
  uint8_t currentSpeedRight;
  unsigned int lastStartTimeRight;
  bool isIncreasingHalfCycleRight;
  uint8_t loaderRightLength;
  uint8_t loadPosRightEnd;
  unsigned int lastLoadRightTime;
  std::vector<float> lastLoadingRightEar;
  uint8_t currentSpeedLeft;
  unsigned int lastStartTimeLeft;
  bool isIncreasingHalfCycleLeft;
  uint8_t loaderLeftLength;
  uint8_t loadPosLeftEnd;
  unsigned int lastLoadLeftTime;
  std::vector<float> lastLoadingLeftEar;
  unsigned int lastStartTimeChest;
  const float diff = 0.1f;
  float chestRainbowRed;
  float chestRainbowGreen;
  float chestRainbowBlue;
  unsigned int lastStartTimeRightFoot;
  float rightFootRainbowRed;
  float rightFootRainbowGreen;
  float rightFootRainbowBlue;
  unsigned int lastStartTimeLeftFoot;
  float leftFootRainbowRed;
  float leftFootRainbowGreen;
  double leftFootRainbowBlue;
};
