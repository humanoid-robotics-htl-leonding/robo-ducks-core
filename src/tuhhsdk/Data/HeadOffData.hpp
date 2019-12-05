//
// Created by obyoxar on 04/09/2019.
//
#pragma once

#include "Framework/DataType.hpp"

class HeadOffData : public DataType<HeadOffData> {

public:

    DataTypeName name = "HeadOffData";

    bool shouldDie = false;
    bool shouldDieSignal = false;


    void reset() {
      shouldDie = false;
    }

    virtual void toValue(Uni::Value& value) const
    {
      value = Uni::Value(Uni::ValueType::OBJECT);
      value["shouldDie"] << shouldDie;
    }

    virtual void fromValue(const Uni::Value& value)
    {
      value["shouldDie"] >> shouldDie;
    }
};
