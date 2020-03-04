#include <utility>

#define POI_KNOWN_INTEREST(innerScore) 300.f + (innerScore)
#define POI_KNOWN_NEED(innerScore) 200.f + (innerScore)
#define POI_GOOD_GUESS(innerScore) 100.f + (innerScore)
#define POI_RANDOM_GUESS(innerScore) 0.f + (innerScore)

#pragma once

/***
 * @author Erik Mayrhofer
 */
class DucksPOI : public DataType<DucksPOI> {
public:
	DataTypeName name = "DucksPOI";

//	static const float KNOWN_NEED = 300.0;
//	static const float KNOWN_INTEREST = 200.0;
//	static const float GOOD_GUESS = 100.0;
//	static const float RANDOM_GUESS = 0.0f;



	enum Type {
		INVALID = 0,
		BALL_CANDIDATE = 1,
		NUM_TYPES
	};

	enum Evaluation {
		NONE = 0,
		UNCOMFORATBLE = 1
	};

	DucksPOI(): DataType(), position(), type(), score(), evaluation() {}

	DucksPOI(Vector2f  position, const Type& type, float score, Evaluation evaluation = Evaluation::NONE)
		: DataType()
		, position(std::move(position))
		, type(type)
		, score(score)
		, evaluation(evaluation)
		{}

	Vector2f position;
	Type type;
	float score;
	Evaluation evaluation;


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
		value["evaluation"] << static_cast<int>(evaluation);
	}
	void fromValue(const Uni::Value &value) override
	{
		int temp;
		value["position"] >> position;
		value["type"] >> temp; type = static_cast<Type>(temp);
		value["score"] >> score;
		value["evaluation"] >> temp; evaluation = static_cast<Evaluation>(temp);
	}
};