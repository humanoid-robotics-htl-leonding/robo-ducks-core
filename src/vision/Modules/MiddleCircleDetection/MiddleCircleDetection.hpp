//
// Created by marc on 2/11/20.
//

#include <Framework/Module.hpp>
#include <Tools/Math/Circle.hpp>
#include <Tools/Math/CircleFitter.cpp>

#include "Data/CameraMatrix.hpp"
#include "Data/FilteredSegments.hpp"
#include "Data/ImageData.hpp"
#include "Data/FieldDimensions.hpp"
#include "Data/CircleData.hpp"


#ifndef TUHHNAO_MIDDLECIRCLEDETECTION_HPP
#define TUHHNAO_MIDDLECIRCLEDETECTION_HPP

#endif //TUHHNAO_MIDDLECIRCLEDETECTION_HPP

class Brain;

class MiddleCircleDetection : public Module<MiddleCircleDetection, Brain> {
public:
    ModuleName name = "MiddleCircleDetection";

    MiddleCircleDetection(const ModuleManagerInterface &manager);

    void cycle();

private:
    /// candidate points
    VecVector2i middleCirclePoints_;
    /// a reference to the filtered segments
    /// robot divides the playground with horizontal and
    /// vertical lines
    const Dependency<FilteredSegments> filteredSegments_;
    /// the minimum number of points in a middle circle segment
    const Parameter<unsigned int> minSegmentLength_;
    /// the maximum number of points in a middle circle segment
    const Parameter<unsigned int> maxSegmentLength_;
    /// a reference to the image without debug points
    const Dependency<ImageData> imageData_;
    /// field Dimensions
    const Dependency<FieldDimensions> fieldDimensions_;
    /// goal post points for debug purposes
    VecVector2i debugMiddleCirclePoints_;

    /// detected lines
    std::vector<Circle<int>> middleCircles_;

    const Dependency<CameraMatrix> cameraMatrix_;

    Production<CircleData> circleData_;

    /// Data of the found Circle on screen
    CircleData foundCircleData;
    /// detect points on the outline of the middle circle
    void detectMiddleCirclePoints();

    void sendImagesForDebug();

    void createCircleData();

    CircleFitter circleFitter_;
    void initCorrectCircle();

    bool CircleIsValid(int iterationAmount, Circle<double> circle);

    int ControlCircleBorder();

    void UpdateCircleBorderPoints(Circle<double> circle);

    VecVector2f circleBorderPoints_;
};