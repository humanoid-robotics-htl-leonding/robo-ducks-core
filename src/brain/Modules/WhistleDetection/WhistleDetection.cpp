#include "WhistleDetection.hpp"
#include "Tools/Chronometer.hpp"
#include "Tools/Math/Statistics.hpp"
#include "print.h"

WhistleDetection::WhistleDetection(const ModuleManagerInterface &manager)
	: Module(manager),
	  recordData_(*this),
	  rawGameControllerState_(*this),
	  cycleInfo_(*this),
	  whistleData_(*this),
	  minFrequency_(*this, "minFrequency", []
	  {}),
	  maxFrequency_(*this, "maxFrequency", []
	  {}),
	  backgroundScaling_(*this, "backgroundScaling", []
	  {}),
	  whistleScaling_(*this, "whistleScaling", []
	  {}),
	  numberOfBands_(*this, "numberOfBands", []
	  {}),
	  minWhistleCount_(*this, "minWhistleCount", []
	  {}),
	  fft_(fftBufferSize_),
	  lastTimeWhistleHeard_(),
	  foundWhistlesBuffer_(foundWhistlesBufferSize_, false)
{
}

void WhistleDetection::cycle()
{
	Chronometer time(debug(), mount_ + ".cycle_time");

	if (rawGameControllerState_->gameState != GameState::SET) {
		return;
	}

	for (unsigned long channelIndex = 0; channelIndex < recordData_->samples.size(); channelIndex++) {
		auto channel = recordData_->samples[channelIndex];
		if (channel.empty()) {
			continue;
		}

		for (auto &sample : channel) //Todo Proper Channel Handling here
		{
			fftBuffer_.push_back(sample);
			if (fftBuffer_.size() == fftBufferSize_) {
				// check current fft buffer for whistle
				foundWhistlesBuffer_.push_back(fftBufferContainsWhistle());
				// count the number of found whistles in the whistle buffer
				unsigned int whistleCount = 0;
				for (bool i : foundWhistlesBuffer_) {
					whistleCount += i;
				}
				debug().update(mount_ + ".whistles_heard_" + std::to_string(channelIndex), whistleCount);
				debug().update(mount_ + ".lastTimeWhistleHeard_" + std::to_string(channelIndex), lastTimeWhistleHeard_);
				// a whistle is reported if the whistle buffer contains at least a certain number of found whistles
				if (whistleCount >= minWhistleCount_()) {
					print("Whistle Heard!", LogLevel::INFO);
					lastTimeWhistleHeard_ = cycleInfo_->startTime;
					break;
				}
				fftBuffer_.clear();
				break;
			}
		}
	}
	whistleData_->lastTimeWhistleHeard = lastTimeWhistleHeard_;
}

bool WhistleDetection::fftBufferContainsWhistle()
{
	if (fftBuffer_.empty())
		return false;

	// apply "Hann window" to reduce spectral leakage
	for (unsigned int i = 0; i < fftBufferSize_; i++) {
		fftBuffer_[i] *= std::pow(std::sin(static_cast<float>(M_PI) * static_cast<float>(i) / fftBufferSize_), 2.0f);
	}
	// perform the fft
	auto freqData = fft_.fft(fftBuffer_);

	// the indices corresponding to the whistle band are computed by dividing by the frequency resolution
	double freqResolution = samplingRate / fftBufferSize_;
	unsigned int minFreqIndex = ceil(minFrequency_() / freqResolution);
	unsigned int maxFreqIndex = ceil(maxFrequency_() / freqResolution);

	if (maxFreqIndex > fftBufferSize_) {
		throw std::runtime_error("WhistleDetection: maxFrequency can not be higher than nyquist frequency.");
	}

	// the absolute values of the comlpex spectrum, the mean and the standard deviation
	std::vector<float> absFreqData(freqData.size());
	for (unsigned int i = 0; i < freqData.size(); i++) {
		absFreqData[i] = std::abs(freqData[i]);
	}
	const float mean = Statistics::mean(absFreqData);
	const float standardDeviation = Statistics::standardDeviation(absFreqData, mean);

	// the spectrum is divided into several bands. for each band, the mean is compared to the background threshold to find the whistle band
	const float backgroundThreshold = mean + backgroundScaling_() * standardDeviation;
	const unsigned int bandSize = ceil((maxFreqIndex - minFreqIndex) / numberOfBands_());
	if (bandSize == 0) {
		throw std::runtime_error(
			"WhistleDetection: bandSize is zero. Probably the min and max index are too close together.");
	}
	// find the start of the the whistle band
	for (unsigned int i = 0; i < numberOfBands_(); i++) {
		const std::vector<float>::const_iterator bandStart = absFreqData.begin() + minFreqIndex;
		const std::vector<float>::const_iterator bandEnd = absFreqData.begin() + minFreqIndex + bandSize;
		assert(bandStart != bandEnd);
		const float bandMean = Statistics::mean(std::vector<float>(bandStart, bandEnd));
		if (bandMean < backgroundThreshold) {
			minFreqIndex += bandSize;
		}
		else {
			break;
		}
	}

	// find the end of the whistle band
	for (unsigned int i = 0; i < numberOfBands_(); i++) {
		const std::vector<float>::const_iterator bandStart = absFreqData.begin() + maxFreqIndex - bandSize;
		const std::vector<float>::const_iterator bandEnd = absFreqData.begin() + maxFreqIndex;
		assert(bandStart != bandEnd);
		const float bandMean = Statistics::mean(std::vector<float>(bandStart, bandEnd));
		if (bandMean < backgroundThreshold) {
			maxFreqIndex -= bandSize;
		}
		else {
			break;
		}
	}

	// a whistle is found in the buffer if the mean of the whistle band is significantly larger than a threshold
	if (minFreqIndex < maxFreqIndex) {
		const std::vector<float>::const_iterator bandStart = absFreqData.begin() + minFreqIndex;
		const std::vector<float>::const_iterator bandEnd = absFreqData.begin() + maxFreqIndex;
		const float whistleMean = Statistics::mean(std::vector<float>(bandStart, bandEnd));
		const float whistleThreshold = mean + whistleScaling_() * standardDeviation;
		if (whistleMean > whistleThreshold) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}
