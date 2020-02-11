#include "PoseHypothesis.hpp"
#include "Tools/Math/Angle.hpp"
#include "Tools/Math/Geometry.hpp"
#include <cmath>

PoseHypothesis::PoseHypothesis(const ModuleBase& module, const FieldDimensions& fieldDimensions,
                               const FieldInfo& fieldInfo)
  : UKFPose2D()
  , baseAssociationAngleThreshold_(module, "baseAssociationAngleThreshold",
                                   [this] { baseAssociationAngleThreshold_() *= TO_RAD; })
  , maxDistToCircleClusterSquared_(module, "maxDistToCircleClusterSquared", [] {})
  , minCircleClusterCount_(module, "minCircleClusterCount", [] {})
  , evalLowPassGain_(module, "evalLowPassGain", [] {})
  , evalAssocationFraction_(module, "evalAssocationFraction", [] {})
  , intersectionDistanceThreshold_(module, "intersectionDistanceThreshold", [] {})
  , intersectionOrientationThreshold_(module, "intersectionOrientationThreshold", [] {})
  , measurementBaseVariance_(module, "measurementBaseVariance",
                             [this] {
                               measurementBaseVariance_().z() *= TO_RAD * TO_RAD;
                               projectionMeasurementModel_.resetParameters(
                                   cameraRPYDeviation_(), measurementBaseVariance_().head<2>());
                             })
  , cameraRPYDeviation_(module, "cameraRPYDeviation",
                        [this] {
                          cameraRPYDeviation_() *= TO_RAD;
                          projectionMeasurementModel_.resetParameters(
                              cameraRPYDeviation_(), measurementBaseVariance_().head<2>());
                        })
  , projectionMeasurementModel_()
  , fieldDimensions_(fieldDimensions)
  , fieldInfo_(fieldInfo)
  , meanEvalError_(-1.f)
  , id_(0)
{
  baseAssociationAngleThreshold_() *= TO_RAD;
  measurementBaseVariance_().z() *= TO_RAD * TO_RAD;
  cameraRPYDeviation_() *= TO_RAD;
  projectionMeasurementModel_.resetParameters(cameraRPYDeviation_(),
                                              measurementBaseVariance_().head<2>());
}

void PoseHypothesis::reset(const Pose& poseMean, const Vector3f& poseCov, float error, int id = 0)
{
  UKFPose2D::reset(Vector3f(poseMean.position.x(), poseMean.position.y(), poseMean.orientation),
                   poseCov);
  meanEvalError_ = error;
  id_ = id;
}

float PoseHypothesis::getMeanEvalError() const
{
  return meanEvalError_;
}

int PoseHypothesis::getID() const
{
  return id_;
}

void PoseHypothesis::evaluate(const std::vector<Line<float>>& setOfRelativeLines)
{
  if (setOfRelativeLines.empty())
  {
    return;
  }

  // The error of the lines needs to be evaluated again, since the update reshaped the sigma
  // environment
  float totalError = 0;
  unsigned int associationCount = 0;

  // evaluate with the state mean:
  for (auto& line : setOfRelativeLines)
  {
    AssociatedLine associatedLine = findAssociatedLine(line, stateMean_, false);
    if (associatedLine.ptrToLine)
    {
      totalError += associatedLine.error;
      associationCount++;
    }
  }

  if (associationCount > 0)
  {
    const float meanEvalError =
        totalError / associationCount * (1.f - evalAssocationFraction_()) +
        (1 - associationCount / static_cast<float>(setOfRelativeLines.size())) *
            evalAssocationFraction_();
    // low pass filter for the error of this pose (this could actually be kalman-filtered as well)
    meanEvalError_ = (meanEvalError_ < 0.f) ? meanEvalError
                                            : (1.f - evalLowPassGain_()) * meanEvalError_ +
                                                  evalLowPassGain_() * meanEvalError;
  }
  else if (meanEvalError_ > 0.f)
  {
    // There were line measurements but non of them could be associated.
    // This needs to be handled sort of gently, since it could also be a false positive measurement
    meanEvalError_ = (1.f - evalLowPassGain_()) * meanEvalError_ + evalLowPassGain_() * 1.f;
  }
}

bool PoseHypothesis::isInNeighbourhood(const PoseHypothesis& other, const Vector2f& eps) const
{
  Vector3f diff = this->stateMean_ - other.stateMean_;

  float posError = ((Vector2f() << diff.x(), diff.y()).finished()).norm();
  float angleDiff = std::abs(Angle::normalizeAngleDiff(diff.z()));

  return (posError <= eps.x() && angleDiff <= eps.y());
}

void PoseHypothesis::mirror()
{
  stateMean_.x() = -stateMean_.x();
  stateMean_.y() = -stateMean_.y();
  stateMean_.z() = Angle::normalized(stateMean_.z() + M_PI);

  for (auto& sigmaPoint : sigmaPoints_)
  {
    sigmaPoint.x() = -sigmaPoint.x();
    sigmaPoint.y() = -sigmaPoint.y();
    sigmaPoint.z() = Angle::normalized(sigmaPoint.z() + M_PI);
  }
}

