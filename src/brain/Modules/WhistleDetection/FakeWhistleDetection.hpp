#pragma once

#include "Data/AudioData.hpp"
#include "Data/CycleInfo.hpp"
#include "Data/GameControllerState.hpp"
#include "Data/WhistleData.hpp"
#include "Framework/Module.hpp"
#include <Tools/Math/FFT.hpp>

#include <boost/circular_buffer.hpp>

class Brain;

/**
 * @class WhistleDetection can detect whether a whistle was whistled while listening.
 * This module will check the microphones during GameState::SET and will detect if whistle
 * was whistled.
 */
class FakeWhistleDetection : public Module<FakeWhistleDetection, Brain>
{
public:
  /// the name of this module
  ModuleName name = "FakeWhistleDetection";
  /**
   * @brief WhistleDetection initializes members
   * @param manager a reference to brain
   */
  FakeWhistleDetection(const ModuleManagerInterface& manager);
  /**
   * @brief cycle detects a whistle in the last recorded audio samples
   */
  void cycle() override;

private:
  /// information about the whistle status in the current cycle
  Production<WhistleData> whistleData_;
};
