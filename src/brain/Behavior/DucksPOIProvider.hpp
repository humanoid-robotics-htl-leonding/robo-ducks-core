#pragma once

#include <Brain.hpp>
#include <Framework/Module.hpp>

#include <Data/DucksPOI.hpp>
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
	void gatherPOIs();
	void votePOIs();
	void updateMostInterestingPOI(const DucksPOI& newMostInterestingPOI);

	Production<DucksPOI> interestingPOI_;

	std::vector<DucksPOI> pois_;
};
