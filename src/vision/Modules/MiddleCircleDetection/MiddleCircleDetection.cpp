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
        , imageSegments_(*this)
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
        middleCirclePoints_.push_back((segment->start+segment->end).unaryExpr(shift)); //fails
    }

    // Check Neighbours
    const double RADIUS_TOLERANCE = 0.2;
    for(auto it = middleCirclePoints_.begin(); it!=middleCirclePoints_.end(); it++)
    {
        bool isValid = true;
        auto firstPoint = *it;
        for(const auto secondPoint: middleCirclePoints_)
        {
            float squaredDistance = (firstPoint-secondPoint).squaredNorm();
                if(squaredDistance > (fieldDimensions_->fieldCenterCircleDiameter *(1- RADIUS_TOLERANCE)) &&
               squaredDistance < (fieldDimensions_->fieldCenterCircleDiameter  * (1+RADIUS_TOLERANCE)))
            {
                isValid=false;
                break;
            }
        }
        if(!isValid){
            // remove point
            it--;
            middleCirclePoints_.erase(std::next(it));
        }
    }
}

void MiddleCircleDetection::cycle() {
    if (!filteredSegments_->valid) {
        return;
    } else {
        detectMiddleCirclePoints();
        debugMiddleCirclePoints_ = middleCirclePoints_;
        initCorrectCircle();
    }


    sendImagesForDebug();

}

void MiddleCircleDetection::initCorrectCircle() {
    double centerX;
    double centerY;
    double radius;



    VecVector2f planePoints;
    Vector2f planePoint;

    PixelToRobot(middleCirclePoints_, planePoints);

    int iterations = circleFitter_.circleFit(planePoints.size(), planePoints, &centerX, &centerY, &radius);
    Circle<float> candidateCircle(Vector2<float>(centerX, centerY), radius);
    /*
     *  We have to make a circle candidate
     *  then we remove all obvious points
     *  and calculate the possibility if
     *  all the points seem to be a circle.
     *
     */

    if (CircleIsValid(iterations, candidateCircle) && ControlCircleBorder(candidateCircle) > 0.9){
            printf("Correct\n");
            foundCircleData.circle.center = candidateCircle.center;
            foundCircleData.circle.radius = candidateCircle.radius;

            GenerateCircleSurroundPoints(candidateCircle);
            double possibility= ControlCircleBorder(candidateCircle);

            /*printf("Amount: %f | Iteration: %d | CenterX: %f | CenterY: %f | Radius: %f \n"
                    ,possibility, iterations, centerX, centerY, radius);*/

            printf("Amount: %f\n"
                    ,possibility);
    }
    else{
        foundCircleData.circle.radius = -1;
    }
}

void MiddleCircleDetection::createCircleData() {
}



bool MiddleCircleDetection::CircleIsValid(int iterationAmount, Circle<float> circle) {
    const double RADIUS_TOLERANCE = 0.2;
    const int MIN_DETECT_POINTS_AMOUNT = 10;

    if(iterationAmount == -1 ||
       circle.radius < (fieldDimensions_->fieldCenterCircleDiameter / 2 - fieldDimensions_->fieldCenterCircleDiameter / 2 * RADIUS_TOLERANCE) ||
       circle.radius > (fieldDimensions_->fieldCenterCircleDiameter / 2 + fieldDimensions_->fieldCenterCircleDiameter / 2 * RADIUS_TOLERANCE) ||
       middleCirclePoints_.size() < MIN_DETECT_POINTS_AMOUNT
    ){
        return false;
    }

    Vector2i pixelCoordsCenter;
    if(!(cameraMatrix_->robotToPixel(circle.center, pixelCoordsCenter))){
        foundCircleData.circle.radius=-1;
        return false;
    }
    Segment segment;
    return imageSegments_->verticalSegmentAt(pixelCoordsCenter, segment) && !(segment.startEdgeType != EdgeType::RISING || segment.endEdgeType != EdgeType::FALLING);

}

double MiddleCircleDetection::ControlCircleBorder(Circle<float> circle) {
    const double RADIUS_TOLERANCE = 0.15;
    double amount = 0;
    VecVector2f planePoints;
    PixelToRobot(middleCirclePoints_, planePoints);
    for(Vector2f point : planePoints){
        double dist = GetVectorDistanceff(circle.center, point);

        if(!(dist < circle.radius-(circle.radius*RADIUS_TOLERANCE) || dist > circle.radius+(circle.radius*RADIUS_TOLERANCE)) ){
            amount++;
        }
    }
    printf("%f",amount/planePoints.size());
    return amount/planePoints.size();
}

double MiddleCircleDetection::GetVectorDistancefi(Vector2f firstVec, Vector2i secondVec){
    double xDif = secondVec.x() - firstVec.x();
    double yDif = secondVec.y() - firstVec.y();

    return abs(sqrt((xDif*xDif)+(yDif*yDif)));
}

double MiddleCircleDetection::GetVectorDistanceff(Vector2f firstVec, Vector2f secondVec){
    double xDif = secondVec.x() - firstVec.x();
    double yDif = secondVec.y() - firstVec.y();

    return abs(sqrt((xDif*xDif)+(yDif*yDif)));
}

/*
 * This Method creates the
 * points to draw on screen for the
 * circle appearance
 */
void MiddleCircleDetection::GenerateCircleSurroundPoints(Circle<float> circle) {
    circleBorderPoints_.clear();

    for (double angle = -180; angle < 181; angle++) {
        float x = circle.radius*cos(angle* M_PI/180) + foundCircleData.circle.center.x();
        float y = circle.radius*sin(angle* M_PI/180) + foundCircleData.circle.center.y();

        circleBorderPoints_.push_back(Vector2f(x,y));
    }
}

void MiddleCircleDetection::PixelToRobot(VecVector2i screenPoints, VecVector2f &planePoints){
    Vector2f planePoint;

    for(Vector2i point : screenPoints){
        if (cameraMatrix_->pixelToRobot(point, planePoint)){
            planePoints.push_back(planePoint);
        }
    }
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
        if (foundCircleData.circle.radius != -1) {


            Vector2i pixelCoords;
            for (Vector2f point : circleBorderPoints_) {
                // both (x,y) and (x,-y) are points on the half-circle
                Vector2f circlePoint(point);
                if (cameraMatrix_->robotToPixel(circlePoint, pixelCoords)) {
                    image.circle(Image422::get444From422Vector(pixelCoords), 5, Color::RED);
                }
            }

            /// Draw middle Point
            Vector2f centerPoint(foundCircleData.circle.center);
            if (cameraMatrix_->robotToPixel(centerPoint, pixelCoords)) {
                image.cross(Image422::get444From422Vector(pixelCoords), 10, Color::YELLOW);
            }
        }
        else{
            //printf("no radius");
        }
        debug().sendImage(mount_ + "." + imageData_->identification + "_image_circle", image);

    }
}