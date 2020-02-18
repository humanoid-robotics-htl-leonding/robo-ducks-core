#include "MotionFilePlayer.hpp"

#include "print.hpp"


MotionFilePlayer::MotionFilePlayer(const CycleInfo& cycleInfo,
                                   const JointSensorData& jointSensorData)
  : cycleInfo_(cycleInfo)
  , jointSensorData_(jointSensorData)
  , startTime_(0)
{
}

bool MotionFilePlayer::loadFromFile(const std::string& filename)
{
  if (!MotionFile::loadFromFile(filename) || !verify())
  {
    return false;
  }
  precompile();
  return true;
}

int MotionFilePlayer::play()
{
  if (!isPlaying())
  {
    Log(LogLevel::DEBUG) << "MotionFile " << header.title << " actually playing...";
    startTime_ = cycleInfo_.startTime;

    for (unsigned int i = 0; i < header.joints.size(); i++)
    {
      startJointValues_.angles[i] = jointSensorData_.angles[header.joints[i]];
    }
    if (!stiffnesses_.empty())
    {
      // Use the stiffnesses from the first frame because interpolation of all the stiffnesses is
      // not necessary.
      startJointValues_.stiffnesses = stiffnesses_[0];
    }
    // If stiffnesses is empty, startJointValues_.stiffnesses won't be used.
    return header.time;
  }
  else
  {
    Log(LogLevel::WARNING) << "MotionFile " << header.title << " already playing! Command discarded.";
    return 0;
  }
}

void MotionFilePlayer::stop()
{
  if (isPlaying())
  {
    Log(LogLevel::DEBUG) << "MotionFile " << header.title << " stopped.";
    startTime_ -= header.time;
  }
  else
  {
    Log(LogLevel::WARNING) << "MotionFile " << header.title << " is not playing! Can not stop it.";
  }
}

MotionFilePlayer::JointValues MotionFilePlayer::cycle()
{
  JointValues result;
  unsigned int i = 0;
  float interpolationFactor;
  const int motionTime = cycleInfo_.getTimeDiff(startTime_, TDT::MILS);
  std::vector<float> last, next;
  if (angles_.empty())
  {
    Log(LogLevel::ERROR) << "MotionFile " << header.title << " does not have angles!";
    return result;
  }
  // Find the current keyframe.
  if (motionTime < angleTimes_[0])
  {
    interpolationFactor = static_cast<float>(motionTime) / angleTimes_[0];
    last = startJointValues_.angles;
    next = angles_[0];
  }
  else
  {
    // Find the first i for which the next pose time is in the future.
    for (i = 0; i < angleTimes_.size() - 1; i++)
    {
      if (angleTimes_[i + 1] > motionTime)
      {
        break;
      }
    }
    last = angles_[i];
    if (i == angleTimes_.size() - 1)
    {
      interpolationFactor = 0;
      // Set next anyway to avoid NaN or other unexpected things.
      next = angles_[i];
    }
    else
    {
      interpolationFactor =
          static_cast<float>(motionTime - angleTimes_[i]) / (angleTimes_[i + 1] - angleTimes_[i]);
      next = angles_[i + 1];
    }
  }
  for (i = 0; i < header.joints.size(); i++)
  {
    result.angles[header.joints[i]] =
        (1 - interpolationFactor) * last[i] + interpolationFactor * next[i];
  }
  // Provide a fallback for motion files that do not have stiffnesses.
  if (stiffnesses_.empty())
  {
    // Assume full stiffness any time.
    for (i = 0; i < header.joints.size(); i++)
    {
      result.stiffnesses[header.joints[i]] = 1.0f;
    }
    Log(LogLevel::ERROR) << "Motion file " << header.title << " does not have stiffnesses!";
  }
  else
  {
    // Find the current keyframe.
    if (motionTime < stiffnessTimes_[0])
    {
      interpolationFactor = static_cast<float>(motionTime) / stiffnessTimes_[0];
      last = startJointValues_.stiffnesses;
      next = stiffnesses_[0];
    }
    else
    {
      for (i = 0; i < stiffnessTimes_.size() - 1; i++)
      {
        if (stiffnessTimes_[i + 1] > motionTime)
        {
          break;
        }
      }
      last = stiffnesses_[i];
      if (i == stiffnessTimes_.size() - 1)
      {
        interpolationFactor = 0;
        next = stiffnesses_[i];
      }
      else
      {
        interpolationFactor = static_cast<float>(motionTime - stiffnessTimes_[i]) /
                              (stiffnessTimes_[i + 1] - stiffnessTimes_[i]);
        next = stiffnesses_[i + 1];
      }
    }
    for (i = 0; i < header.joints.size(); i++)
    {
      result.stiffnesses[header.joints[i]] =
          (1 - interpolationFactor) * last[i] + interpolationFactor * next[i];
    }
  }
  return result;
}

bool MotionFilePlayer::isPlaying() const
{
  return cycleInfo_.getTimeDiff(startTime_, TDT::MILS) < header.time;
}

void MotionFilePlayer::precompile()
{
  int relTimeASum = 0;
  int relTimeSSum = 0;

  angles_.clear();
  angleTimes_.clear();
  stiffnesses_.clear();
  stiffnessTimes_.clear();



  for (auto& pos : position)
  {
    relTimeASum += pos.time;
  }
  for (auto& stiff : stiffness)
  {
    relTimeSSum += stiff.time;
  }
    if(relTimeASum < header.time){
        angles_.reserve(position.size()+1);
        angleTimes_.reserve(position.size()+1);

    }
    else {
        angles_.reserve(position.size());
        angleTimes_.reserve(position.size());

    }
    if(relTimeSSum < header.time){

        stiffnesses_.reserve(stiffness.size()+1);
        stiffnessTimes_.reserve(stiffness.size()+1);
    }
    else {

        stiffnesses_.reserve(stiffness.size());
        stiffnessTimes_.reserve(stiffness.size());
    }


  int time = 0;
  for (auto& pos : position)
  {
    time += pos.time;

    angleTimes_.push_back(time);
    angles_.push_back(pos.parameters);
  }
  if(relTimeASum < header.time){
      angleTimes_.push_back(header.time);
      angles_.push_back(position.at(position.size()-1).parameters);
  }

  time = 0;
  for (auto& stiff : stiffness)
  {
      time += stiff.time;
      stiffnessTimes_.push_back(time);
    stiffnesses_.push_back(stiff.parameters);
  }
    if(relTimeSSum < header.time){
        angleTimes_.push_back(header.time);
        angles_.push_back(stiffness.at(stiffness.size()-1).parameters);
    }
}
