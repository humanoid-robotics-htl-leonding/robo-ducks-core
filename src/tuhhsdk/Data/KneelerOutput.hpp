//
// Created by max on 12/3/19.
//
#pragma once

#include "MotionOutput.hpp"

class KneelerOutput : public DataType<KneelerOutput, MotionOutput> {
public:
    DataTypeName name = "KneelerOutput";
    bool wantToSend;
    /**
     */
    void reset()
    {
        MotionOutput::reset();
        wantToSend = false;
    }

    virtual void toValue(Uni::Value& value) const
    {
        MotionOutput::toValue(value);
        value["wantToSend"] << wantToSend;
    }

    virtual void fromValue(const Uni::Value& value)
    {
        MotionOutput::fromValue(value);
        value["wantToSend"] >> wantToSend;
    }
};