Eigen::Matrix3f
PoseHypothesis::computePoseCovFromFullPoseFeature(const Vector2f& relativeFeaturePosition,
                                                  const float associatedOrientation,
                                                  const KinematicMatrix& cam2ground) const
{
  // the covariance matrix in robot coordianates
  const Eigen::Matrix2f relativeXYCov = projectionMeasurementModel_.computeCovFromErrorPropagation(
      relativeFeaturePosition, cam2ground);
  // transform to world orientation of the feature
  const Eigen::Rotation2Df robot2global(associatedOrientation);
  const auto dynamicCov = robot2global * relativeXYCov * robot2global.inverse();
  // use only diagonal entries
  return Eigen::DiagonalMatrix<float, 3>(measurementBaseVariance_().x() + dynamicCov(0, 0),
                                         measurementBaseVariance_().y() + dynamicCov(1, 1),
                                         measurementBaseVariance_().z());
}

bool PoseHypothesis::computeCircleCenterFromCandidates(
    const std::vector<Vector2f>& centerPointCandidates_, Vector2f& result) const
{
  assert(centerPointCandidates_.size() > 0);
  std::vector<PointCluster2D> circleClusters;
  circleClusters.reserve(centerPointCandidates_.size());
  circleClusters.emplace_back(centerPointCandidates_[0], 1);

  for (unsigned int i = 1; i < centerPointCandidates_.size(); i++)
  {
    bool mergedWithCluster = false;
    for (auto& cluster : circleClusters)
    {
      if ((centerPointCandidates_[i] - cluster.center).squaredNorm() <
          maxDistToCircleClusterSquared_())
      {
        // add point to cluster if sufficiently near to cluster center
        cluster.center = cluster.center * cluster.clusterCount + centerPointCandidates_[i];
        cluster.center /= ++cluster.clusterCount;
        mergedWithCluster = true;
      }
    }
    // if not near enough to any cluster, open a new one
    if (!mergedWithCluster)
    {
      circleClusters.emplace_back(centerPointCandidates_[i], 1);
    }
  }

  auto bestCluster = circleClusters.begin();
  for (auto itCluster = std::next(bestCluster); itCluster != circleClusters.end(); itCluster++)
  {
    if (itCluster->clusterCount > bestCluster->clusterCount)
    {
      bestCluster = itCluster;
    }
  }

  // reason about good count threshold
  if (bestCluster->clusterCount >= (unsigned int)minCircleClusterCount_())
  {
    result = bestCluster->center;
    return true;
  }
  else
  {
    return false;
  }
}

PoseHypothesis::AssociatedLine
PoseHypothesis::findAssociatedLine(const Line<float>& relativeLine, const Vector3f& referencePose,
                                   const bool pruneByDistanceThreshold) const
{

  // calculate absolute line from relativeLine and referencePose
  Pose refPose = Pose({referencePose.x(), referencePose.y()}, referencePose.z());
  Line<float> absoluteLine(refPose * relativeLine.p1, refPose * relativeLine.p2);
  Vector2f absoluteLineVector = absoluteLine.p1 - absoluteLine.p2;
  float lineLength = absoluteLineVector.squaredNorm();

  // an associated line needs to have about the right angle
  const float badAngleThreshold = baseAssociationAngleThreshold_() + std::sqrt(stateCov_(2, 2));
  // a line is considered long if it is significantly longer then the penalty area width. The
  // penalty area width is used since here we have lines that are very close together. Lines longer
  // then the penalty area are always far away from eachother. Thus they can be associated over a
  // longer distance.
  const bool isLongLine = lineLength > std::pow(0.2f + fieldDimensions_.fieldPenaltyAreaWidth, 2);
  // an associated line should be not too far away from the projected position (fp-rejection)
  const float badDistanceThreshold = (isLongLine ? 0.25f * fieldDimensions_.fieldLength
                                                 : fieldDimensions_.fieldPenaltyAreaLength) +
                                     std::sqrt(std::max(stateCov_(0, 0), stateCov_(1, 1)));

  AssociatedLine associatedLine;
  float angle = 0.f;
  float centerPointDistance = 0.f;
  Vector2f centerOfLineSegment = (absoluteLine.p1 + absoluteLine.p2) / 2;
  for (auto& fieldLine : fieldInfo_.lines)
  {
    const float fieldLineLength = (fieldLine.p1 - fieldLine.p2).squaredNorm();
    angle = angularMetric(absoluteLine.p1 - absoluteLine.p2, fieldLine.p1 - fieldLine.p2);
    centerPointDistance = Geometry::distPointToLineSegment(fieldLine, centerOfLineSegment);
    if (angle > badAngleThreshold ||
        (centerPointDistance > badDistanceThreshold && pruneByDistanceThreshold) ||
        (lineLength > fieldLineLength && (isFieldLineAlongAxis(fieldLine) || isLongLine)))
    {
      continue;
    }
    float normalizedAngleError = angle / badAngleThreshold;
    float normalizedDistError = centerPointDistance / badDistanceThreshold;
    float thisLineError = normalizedAngleError + normalizedDistError;
    if (thisLineError < associatedLine.error)
    {
      associatedLine.error = thisLineError;
      associatedLine.ptrToLine = &fieldLine;
    }
  }

  associatedLine.error *= associatedLine.error;
  return associatedLine;
}

