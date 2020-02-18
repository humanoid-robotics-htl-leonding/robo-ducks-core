#pragma once

DucksActionCommand roleSupportStriker(const DucksDataSet &d)
{
	return DucksActionCommand::kneel().invalidate();
}