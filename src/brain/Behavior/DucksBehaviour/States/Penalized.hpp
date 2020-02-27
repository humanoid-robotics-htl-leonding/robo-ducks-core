#pragma once

DucksActionCommand penalized(const DucksDataSet &d)
{
	return DucksActionCommand::kneel().combineLeftLED(DucksActionCommand::EyeLED::colors(1.0, 0.0, 0.0));
}