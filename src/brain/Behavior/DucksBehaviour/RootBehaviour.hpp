//
// Created by obyoxar on 26/11/2019.
//
#pragma once

DucksActionCommand rootBehavior(const DucksDataSet &d)
{
	DucksActionCommand state = chooseState(d);
	return state;
}