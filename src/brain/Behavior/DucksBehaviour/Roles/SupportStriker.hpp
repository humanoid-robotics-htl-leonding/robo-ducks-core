#pragma once

DucksActionCommand roleSupportStriker(const DuckDataSet &d)
{
	return DucksActionCommand::kneel().invalidate();
}