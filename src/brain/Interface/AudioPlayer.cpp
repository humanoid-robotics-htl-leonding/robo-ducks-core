//
// Created by Finn Poppinga on 04.05.16.
//

#include "AudioPlayer.hpp"
#include "print.h"

#define SPEAKER_RESOLUTION 44100.0

AudioPlayer::AudioPlayer(const ModuleManagerInterface& manager)
  : Module(manager)
  , audioInterface_(robotInterface().getAudio())
  , playbackData_(*this)
  , audioRequest_(*this)
  , cycleInfo_(*this)
{
  audioInterface_.startPlayback();
}

AudioPlayer::~AudioPlayer()
{
  audioInterface_.stopPlayback();
}

void AudioPlayer::cycle()
{
  if (!playbackData_->samples[0].empty())
  {
    audioInterface_.playbackAudioData(playbackData_->samples[0]);
  }else if(audioRequest_->frequency != 0){


    float frequency = audioRequest_->frequency;
    double cycleStartTime = ((double) cycleInfo_->startTime)/1000.0; //s

    endTime_ = cycleStartTime + 0.1; //s

    if(lastFrameWritten_ < cycleStartTime){
      lastFrameWritten_ = cycleStartTime;
    }

    std::vector<float> samples;
    for(int i = 0; lastFrameWritten_ < endTime_; i++){
      double timeIisPlayed = lastFrameWritten_ + 1/SPEAKER_RESOLUTION; //s
      double value = sin(timeIisPlayed*frequency*2*M_PI);
      samples.push_back(value);
      lastFrameWritten_ = timeIisPlayed; //s
    }
    audioInterface_.playbackAudioData(samples);
  }
}