void PoseHypothesis::updateWithSetOfLines(const std::vector<Line<float>>& setOfRelativeLines,
                                          const KinematicMatrix& cam2ground,
                                          const bool searchCirceInLines)
{
  // Find different pose variants within this hypothesis (from explizit calculation or sigma points)
  // For now we simply associate for every sigma point

  // chose the association with the smales error for actual lineSensorUpdate
  centerPointCandidates_.clear();
  centerPointCandidates_.reserve(setOfRelativeLines.size() * 2);
  for (auto& line : setOfRelativeLines)
  {
    lineSensorUpdate(line, stateMean_, cam2ground);
  }
  // if enabled, use the center circle candidates to find the center circle
  if (searchCirceInLines)
  {
    Vector2f relativeCenterCirclePosition;
    // there are two candidates per tangent and those two should never be within the same cluster
    // thus one needs at least minCircleClousterCount_ * 2 points
    if (!(centerPointCandidates_.size() >= (unsigned int)minCircleClusterCount_() * 2))
    {
      return;
    }
    if (computeCircleCenterFromCandidates(centerPointCandidates_, relativeCenterCirclePosition))
    {
      Matrix2f cov = Matrix2f::Identity() * 1.f;
      fieldPointUpdate(relativeCenterCirclePosition, {0.f, 0.f}, cov);
    }
  }
}

void PoseHypothesis::updateWithCenterCircle(const LandmarkModel::CenterCircle& centerCircle,
                                            const KinematicMatrix& cam2ground,
                                            const bool ignoreCirclePerceptsNearGoalSupport)
{
  if (ignoreCirclePerceptsNearGoalSupport)
  {
    // where would the circle bee if projected from the pose of this hypothesis
    const Vector2f projectedCirclePosition = getPoseMean() * centerCircle.position;
    // if the circle percept is near the opponents goal when projected from the hypothesis, it will
    // be ignore
    if (std::abs(projectedCirclePosition.x()) >
            // considering the center circle diameter since the circle center might also be
            // projected behind us
            fieldDimensions_.fieldLength * 0.5f - fieldDimensions_.fieldCenterCircleDiameter &&
        std::abs(projectedCirclePosition.y()) <
            (fieldDimensions_.goalInnerWidth + 2 * fieldDimensions_.goalPostDiameter) * 0.75f)
    {
      return;
    }
  }
  if (centerCircle.hasOrientation)
  {
    // find the most plausible pose that explains the center circle observeration
    const Pose observationPose1 =
        Pose(centerCircle.position, Angle::normalized(centerCircle.orientation)).inverse();
    const Pose observationPose2 =
        Pose(-observationPose1.position, Angle::normalized(observationPose1.orientation + M_PI));
    // a pose update is performed with the observation pose, that differs less in orientation since
    // the orientation is very reliable thanks to the imu-sensorfusion
    const auto update = Angle::angleDiff(observationPose1.orientation, stateMean_.z()) <
                                Angle::angleDiff(observationPose2.orientation, stateMean_.z())
                            ? Vector3f(observationPose1.position.x(), observationPose1.position.y(),
                                       observationPose1.orientation)
                            : Vector3f(observationPose2.position.x(), observationPose2.position.y(),
                                       observationPose2.orientation);
    // compute the covariance from the error model of the camera pose
    const auto cov =
        computePoseCovFromFullPoseFeature(centerCircle.position, update.z(), cam2ground);
    poseSensorUpdate(update, cov);
  }
  else
  {
    // compute the covariance from the error model of the camera pose
    const auto cov = projectionMeasurementModel_.computePointCovFromPositionFeature(
        centerCircle.position, cam2ground);
    fieldPointUpdate(centerCircle.position, {0.f, 0.f}, cov);
  }
}

void PoseHypothesis::updateWithPenaltyArea(const LandmarkModel::PenaltyArea& relativePenaltyArea,
                                           const KinematicMatrix& cam2ground,
                                           const bool ignorePenaltyAreasWithoutOrientation)
{
  if (relativePenaltyArea.hasOrientation)
  {
    const auto opponentPenaltySpotPosition = Vector2f(
        fieldDimensions_.fieldLength / 2 - fieldDimensions_.fieldPenaltyMarkerDistance, 0.f);
    // find the most plausible pose that explains the penalty area observeration
    const Pose observationPose1 =
        Pose(opponentPenaltySpotPosition) *
        Pose(relativePenaltyArea.position, relativePenaltyArea.orientation).inverse();
    const Pose observationPose2 =
        Pose(-observationPose1.position, Angle::normalized(observationPose1.orientation + M_PI));
    // a pose update is performed with the observation pose, that differs less in orientation since
    // the orientation is very reliable thanks to the imu-sensorfusion
    const auto update = Angle::angleDiff(observationPose1.orientation, stateMean_.z()) <
                                Angle::angleDiff(observationPose2.orientation, stateMean_.z())
                            ? Vector3f(observationPose1.position.x(), observationPose1.position.y(),
                                       observationPose1.orientation)
                            : Vector3f(observationPose2.position.x(), observationPose2.position.y(),
                                       observationPose2.orientation);
    // compute the covariance from the error model of the camera pose
    const auto cov =
        computePoseCovFromFullPoseFeature(relativePenaltyArea.position, update.z(), cam2ground);
    poseSensorUpdate(update, cov);
  }
  else if (!ignorePenaltyAreasWithoutOrientation)
  {
    const auto& relativePenaltySpot = relativePenaltyArea.position;
    // the absolute position of the penalty spot when projected from the curren state mean
    Vector2f projectedPenaltySpot = getPoseMean() * relativePenaltySpot;
    // find out with penalty spot this was in the world
    assert(fieldInfo_.penaltySpots.size() == 2);
    Vector2f associatedPenaltySpot =
        (fieldInfo_.penaltySpots[0] - projectedPenaltySpot).squaredNorm() <
                (fieldInfo_.penaltySpots[1] - projectedPenaltySpot).squaredNorm()
            ? fieldInfo_.penaltySpots[0]
            : fieldInfo_.penaltySpots[1];
    // compute the covariance matrix of the point feature for the update
    const auto cov = projectionMeasurementModel_.computePointCovFromPositionFeature(
        relativePenaltySpot, cam2ground);
    // perform a UKF-update under the assumption that the detected penalty spot matches the
    // associated field position
    fieldPointUpdate(relativePenaltySpot, associatedPenaltySpot, cov);
  }
}

