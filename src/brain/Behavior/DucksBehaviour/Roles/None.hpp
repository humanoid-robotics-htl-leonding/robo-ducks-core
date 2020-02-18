#pragma once

DucksActionCommand roleNone(const DucksDataSet &d)
{
	return DucksActionCommand::kneel().invalidate();
}