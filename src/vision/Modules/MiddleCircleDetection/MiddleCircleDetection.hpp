//
// Created by marc on 2/11/20.
//

#pragma once

#include <Framework/Module.hpp>
#include <Tools/Math/Circle.hpp>

#include "Data/CameraMatrix.hpp"
#include "Data/FilteredSegments.hpp"
#include "Data/ImageSegments.hpp"
#include "Data/ImageData.hpp"
#include "Data/FieldDimensions.hpp"
#include "Data/CircleData.hpp"

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
    const Dependency<ImageSegments> imageSegments_;
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

    void initCorrectCircle();

    bool circleIsValid(const Circle<float>& circle);

    double controlCircleBorder(const Circle<float>& circle);

    void generateCircleSurroundPoints(const Circle<float>& circle);

    void pixelToRobot(const VecVector2i& screenPoints, VecVector2f &planePoints) const;

    VecVector2f circleBorderPoints_;

};