void PoseHypothesis::updateWithSetOfIntersections(const std::vector<LandmarkModel::Intersection>& intersections,
											const KinematicMatrix& cam2ground) {
	cam2ground.invert();
	if (intersections.size() > 1) {

		auto it = intersections.begin();
		for (; std::next(it) != intersections.end(); it++) {

			auto ti = std::next(it);
			for (; ti != intersections.end(); ti++) {

				auto intersection1 = *it;
				auto intersection2 = *ti;
				switch(intersection1.intersectionType) {

					case LandmarkModel::Intersection::IntersectionType::LINTERSECTION:
						switch(intersection2.intersectionType) {

							case LandmarkModel::Intersection::IntersectionType::LINTERSECTION:
								updateWithLLIntersections(intersection1, intersection2, cam2ground);
								break;

							case LandmarkModel::Intersection::IntersectionType::TINTERSECTION:
								updateWithLTIntersections(intersection1, intersection2, cam2ground);
								break;

							case LandmarkModel::Intersection::IntersectionType::XINTERSECTION:
								break;

							default:
								break;
						}
						break;

					case LandmarkModel::Intersection::IntersectionType::TINTERSECTION:
						switch(intersection2.intersectionType) {

							case LandmarkModel::Intersection::IntersectionType::LINTERSECTION:
								updateWithLTIntersections(intersection2, intersection1, cam2ground);
								break;

							case LandmarkModel::Intersection::IntersectionType::TINTERSECTION:
								updateWithTTIntersections(intersection1, intersection2, cam2ground);
								break;

							case LandmarkModel::Intersection::IntersectionType::XINTERSECTION:
								updateWithTXIntersections(intersection1, intersection2, cam2ground);
								break;

							default:
								break;
						}
						break;

					case LandmarkModel::Intersection::IntersectionType::XINTERSECTION:
						switch(intersection2.intersectionType) {

							case LandmarkModel::Intersection::IntersectionType::LINTERSECTION:
								break;

							case LandmarkModel::Intersection::IntersectionType::TINTERSECTION:
								updateWithTXIntersections(intersection2, intersection1, cam2ground);
								break;

							case LandmarkModel::Intersection::IntersectionType::XINTERSECTION:
								updateWithXXIntersections(intersection1, intersection2, cam2ground);
								break;

							default:
								break;
						}
						break;

					default:
						break;
				}

			}

		}

	}
}

