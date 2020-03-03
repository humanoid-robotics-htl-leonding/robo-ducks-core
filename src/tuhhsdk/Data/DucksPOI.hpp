#pragma once

/***
 * @author Erik Mayrhofer
 */
class DucksPOI : public DataType<DucksPOI> {
public:
	DataTypeName name = "DucksPOI";

	enum Type {
		INVALID = 0,
		BALL_CANDIDATE = 1,
		NUM_TYPES
	};

	Vector2f position;
	Type type;
	float score;

	void reset() override
	{
		position = {0, 0};
		type = Type::INVALID;
		score = 0;
	}
	void toValue(Uni::Value &value) const override
	{
		value = Uni::Value(Uni::ValueType::OBJECT);
		value["position"] << position;
		value["type"] << static_cast<int>(type);
		value["score"] << score;
	}
	void fromValue(const Uni::Value &value) override
	{
		int temp;
		value["position"] >> position;
		value["type"] >> temp; type = static_cast<Type>(temp);
		value["score"] >> score;
	}
};