//
// Created by Finn Poppinga on 04.05.16.
//

#include "AudioPlayer.hpp"
#include "print.h"

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
  if (!playbackData_->samples.empty())
  {
    audioInterface_.playbackAudioData(playbackData_->samples);
  }else if(audioRequest_->frequency != 0){
    //44100 Hz
    float cycleTime = cycleInfo_->cycleTime; //s
    float frequency = audioRequest_->frequency; //Hz
    float periodDuration = 1.f/frequency; //s
    int sampleCount = (int)(cycleTime*44100.f);
    std::vector<float> samples;
    float time;
    for(int i = 0; i <= sampleCount; i++){
      time = ((float)i)/44100.f;
      samples.push_back(sin(time/periodDuration*M_PI*2));
    }
//    print("Generated Samples: ", samples.size(), LogLevel::INFO);
//    print("CycleTime: ", cycleTime, LogLevel::INFO);
//    print("Periodduration: ", periodDuration, LogLevel::INFO);
//    print("Frequency: ", frequency, LogLevel::INFO);
//    print("Last Time: ", time, LogLevel::INFO);
    audioInterface_.playbackAudioData(samples);
  }
}