void PoseHypothesis::updateWithLLIntersections(const LandmarkModel::Intersection& intersection1,
		const LandmarkModel::Intersection& intersection2,
		const KinematicMatrix& cam2ground) {
	float orientationDiff = Angle::angleDiff(intersection1.orientation,
											 intersection2.orientation);
	Pose associatedL1;
	Pose associatedL2;
	bool found = false;
	float distance = (intersection1.position - intersection2.position).norm();
	if (abs(orientationDiff - M_PI / 2) < intersectionOrientationThreshold_()) { // TODO Parameter
		if (abs(distance - fieldDimensions_.fieldPenaltyAreaWidth) < intersectionDistanceThreshold_()) {
			std::cerr << "LLPA" << std::endl;
			if (Angle::normalized(intersection2.orientation - intersection1.orientation) > 0) {
				// Outside of penalty area
				associatedL1 = Pose(
						fieldDimensions_.fieldLength / 2 - fieldDimensions_.fieldPenaltyAreaLength,
						fieldDimensions_.fieldPenaltyAreaWidth / 2,
						-M_PI / 4);
				associatedL2 = Pose(associatedL1.position.x(), -associatedL1.position.y(), -associatedL2.orientation);
			}
			else {
				// Inside of penalty area
				associatedL1 = Pose(
						fieldDimensions_.fieldLength / 2 - fieldDimensions_.fieldPenaltyAreaLength,
						-fieldDimensions_.fieldPenaltyAreaWidth / 2,
						M_PI / 4);
				associatedL2 = Pose(associatedL1.position.x(), -associatedL1.position.y(),  -associatedL1.orientation);
			}
			found = true;
		} else if (abs(distance - fieldDimensions_.fieldWidth) < intersectionDistanceThreshold_()) {
			std::cerr << "LLFW" << std::endl;
		} else if (abs(distance - fieldDimensions_.fieldLength) < intersectionDistanceThreshold_()) {
			std::cerr << "LLFL" << std::endl;
		}
	}
	if (found) {
		const Pose observationPose1L1 = associatedL1 * Pose(intersection1.position, intersection1.orientation).inverse();
		const Pose observationPose2L1 = Pose(-observationPose1L1.position, Angle::normalized(observationPose1L1.orientation + M_PI));
		const auto updateLL = Angle::angleDiff(observationPose1L1.orientation, stateMean_.z()) <
							  Angle::angleDiff(observationPose2L1.orientation, stateMean_.z())
							  ? Vector3f(observationPose1L1.position.x(), observationPose1L1.position.y(),
										 observationPose1L1.orientation)
							  : Vector3f(observationPose2L1.position.x(), observationPose2L1.position.y(),
										 observationPose2L1.orientation);
		const auto covLL =
				computePoseCovFromFullPoseFeature(intersection1.position, updateLL.z(), cam2ground);
		poseSensorUpdate(updateLL, covLL);
		const Pose observationPose1L2 = associatedL2 * Pose(intersection2.position, intersection2.orientation).inverse();
		const Pose observationPose2L2 = Pose(-observationPose1L2.position, Angle::normalized(observationPose1L2.orientation + M_PI));
		const auto updateLR = Angle::angleDiff(observationPose1L2.orientation, stateMean_.z()) <
							  Angle::angleDiff(observationPose2L2.orientation, stateMean_.z())
							  ? Vector3f(observationPose1L2.position.x(), observationPose1L2.position.y(),
										 observationPose1L2.orientation)
							  : Vector3f(observationPose2L2.position.x(), observationPose2L2.position.y(),
										 observationPose2L2.orientation);
		const auto covT =
				computePoseCovFromFullPoseFeature(intersection2.position, updateLR.z(), cam2ground);
		std::cerr << updateLL.x() << "/" << updateLL.y() << "/" << updateLL.z() << std::endl;
		std::cerr << updateLR.x() << "/" << updateLR.y() << "/" << updateLR.z() << std::endl;
		poseSensorUpdate(updateLR, covT);
	}
}

