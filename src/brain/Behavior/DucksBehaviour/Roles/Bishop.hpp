#pragma once

DucksActionCommand roleBishop(const DuckDataSet &d)
{
	return DucksActionCommand::kneel().invalidate();
}