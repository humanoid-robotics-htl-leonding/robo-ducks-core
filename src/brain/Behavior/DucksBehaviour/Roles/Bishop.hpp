#pragma once

DucksActionCommand roleBishop(const DucksDataSet &d)
{
	return DucksActionCommand::stand().invalidate();
}