void PoseHypothesis::updateWithLTIntersections(const LandmarkModel::Intersection& LIntersection,
											  const LandmarkModel::Intersection& TIntersection,
											  const KinematicMatrix& cam2ground) {
	float orientationDiff = Angle::angleDiff(LIntersection.orientation,
											 TIntersection.orientation);
	Pose associatedL;
	Pose associatedT;
	bool found = false;
	float distance = (LIntersection.position - TIntersection.position).norm();
	if (abs(orientationDiff - M_PI * 3 / 4) < intersectionOrientationThreshold_()) { // TODO Parameter
		// Penalty area
		if (abs(distance - fieldDimensions_.fieldPenaltyAreaLength) < intersectionDistanceThreshold_()) {
			if (Angle::normalized(TIntersection.orientation - LIntersection.orientation) > 0) {
				// Right side of penalty area
				std::cerr << "LTPAR" << std::endl;
				associatedL = Pose(
						fieldDimensions_.fieldLength / 2 - fieldDimensions_.fieldPenaltyAreaLength,
						-fieldDimensions_.fieldPenaltyAreaWidth / 2,
						M_PI / 4);
				associatedT = Pose(fieldDimensions_.fieldLength / 2, associatedL.position.y(), M_PI);
			}
			else {
				// Left side of penalty area
				std::cerr << "LTPAL" << std::endl;
				associatedL = Pose(
						fieldDimensions_.fieldLength / 2 - fieldDimensions_.fieldPenaltyAreaLength,
						fieldDimensions_.fieldPenaltyAreaWidth / 2,
						-M_PI / 4);
				associatedT = Pose(fieldDimensions_.fieldLength / 2, associatedL.position.y(), M_PI);
			}
			found = true;
		}
	} else if (abs(orientationDiff - M_PI / 4) < intersectionOrientationThreshold_()) {
		// Side line or back line
		if (abs(distance - fieldDimensions_.fieldLength / 2) < intersectionDistanceThreshold_()) {
			// Side line
			if (Angle::normalized(TIntersection.orientation - LIntersection.orientation) > 0) {
				// Right side of goal
				std::cerr << "LTSLR" << std::endl;
				associatedL = Pose(
						fieldDimensions_.fieldLength / 2,
						-fieldDimensions_.fieldWidth / 2,
						M_PI * 3 / 4);
				associatedT = Pose(
						0,
						-fieldDimensions_.fieldPenaltyAreaWidth / 2,
						M_PI / 2);
			} else {
				// Left side of goal
				std::cerr << "LTSLL" << std::endl;
				associatedL = Pose(
						fieldDimensions_.fieldLength / 2,
						fieldDimensions_.fieldWidth / 2,
						-M_PI * 3 / 4);
				associatedT = Pose(
						0,
						fieldDimensions_.fieldPenaltyAreaWidth / 2,
						-M_PI / 2);
			}
			found = true;
		} else if (abs(distance - (fieldDimensions_.fieldWidth / 2 - fieldDimensions_.fieldPenaltyAreaWidth / 2)) < intersectionDistanceThreshold_()) {
			// Back line direct
			if (Angle::normalized(TIntersection.orientation - LIntersection.orientation) > 0) {
				// Right side of goal
				std::cerr << "LTBLDR" << std::endl;
				associatedL = Pose(
						fieldDimensions_.fieldLength / 2,
						-fieldDimensions_.fieldWidth / 2,
						M_PI * 3 / 4);
				associatedT = Pose(
						associatedL.position.x(),
						-fieldDimensions_.fieldPenaltyAreaWidth / 2,
						M_PI);
			} else {
				// Left side of goal
				std::cerr << "LTBLDL" << std::endl;
				associatedL = Pose(
						fieldDimensions_.fieldLength / 2,
						fieldDimensions_.fieldWidth / 2,
						-M_PI * 3 / 4);
				associatedT = Pose(
						associatedL.position.x(),
						fieldDimensions_.fieldPenaltyAreaWidth / 2,
						M_PI);
			}
			found = true;
		} else if (abs(distance - (fieldDimensions_.fieldWidth / 2 + fieldDimensions_.fieldPenaltyAreaWidth / 2)) < intersectionDistanceThreshold_()) {
			// Back line with T between
			if (Angle::normalized(TIntersection.orientation - LIntersection.orientation) > 0) {
				// Right side of goal
				std::cerr << "LTBLXR" << std::endl;
				associatedL = Pose(
						fieldDimensions_.fieldLength / 2,
						-fieldDimensions_.fieldWidth / 2,
						M_PI * 3 / 4);
				associatedT = Pose(
						associatedL.position.x(),
						fieldDimensions_.fieldPenaltyAreaWidth / 2,
						-M_PI);
			} else {
				// Left side of goal
				std::cerr << "LTBLXR" << std::endl;
				associatedL = Pose(
						fieldDimensions_.fieldLength / 2,
						-fieldDimensions_.fieldWidth / 2,
						-M_PI * 3 / 4);
				associatedT = Pose(
						associatedL.position.x(),
						fieldDimensions_.fieldPenaltyAreaWidth / 2,
						-M_PI);
			}
			found = true;
		}
	}
	if (found) {
		const Pose observationPose1L = associatedL * Pose(LIntersection.position, LIntersection.orientation).inverse();
		const Pose observationPose2L = Pose(-observationPose1L.position, Angle::normalized(observationPose1L.orientation + M_PI));
		const auto updateL = Angle::angleDiff(observationPose1L.orientation, stateMean_.z()) <
							 Angle::angleDiff(observationPose2L.orientation, stateMean_.z())
							 ? Vector3f(observationPose1L.position.x(), observationPose1L.position.y(),
										observationPose1L.orientation)
							 : Vector3f(observationPose2L.position.x(), observationPose2L.position.y(),
										observationPose2L.orientation);
		const auto covL =
				computePoseCovFromFullPoseFeature(LIntersection.position, updateL.z(), cam2ground);
		poseSensorUpdate(updateL, covL);
		const Pose observationPose1T = associatedT * Pose(TIntersection.position, TIntersection.orientation).inverse();
		const Pose observationPose2T = Pose(-observationPose1T.position, Angle::normalized(observationPose1T.orientation + M_PI));
		const auto updateT = Angle::angleDiff(observationPose1T.orientation, stateMean_.z()) <
							 Angle::angleDiff(observationPose2T.orientation, stateMean_.z())
							 ? Vector3f(observationPose1T.position.x(), observationPose1T.position.y(),
										observationPose1T.orientation)
							 : Vector3f(observationPose2T.position.x(), observationPose2T.position.y(),
										observationPose2T.orientation);
		const auto covT =
				computePoseCovFromFullPoseFeature(TIntersection.position, updateT.z(), cam2ground);
		std::cerr << updateL.x() << "/" << updateL.y() << "/" << updateL.z() << std::endl;
		std::cerr << updateT.x() << "/" << updateT.y() << "/" << updateT.z() << std::endl;
		poseSensorUpdate(updateT, covT);
	}
}

void PoseHypothesis::updateWithTTIntersections(const LandmarkModel::Intersection& ,
											   const LandmarkModel::Intersection& ,
											   const KinematicMatrix& ) {
	return;
}

