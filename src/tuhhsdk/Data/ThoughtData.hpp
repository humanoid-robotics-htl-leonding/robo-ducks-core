#pragma once

class ThoughtData : public DataType<ThoughtData> {
public:
    DataTypeName name = "ThoughtData";

    void reset() override {

    }

    void toValue(Uni::Value &value) const override {
      value["Yeet"] << "TODO";
    }

    void fromValue(const Uni::Value &value) override {
      value["Yeet"];
    }
};