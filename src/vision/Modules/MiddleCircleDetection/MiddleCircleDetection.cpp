//
// Created by marc on 2/11/20.
//

#include <Tools/Chronometer.hpp>
#include <Tools/Math/Geometry.hpp>
#include "MiddleCircleDetection.hpp"
#include <Modules/LineDetection/LineDetection.hpp>

MiddleCircleDetection::MiddleCircleDetection(const ModuleManagerInterface &manager)
        : Module(manager)
        , filteredSegments_(*this)
        , minSegmentLength_(*this, "minSegmentLength", [] {})
        , maxSegmentLength_(*this, "maxSegmentLength", [] {})
        , imageData_(*this)
        ,fieldDimensions_(*this)
        , cameraMatrix_(*this)
        , circleData_(*this)
{
}

void MiddleCircleDetection::detectMiddleCirclePoints() {
    middleCirclePoints_.clear();
    auto shift = [](int c) { return c >> 1; };
    for (const auto &segment : filteredSegments_->vertical) {
        if (static_cast<unsigned int>(segment->scanPoints) < minSegmentLength_() ||
            static_cast<unsigned int>(segment->scanPoints) > maxSegmentLength_() ||
            segment->startEdgeType != EdgeType::RISING || segment->endEdgeType != EdgeType::FALLING)
        {
            continue;
        }

        middleCirclePoints_.push_back((segment->start + segment->end).unaryExpr(shift));
    }
}

void MiddleCircleDetection::cycle() {
    if (!filteredSegments_->valid) {
        return;
    } else {
        detectMiddleCirclePoints();
        debugMiddleCirclePoints_ = middleCirclePoints_;
        //todo get points from line detection
        initCorrectCircle();
    }

    sendImagesForDebug();

}

void MiddleCircleDetection::initCorrectCircle() {
    double centerX;
    double centerY;
    double radius;

    /*Vector2i newVec1;
    newVec1.x() = 1;
    newVec1.y() = 1;
    Vector2i newVec2;
    newVec2.x() = -1;
    newVec2.y() = -1;
    Vector2i newVec3;
    newVec3.x() = 1;
    newVec3.y() = -1;
    Vector2i newVec4;
    newVec4.x() = -1;
    newVec4.y() = 1;
    Vector2i newVec5;
    newVec5.x() = 1;
    newVec5.y() = 0;
    Vector2i newVec6;
    newVec6.x() = -1;
    newVec6.y() = 0;
    Vector2i newVec7;
    newVec7.x() = 0;
    newVec7.y() = -1;
    Vector2i newVec8;
    newVec8.x() = 0;
    newVec8.y() = 1;

    VecVector2i vecList;
    vecList.push_back(newVec1);
    vecList.push_back(newVec2);
    vecList.push_back(newVec3);
    vecList.push_back(newVec4);
    vecList.push_back(newVec5);
    vecList.push_back(newVec6);
    vecList.push_back(newVec7);
    vecList.push_back(newVec8);*/

    VecVector2f planePoints;
    Vector2f planePoint;
    for(auto point : middleCirclePoints_){
        if (cameraMatrix_->pixelToRobot(point, planePoint)){
            planePoints.push_back(planePoint);
        }
    }

    int iterations = circleFitter_.circleFit(planePoints.size(), planePoints, &centerX, &centerY, &radius);
    Circle<double> circle(Vector2<double>(centerX,centerY), radius);


    if (CircleIsValid(iterations,circle)){
        UpdateCircleBorderPoints(circle);
        //double amount = ControlCircleBorder(circle);
        //if(neightbourAmountPercentage == 1)
        {
            foundCircleData.circle.center = Vector2f(centerX, centerY);
            foundCircleData.circle.radius = radius;
            printf("Amount: %d | Iteration: %d | CenterX: %f | CenterY: %f | Radius: %f \n"
                    ,ControlCircleBorder(), iterations, centerX, centerY, radius);
        }
    }
}

void MiddleCircleDetection::createCircleData() {
    printf("Radius: %f \n", circleData_->circle.radius);
}

void MiddleCircleDetection::sendImagesForDebug()
{
    auto mount = mount_ + "." + imageData_->identification + "_image_circle";
    Image image(imageData_->image422.to444Image());

    if (debug().isSubscribed(mount)) {

        /// Draw found Points
        for (const auto &point : debugMiddleCirclePoints_) {
            image.circle(Image422::get444From422Vector(point), 5, Color::ORANGE);
        }

        /// Draw Circle
        Vector2i pixelCoords;
        for (auto point : circleBorderPoints_) {
            // both (x,y) and (x,-y) are points on the half-circle
            Vector2f circlePoint(point);
            if (cameraMatrix_->robotToPixel(circlePoint,pixelCoords)){
                image.circle(Image422::get444From422Vector(pixelCoords), 5, Color::RED);
            }
        }

        /// Draw middle Point
        Vector2f centerPoint(foundCircleData.circle.center);
        if (cameraMatrix_->robotToPixel(centerPoint,pixelCoords)){
            image.cross(Image422::get444From422Vector(pixelCoords), 10, Color::YELLOW);
        }

        debug().sendImage(mount_ + "." + imageData_->identification + "_image_circle", image);

    }
}

bool MiddleCircleDetection::CircleIsValid(int iterationAmount, Circle<double> circle) {
    const double RADIUS_TOLERANCE = 0.2;
    if(iterationAmount == -1 ||
       circle.radius < (fieldDimensions_->fieldCenterCircleDiameter / 2 - fieldDimensions_->fieldCenterCircleDiameter / 2 * RADIUS_TOLERANCE) ||
       circle.radius > (fieldDimensions_->fieldCenterCircleDiameter / 2 + fieldDimensions_->fieldCenterCircleDiameter / 2 * RADIUS_TOLERANCE)
    ){
        return false;
    }
    return true;

}

int MiddleCircleDetection::ControlCircleBorder() {
    int amount = 0;
    auto circle = foundCircleData.circle;
    //const double RADIUS_TOLERANCE = 0.2;
    for (auto point : middleCirclePoints_){
        if ((point.x() - circle.center.x())*(point.x() - circle.center.x()) +
                (point.y() - circle.center.y())*(point.y() - circle.center.y()) <=
                circle.radius*circle.radius){
            amount++;
        }
    }
    return amount;
}

void MiddleCircleDetection::UpdateCircleBorderPoints(Circle<double> circle) {
    circleBorderPoints_.clear();

    for (int angle = -180; angle < 181; angle++) {
        double x = circle.radius*cos(angle* M_PI/180) + foundCircleData.circle.center.x();
        double y = circle.radius*sin(angle* M_PI/180) + foundCircleData.circle.center.y();

        circleBorderPoints_.push_back(Vector2f(x,y));
    }
}