void PoseHypothesis::updateWithTXIntersections(const LandmarkModel::Intersection& TIntersection,
											   const LandmarkModel::Intersection& XIntersection,
											   const KinematicMatrix& cam2ground) {;
	Pose associatedT;
	Vector2f associatedX;
	bool found = false;
	float distance = (TIntersection.position - XIntersection.position).norm();
	if (abs(distance - (fieldDimensions_.fieldWidth / 2 - fieldDimensions_.fieldCenterCircleDiameter / 2)) < intersectionDistanceThreshold_()) {
		// Direct
		std::cerr << "TXD" << std::endl;
		associatedT = Pose(
				0,
				fieldDimensions_.fieldWidth / 2,
				-M_PI);
		associatedX = Vector2f(
				0,
				-(fieldDimensions_.fieldWidth / 2 - fieldDimensions_.fieldCenterCircleDiameter / 2));
		found = true;
	} else if (abs(distance - (fieldDimensions_.fieldWidth / 2 + fieldDimensions_.fieldCenterCircleDiameter / 2)) < intersectionDistanceThreshold_()) {
		// X between
		std::cerr << "TXX" << std::endl;
		associatedT = Pose(
				0,
				fieldDimensions_.fieldWidth / 2,
				-M_PI);
		associatedX = Vector2f(
				0,
				fieldDimensions_.fieldWidth / 2 - fieldDimensions_.fieldCenterCircleDiameter / 2);
		found = true;
	}
	if (found) {
		const Pose observationPose1T = associatedT * Pose(TIntersection.position, TIntersection.orientation).inverse();
		const Pose observationPose2T = Pose(-observationPose1T.position, Angle::normalized(observationPose1T.orientation + M_PI));
		const auto updateT = Angle::angleDiff(observationPose1T.orientation, stateMean_.z()) <
							 Angle::angleDiff(observationPose2T.orientation, stateMean_.z())
							 ? Vector3f(observationPose1T.position.x(), observationPose1T.position.y(),
										observationPose1T.orientation)
							 : Vector3f(observationPose2T.position.x(), observationPose2T.position.y(),
										observationPose2T.orientation);
		const auto covT =
				computePoseCovFromFullPoseFeature(TIntersection.position, updateT.z(), cam2ground);
		poseSensorUpdate(updateT, covT);
		const auto covX =
				projectionMeasurementModel_.computePointCovFromPositionFeature(XIntersection.position, cam2ground);
		std::cerr << updateT.x() << "/" << updateT.y() << "/" << updateT.z() << std::endl;
		fieldPointUpdate(XIntersection.position, associatedX, covX);
	}
}

void PoseHypothesis::updateWithXXIntersections(const LandmarkModel::Intersection& intersection1,
											   const LandmarkModel::Intersection& intersection2,
											   const KinematicMatrix& cam2ground) {
	float distance = (intersection1.position - intersection2.position).norm();
	if (abs(distance - fieldDimensions_.fieldCenterCircleDiameter) < intersectionDistanceThreshold_()) {
		Vector2f associatedX1;
		Vector2f associatedX2;
		if ((abs(stateMean_.z()) <= M_PI / 2 && intersection1.position.y() > intersection1.position.y()) ||
			(abs(stateMean_.z()) >= M_PI / 2 && intersection2.position.y() < intersection2.position.y())) {
			associatedX1 = Vector2f(
					0,
					fieldDimensions_.fieldCenterCircleDiameter / 2);
		} else {
			associatedX1 = Vector2f(
					0,
					-fieldDimensions_.fieldCenterCircleDiameter / 2);
		}
		const auto covX1 =
				projectionMeasurementModel_.computePointCovFromPositionFeature(intersection1.position, cam2ground);
		fieldPointUpdate(intersection1.position, associatedX1, covX1);
		associatedX2 = Vector2f(
				0,
				-associatedX1.y());
		const auto covX2 =
				projectionMeasurementModel_.computePointCovFromPositionFeature(intersection2.position, cam2ground);
		fieldPointUpdate(intersection2.position, associatedX2, covX2);
	}
}

void PoseHypothesis::lineSensorUpdate(const Line<float>& relativeLine, const Vector3f& refPose,
                                      const KinematicMatrix& cam2ground)
{
  /**
   * In order to use a line for a sensor update the percepted line as to be associated with a
   * known line on the field. At this point the association must have been successfull.
   *
   * The algorithm can obtain to independent infromation of a (straight) line:
   * - a distance infromation (for one direction) obtained from the distance to the line (hesse
   * normal form)
   * - a orientation information obtained from the orientation of the line
   */
  auto associatedLine = findAssociatedLine(relativeLine, refPose);
  if (!associatedLine.ptrToLine)
  {
    // Lines that could not be associated will be dropped
    return;
  }
  if (!isFieldLineAlongAxis(*associatedLine.ptrToLine))
  {
    // line was associated with center circle
    generateCenterCircleCandatesFromTanget(relativeLine);
    return;
  }

  bool lineAlongY = (associatedLine.ptrToLine->p1.x() == associatedLine.ptrToLine->p2.x());

  // All line passed to this method are either vertical or horizontal (circle segements are handled
  // in UKFPose2D::circleSegementUpdate
  Vector2f pose1DObservation =
      computePose1DFromLine(relativeLine, *associatedLine.ptrToLine, refPose);
  if (!fieldDimensions_.isInsideCarpet(lineAlongY ? Vector2f(pose1DObservation.x(), 0.f)
                                                  : Vector2f(0.f, pose1DObservation.x())))
  {
    // drop updates out side the carpet
    return;
  }

  // Calculating the cov for this measurement
  Vector2f centerPoint = (relativeLine.p1 + relativeLine.p2) / 2;
  Vector2f eCenterPoint = {centerPoint.x(), centerPoint.y()};
  Matrix2f relativeXYCov =
      projectionMeasurementModel_.computeCovFromErrorPropagation(eCenterPoint, cam2ground);
  // rotate to to global:
  Rotation2Df robot2global(pose1DObservation.y());
  Matrix2f absoluteXYCov = robot2global * relativeXYCov * robot2global.inverse();
  // compose dist and angle cov:
  float distVariance = lineAlongY ? absoluteXYCov(0, 0) : absoluteXYCov(1, 1);
  // the landmark filter will ensure that we don't get too short line segements
  assert((relativeLine.p1 - relativeLine.p2).squaredNorm() > 0.000001f);
  const float angleVariance =
      pow(atan(sqrt(4.f * distVariance / (relativeLine.p1 - relativeLine.p2).squaredNorm())), 2);
  const Matrix2f lineCov = (Matrix2f() << distVariance + measurementBaseVariance_().x(), 0.f, 0.f,
                            angleVariance + measurementBaseVariance_().z())
                               .finished();

  pose1DSensorUpdate(pose1DObservation, lineAlongY, lineCov);
}

