#pragma once

DucksActionCommand penalized(const DucksDataSet &d)
{
	return DucksActionCommand::penalized().combineLeftLED(DucksActionCommand::EyeLED::colors(1.0, 0.0, 0.0));
}