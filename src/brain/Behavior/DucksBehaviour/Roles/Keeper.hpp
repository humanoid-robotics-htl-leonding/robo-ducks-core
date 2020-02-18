#pragma once

DucksActionCommand roleKeeper(const DucksDataSet &d)
{
	return DucksActionCommand::stand().invalidate();
}