Vector2f PoseHypothesis::computePose1DFromLine(const Line<float>& relativeLine,
                                               const Line<float>& associatedLine,
                                               const Vector3f& referencePose) const
{
  bool lineAlongY = associatedLine.p1.x() == associatedLine.p2.x();

  Vector2f absolute1DPose = Vector2f::Zero();

  // transform line into absolute coordinates (with reference pose)
  Line<float> absoluteLine = absoluteFromRelativeLine(relativeLine, referencePose);

  // determine order of line end points so that the "difference vector" points towards positive
  // values of the axis
  bool p2IsUpperPoint = ((lineAlongY && absoluteLine.p2.y() > absoluteLine.p1.y()) ||
                         (!lineAlongY && absoluteLine.p2.x() > absoluteLine.p1.x()));

  // hesseNormalForm will provide distance and side information due to the fact, that one can
  // ensure now, that the vector points towards higher values of the axis
  Line<float> relativeLineSignRight = p2IsUpperPoint
                                          ? Line<float>(relativeLine.p1, relativeLine.p2)
                                          : Line<float>(relativeLine.p2, relativeLine.p1);
  Vector2f relativeLineVectorSignRight = relativeLineSignRight.p2 - relativeLineSignRight.p1;

  float distanceLeftOfLine = hesseNormalDist(relativeLineSignRight, {0, 0});

  if (lineAlongY)
  { // one can obtain an y update form a line along the y axis
    absolute1DPose(0) = associatedLine.p1.x() - distanceLeftOfLine;
    absolute1DPose(1) = atan2(relativeLineVectorSignRight.x(), relativeLineVectorSignRight.y());
  }
  else
  {
    // For now this can only handle vertical and horizontal lines.
    // Every other line has to be handled in the circleSegementSensorUpdate
    assert(associatedLine.p1.y() == associatedLine.p2.y());
    absolute1DPose(0) = associatedLine.p1.y() + distanceLeftOfLine;
    absolute1DPose(1) = atan2(-relativeLineVectorSignRight.y(), relativeLineVectorSignRight.x());
  }
  // By calculating the orientation with atan2 the angle is implicitly normalized
  return absolute1DPose;
}

void PoseHypothesis::generateCenterCircleCandatesFromTanget(const Line<float>& relativeLine)
{
  // the center of the line
  const Vector2f relativeLineCenter = (relativeLine.p1 + relativeLine.p2) * 0.5f;
  // calculate the othorgonal line vector:
  const Vector2f lineVector = relativeLine.p2 - relativeLine.p1;
  const Vector2f relativeOthorgonalLineRadius = Vector2f(lineVector.y(), -lineVector.x()) /
                                                (lineVector.norm()) *
                                                fieldDimensions_.fieldCenterCircleDiameter * 0.5f;
  // calculate the relativeCircleCenter based on point with lower error
  const Vector2f relativeCircleCenter;

  const Vector2f relativeCircleCenterCandidate1 = relativeLineCenter + relativeOthorgonalLineRadius;
  const Vector2f relativeCircleCenterCandidate2 = relativeLineCenter - relativeOthorgonalLineRadius;

  centerPointCandidates_.emplace_back(relativeCircleCenterCandidate1.x(),
                                      relativeCircleCenterCandidate1.y());
  centerPointCandidates_.emplace_back(relativeCircleCenterCandidate2.x(),
                                      relativeCircleCenterCandidate2.y());
}

float PoseHypothesis::angularMetric(const Vector2f& a, const Vector2f& b) const
{
  // a * b = |a| * |b| * cos(alpha)
  float result = fabs(std::acos(a.dot(b) / (a.norm() * b.norm())));
  return (result > M_PI_2) ? M_PI - result : result;
}

bool PoseHypothesis::isFieldLineAlongAxis(const Line<float>& fieldLine) const
{
  return fieldLine.p2.x() == fieldLine.p1.x() || fieldLine.p2.y() == fieldLine.p1.y();
}


Line<float> PoseHypothesis::absoluteFromRelativeLine(const Line<float>& relativeLine,
                                                     const Vector3f& referencePose) const
{
  Pose refPose(referencePose.x(), referencePose.y(), referencePose.z());
  return Line<float>(refPose * relativeLine.p1, refPose * relativeLine.p2);
}

void PoseHypothesis::toValue(Uni::Value& value) const
{
  value = Uni::Value(Uni::ValueType::OBJECT);
  value["stateMean"] << stateMean_;
  value["sigmaPoints"] << sigmaPoints_;
  value["meanEvalError"] << meanEvalError_;
  value["id"] << id_;
}
