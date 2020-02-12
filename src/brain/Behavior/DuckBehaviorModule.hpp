#pragma once

#include <mutex>
#include <Data/HeadOffData.hpp>
#include <Brain.hpp>
#include "Data/BallSearchPosition.hpp"
#include "Data/BallState.hpp"
#include "Data/BishopPosition.hpp"
#include "Data/BodyPose.hpp"
#include "Data/ButtonData.hpp"
#include "Data/CycleInfo.hpp"
#include "Data/DefendingPosition.hpp"
#include "Data/FieldDimensions.hpp"
#include "Data/GameControllerState.hpp"
#include "Data/HeadMotionOutput.hpp"
#include "Data/HeadPositionData.hpp"
#include "Data/KeeperAction.hpp"
#include "Data/MotionRequest.hpp"
#include "Data/MotionState.hpp"
#include "Data/PenaltyKeeperAction.hpp"
#include "Data/PenaltyStrikerAction.hpp"
#include "Data/PlayerConfiguration.hpp"
#include "Data/PlayingRoles.hpp"
#include "Data/RobotPosition.hpp"
#include "Data/SetPosition.hpp"
#include "Data/StrikerAction.hpp"
#include "Data/SupportingPosition.hpp"
#include "Data/ReplacementKeeperAction.hpp"
#include "Data/TeamBallModel.hpp"
#include "Data/TeamPlayers.hpp"
#include "Data/WorldState.hpp"
#include "Data/AudioData.hpp"
#include "Data/ThoughtControlRequest.hpp"
#include "Data/LEDRequest.hpp"
#include "Framework/Module.hpp"
#include "Thoughts.hpp"
#include "DuckDataSet.hpp"


class DuckBehaviorModule: public Module<DuckBehaviorModule, Brain>
{
public:
	/// the name of this module
	ModuleName name = "DuckBehaviorModule";

	/**
	 * @brief BehaviorModule initializes members
	 * @param manager a reference to brain
	 */
	explicit DuckBehaviorModule(const ModuleManagerInterface &manager);

	/**
	 * @brief cycle executes the behavior
	 */
	void cycle() override;

private:
	/// mutex that locks the actual remote motion request
	std::mutex actualRemoteMotionRequestLock_;
	/// the remote motion request (may be changed by other threads)
	Parameter<MotionRequest> remoteMotionRequest_;
	/// whether the remote motion request shall be used (can only be activated in INITIAL)
	Parameter<bool> useRemoteMotionRequest_;

	const Dependency<GameControllerState> gameControllerState_;
	const Dependency<BallState> ballState_;
	const Dependency<RobotPosition> robotPosition_;
	const Dependency<BodyPose> bodyPose_;
	const Dependency<PlayerConfiguration> playerConfiguration_;
	const Dependency<PlayingRoles> playingRoles_;
	const Dependency<MotionState> motionState_;
	const Dependency<HeadPositionData> headPositionData_;
	const Dependency<HeadMotionOutput> headMotionOutput_;
	const Dependency<TeamBallModel> teamBallModel_;
	const Dependency<TeamPlayers> teamPlayers_;
	const Dependency<DuckBallSearchPosition> ballSearchPosition_;
	const Dependency<FieldDimensions> fieldDimensions_;
	const Dependency<StrikerAction> strikerAction_;
	const Dependency<PenaltyStrikerAction> penaltyStrikerAction_;
	const Dependency<KickConfigurationData> kickConfigurationData_;
	const Dependency<KeeperAction> keeperAction_;
	const Dependency<PenaltyKeeperAction> penaltyKeeperAction_;
	const Dependency<CycleInfo> cycleInfo_;
	const Dependency<SetPosition> setPosition_;
	const Dependency<DefendingPosition> defendingPosition_;
	const Dependency<BishopPosition> bishopPosition_;
	const Dependency<SupportingPosition> supportingPosition_;
	const Dependency<ReplacementKeeperAction> replacementKeeperAction_;
	const Dependency<ButtonData> buttonData_;
	const Dependency<WorldState> worldState_;
	const Dependency<HeadOffData> headOffData_;

	Production<MotionRequest> motionRequest_;
	Production<AudioRequest> audioRequest_;
	Production<PlaybackData<AudioInterface::numChannels>> playbackData_;
	Production<LEDRequest> ledRequest_;
	Production<ThoughtControlRequest> thoughtControlRequest_;

	/// the last action command that was computed by the behavior
	ActionCommand actionCommand_;
	/// Thoughts
	Thoughts thoughts_;
	/// the data set/bundle that is passed to the behavior
	DuckDataSet dataSet_;
	/// a thread-safe copy of the remote motion request
	MotionRequest actualRemoteMotionRequest_;
};
