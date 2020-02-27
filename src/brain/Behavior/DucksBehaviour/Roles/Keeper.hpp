#pragma once

DucksActionCommand roleKeeper(const DuckDataSet &d)
{
	return DucksActionCommand::stand().invalidate();
}