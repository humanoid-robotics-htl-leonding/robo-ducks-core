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


    TimePoint stateSince;
private:
    GameState gameState;
};


