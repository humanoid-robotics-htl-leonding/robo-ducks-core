//
// Created by obyoxar on 26/11/2019.
//
#pragma once

#include "ActionCommand.hpp"
#include "DuckDataSet.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

namespace ducks
{
#include "Behavior/DucksBehaviour/Utils/WalkTo.hpp"
#include "DucksBehaviour/Roles/None.hpp"
#include "DucksBehaviour/Roles/Striker.hpp"
#include "DucksBehaviour/Roles/Keeper.hpp"
#include "DucksBehaviour/Roles/Defender.hpp"
#include "DucksBehaviour/Roles/Bishop.hpp"
#include "DucksBehaviour/Roles/SupportStriker.hpp"
#include "DucksBehaviour/Roles/Roles.hpp"
#include "DucksBehaviour/States/Started.hpp"
#include "DucksBehaviour/States/Initial.hpp"
#include "DucksBehaviour/States/Penalized.hpp"
#include "DucksBehaviour/States/Playing.hpp"
#include "DucksBehaviour/States/Ready.hpp"
#include "DucksBehaviour/States/Set.hpp"
#include "DucksBehaviour/States/State.hpp"
#include "DucksBehaviour/RootBehaviour.hpp"
}

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined __GNUC__
#pragma GCC diagnostic pop
#endif