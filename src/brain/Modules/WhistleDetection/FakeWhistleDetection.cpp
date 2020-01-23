#include "FakeWhistleDetection.hpp"
#include "Tools/Chronometer.hpp"
#include "Tools/Math/Statistics.hpp"
#include "print.h"

FakeWhistleDetection::FakeWhistleDetection(const ModuleManagerInterface &manager)
	: Module(manager),
	  whistleData_(*this)
{
}
void FakeWhistleDetection::cycle()
{
}
