#include "AudioReceiver.hpp"


AudioReceiver::AudioReceiver(const ModuleManagerInterface& manager)
  : Module(manager)
  , audioInterface_(robotInterface().getAudio())
  , recordData_(*this)
{
  audioInterface_.startCapture();
}

AudioReceiver::~AudioReceiver()
{
  audioInterface_.stopCapture();
}

void AudioReceiver::cycle()
{
//  audioInterface_.readAudioData(recordData_->samples);

/* TODO
    for (unsigned int channel = 0; channel < AudioInterface::numChannels; channel++)
    {
        audioInterface_.readAudioData(recordData_->samples[channel], static_cast<AudioInterface::Microphone>(channel));
        if (recordData_->samples[channel].size())
        {
            subsampledData_[channel].clear();
            for (std::size_t i = 0; i < recordData_->samples[channel].size(); i+=1)
            {
                subsampledData_[channel].push_back(recordData_->samples[channel][i]);
            }
            debug().update(mount_ + ".audioSamples_" + audioInterface_.microphoneNames[channel], subsampledData_[channel]);
        }
    }
    */
}
