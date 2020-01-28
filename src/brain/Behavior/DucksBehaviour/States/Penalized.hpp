#pragma once

ActionCommand penalized(const DuckDataSet &d)
{
	return ActionCommand::kneel().combineLeftLED(ActionCommand::EyeLED::colors(1.0, 0.0, 0.0));
}