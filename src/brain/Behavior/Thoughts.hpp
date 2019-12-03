//
// Created by obyoxar on 03/12/2019.
//

#pragma once

#include <Data/GameControllerState.hpp>

#include "DataSet.hpp"

class Thoughts {
public:
    explicit Thoughts();
    void update(DataSet& d);

    bool handleNewState();

    TimePoint stateSince;
private:
    bool isStateNew;
    GameState gameState;
};


