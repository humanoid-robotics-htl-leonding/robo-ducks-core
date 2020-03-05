#pragma once

#include <Brain.hpp>
#include <Framework/Module.hpp>

#include <Data/DucksPOI.hpp>
#include <Data/TeamBallModel.hpp>
#include <Data/RobotPosition.hpp>
#include <Data/Desperation.hpp>
#include <Data/BallSearchMap.hpp>
/***
 * @author Erik Mayrhofer
 */
class DucksPOIProvider: public Module<DucksPOIProvider, Brain>
{
public:
	ModuleName name = "DucksPOIProvider";

	explicit DucksPOIProvider(const ModuleManagerInterface& manager);

	void cycle() override;
private:

	struct POICooldown : public Uni::To{
		POICooldown(const DucksPOI& poi, const TimePoint& doneAt) : Uni::To(), poi(poi), doneAt(doneAt){

		}

		DucksPOI poi;
		TimePoint doneAt;

		void toValue(Uni::Value &value) const override
		{
			value = Uni::Value(Uni::ValueType::OBJECT);
			value["poi"] << poi;
			value["doneAt"] << doneAt;
		}
	};

	void gatherPOIs();
	void votePOIs();
	void updateMostInterestingPOI(const DucksPOI& newMostInterestingPOI);
	void proposePosition(const Vector2f& position, DucksPOI::Type type, float proposedScore);
	bool liesWithinCooldown(const Vector2f& position);

	const Dependency<TeamBallModel> teamBallModel_;
	const Dependency<Desperation> desperation_;
	const Dependency<RobotPosition> robotPosition_;
	const Dependency<BallSearchMap> ballSearchMap_;

	Production<DucksPOI> interestingPOI_;

	Vector2f oldPOIPosition_;
	TimePoint startedLookingAtPOI_;

	std::vector<DucksPOI> pois_;



	DucksPOI currentPOI_;
	TimePoint currentPOIDeath_;

	std::vector<POICooldown> cooldowns_;

};
