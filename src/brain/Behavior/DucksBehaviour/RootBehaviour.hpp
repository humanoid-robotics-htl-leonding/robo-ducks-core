//
// Created by obyoxar on 26/11/2019.
//
#pragma once

ActionCommand rootBehavior(const DuckDataSet &d)
{
    DucksActionCommand state = chooseState(d);
    return state;
}