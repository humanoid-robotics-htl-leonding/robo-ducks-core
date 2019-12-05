//
// Created by obyoxar on 03/12/2019.
//

#pragma once

#include <Framework/DataType.hpp>

enum class ThoughtCommand {
    RESET_COMPASS_DIRECTION = 0,

    MAX
};

class ThoughtControlRequest : public DataType<ThoughtControlRequest> {
public:
    /// the name of this DataType
    DataTypeName name = "ThoughtControlRequest";

    bool isCommandSet(ThoughtCommand command) const{
      return commands_[static_cast<unsigned int>(command)];
    }

    void setCommand(ThoughtCommand command, bool newValue = true){
      commands_[static_cast<unsigned int>(command)] = newValue;
    }

    void apply(const std::array<bool, (int) ThoughtCommand::MAX>& commands){
      commands_ = commands;
    }

    void reset() override {
      for(auto i = commands_.begin(); i < commands_.end(); i++){
        (*i)=false;
      }
    }

    void toValue(Uni::Value &value) const override {
      value["resetCompassDirection"] << commands_[static_cast<unsigned int>(ThoughtCommand::RESET_COMPASS_DIRECTION)];
    }

    void fromValue(const Uni::Value &value) override {
      value["resetCompassDirection"] >> commands_[static_cast<unsigned int>(ThoughtCommand::RESET_COMPASS_DIRECTION)];
    }

private:
    std::array<bool, (int) ThoughtCommand::MAX> commands_{};
};