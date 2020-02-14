#pragma once

DucksActionCommand roleNone(const DuckDataSet &d)
{
	return DucksActionCommand::kneel().invalidate();
}