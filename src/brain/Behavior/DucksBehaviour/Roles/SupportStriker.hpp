#pragma once

DucksActionCommand roleSupportStriker(const DucksDataSet &d)
{
	return DucksActionCommand::stand().invalidate();
}