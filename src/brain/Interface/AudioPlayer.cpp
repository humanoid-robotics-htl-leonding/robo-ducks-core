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
  std::cout << "Frequency: " << audioRequest_->frequency << std::endl;
  if (!playbackData_->samples.empty())
  {
    audioInterface_.playbackAudioData(playbackData_->samples);
  }else if(audioRequest_->frequency != 0){


    float frequency = audioRequest_->frequency;
    double cycleStartTime = ((double) cycleInfo_->startTime)/1000.0; //s

    endTime_ = cycleStartTime + 0.1; //s

    if(lastFrameWritten_ < cycleStartTime){
      lastFrameWritten_ = cycleStartTime;
    }
//    double timeToWrite = endTime_ - lastFrameWritten_;

    std::vector<float> samples;
    for(int i = 0; lastFrameWritten_ < endTime_; i++){
      double timeIisPlayed = lastFrameWritten_ + 1/44100.f; //s
      double value = sin(timeIisPlayed*frequency*2*M_PI);
      samples.push_back(value);
//      std::cout << value << ";" << timeIisPlayed << std::endl;
      lastFrameWritten_ = timeIisPlayed; //s
    }
    audioInterface_.playbackAudioData(samples);


    /*

    //44100 Hz

    double actualSampleStartTime = std::max(timeOfLastSample_, cycleStartTime); //s

  /// ---------------|----------------|----------------|----------------|----------------|----------------|
  /// =====SAMPLE=======I[------------][---]
//    print("CycleStartTime: ", cycleStartTime, LogLevel::INFO);
//    print("ActualSampleStartTime: ", actualSampleStartTime, LogLevel::INFO);

    assert(actualSampleStartTime >= cycleStartTime);

    double cycleTime = cycleInfo_->cycleTime; //s
    double actualSampleTime = cycleTime - (actualSampleStartTime - cycleStartTime); //s

    assert(actualSampleTime > 0);

    float frequency = audioRequest_->frequency; //Hz


//    int sampleCount = (int)std::ceil(((actualSampleTime + cycleTime*0.5)*44100.f)); //

//    print("SampleCount: ", sampleCount, LogLevel::INFO);
//    print("CycleTime: ", cycleTime, LogLevel::INFO);
//    print("ActualSampleTime: ", actualSampleTime, LogLevel::INFO);
//    print("LastIPlayed: ", timeOfLastSample_, LogLevel::INFO);
//    print("=====", LogLevel::INFO);

    double timeIisPlayed = 0; //s
    double sampleEndTime = actualSampleStartTime + actualSampleTime;
    double cycleEndTime = cycleStartTime + cycleTime;
    assert(sampleEndTime >= cycleEndTime);
    assert(sampleEndTime < cycleEndTime + cycleTime);

    std::cout << 3 << ";" << actualSampleStartTime << std::endl;

    for(int i = 0; timeIisPlayed < sampleEndTime; i++) {
      timeIisPlayed = actualSampleStartTime + (double)i * 1/44100.f; //s
      double value = sin(timeIisPlayed*frequency*2*M_PI);
      samples.push_back(value);
      std::cout << value << ";" << timeIisPlayed << std::endl;
    }

    timeOfLastSample_ = timeIisPlayed; //s
    std::cout << 2 << ";" << timeOfLastSample_ << std::endl;

//    print("Generated Samples: ", samples.size(), LogLevel::INFO);
//    print("CycleTime: ", cycleTime, LogLevel::INFO);
//    print("Periodduration: ", periodDuration, LogLevel::INFO);
//    print("Frequency: ", frequency, LogLevel::INFO);
//    print("Last Time: ", time, LogLevel::INFO);
      audioInterface_.stopPlayback();
      audioInterface_.playbackAudioData(samples);
      audioInterface_.startPlayback();*/
  }
}
