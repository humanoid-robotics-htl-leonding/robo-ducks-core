//
// Created by max on 3/9/20.
//

#include "Framework/DataType.hpp"
#include "Tools/Math/Pose.hpp"
#include "Tools/Math/Rectangle.hpp"

class DucksBishopAction: public DataType<DucksBishopAction>
{
public:
    /// the name of this DataType
    DataTypeName name = "DucksBishopAction";
    /// whether the playing position is valid

    enum Type
    {
        PATROL_AREA,
        EVADE_LIBERATION_STRIKE,
        GO_TO_BALL_POSITION,
        DRIBBLE_TO_KICK_LOCATION,
        PASS,
        STRIKE,
        SHADOW_BALL
    };

    bool valid = false;
    Type type = Type::PATROL_AREA;
    Pose targetPose;
    Vector2f kickTarget;
    /**
     * @brief invalidates the position
     */
    void reset()
    {
        valid = false;
    }

    virtual void toValue(Uni::Value &value) const
    {
        value = Uni::Value(Uni::ValueType::OBJECT);
        value["valid"] << valid;
        value["type"] << static_cast<int>(type);
        value["targetPose"]<<targetPose;
        value["kickTarget"] <<kickTarget;

    }

    virtual void fromValue(const Uni::Value &value)
    {
        value["valid"] >> valid;
        int readNumber = 0;
        value["type"] >> readNumber;
        type = static_cast<Type>(readNumber);
        value["targetPose"] >> targetPose;
        value["kickTarget"] >> kickTarget;

    }
};
