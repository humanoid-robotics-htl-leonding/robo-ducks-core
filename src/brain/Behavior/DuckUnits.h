//
// Created by obyoxar on 26/11/2019.
//
#pragma once

#include "ActionCommand.hpp"
#include "DataSet.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "DucksBehaviour/States/Started.hpp"
#include "DucksBehaviour/States/Initial.hpp"
#include "DucksBehaviour/States/Penalized.hpp"
#include "DucksBehaviour/States/Playing.hpp"
#include "DucksBehaviour/States/Ready.hpp"
#include "DucksBehaviour/States/Set.hpp"
#include "DucksBehaviour/States/State.hpp"
#include "DucksBehaviour/RootBehaviour.hpp"

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined __GNUC__
#pragma GCC diagnostic pop
#endif