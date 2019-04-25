#include <cassert>
#include <mutex>

#include "Framework/Thread.hpp"

#include "print.h"

#include "SharedObject.hpp"


SharedObject::SharedObject(const std::string& name, ThreadData& tData)
  : thread_()
  , threadDatum_(tData)
{
  Log(LogLevel::DEBUG) << "New SharedObject was instantiated with name: " << name
  ThreadFactoryBase* factory;
  for (factory = ThreadFactoryBase::begin; factory != nullptr; factory = factory->next)
  {
    Log(LogLevel::DEBUG) << "Comparing: " << factory->getName();
    if (factory->getName() == name)
    {
      thread_ = factory->produce(threadDatum_);
      break;
    }
  }
  assert(factory != nullptr);
}

void SharedObject::start()
{
  thread_->start();
}

void SharedObject::stop()
{
  thread_->stop();
}

void SharedObject::join()
{
  thread_->join();
}
