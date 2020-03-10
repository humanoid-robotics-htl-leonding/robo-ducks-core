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
        , radiusTolerance_(*this, "radiusTolerance", [] {})
        , imageData_(*this)
        , fieldDimensions_(*this)
        , cameraMatrix_(*this)
        , circleData_(*this) {
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
        middleCirclePoints_.push_back((segment->start+segment->end).unaryExpr(shift));
    }
}

void MiddleCircleDetection::cycle() {

    if (filteredSegments_->valid) {
        detectMiddleCirclePoints();
        debugMiddleCirclePoints_ = middleCirclePoints_;
        initCorrectCircle();
    }

    sendImagesForDebug();
}

class MiddleCircle
{
    // Class for Circle
// A circle has 7 fields:
//     a, b, r (of type reals), the circle parameters
//     s (of type reals), the estimate of sigma (standard deviation)
//     g (of type reals), the norm of the gradient of the objective function
//
public:

    // The fields of a Circle
    float a, b, r, s, g, Gx, Gy;
    int i, j;

    // constructors
    MiddleCircle();
};


/************************************************************************
			BODY OF THE MEMBER ROUTINES
************************************************************************/
// Default constructor

MiddleCircle::MiddleCircle()
{
    a=0.; b=0.; r=1.; s=0.; i=0; j=0;
}


Circle<float> circleFitByHyper(const VecVector2f& points)
{
    int iter,IterMAX=99;

    Vector3f i;
    Eigen::Matrix<float, 2, 3> M;
    M.fill(0);
    float Mz,Cov_xy,Var_z;
    float A0,A1,A2,A22;
    float Dy,xnew,x,ynew,y;
    float DET,Xcenter,Ycenter;

    MiddleCircle circle;

    // Compute x- and y- sample means
	Vector2f mean = {0, 0};

	for (auto point : points) {
		mean += point;
	}

	mean /= points.size();

//     computing moments

    for (auto point : points)
    {
    	i.x() = point.x() - mean.x();
    	i.y() = point.y() - mean.y();
    	i.z() = i.x() * i.x() + i.y() * i.y();

    	M(0, 0) += i.x() * i.x();
    	M(0, 1) += i.x() * i.y();
    	M(0, 2) += i.x() * i.z();
    	M(1, 0) += i.y() * i.y();
    	M(1, 1) += i.y() * i.z();
    	M(1, 2) += i.z() * i.z();
    }
   	M /= points.size();

//    computing the coefficients of the characteristic polynomial

    Mz = M(0, 0) + M(1, 0);
    Cov_xy = M(0, 0) * M(1, 0) - M(0, 1) * M(0, 1);
    Var_z = M(1, 2) - Mz * Mz;

    A2 = 4.0*Cov_xy - 3.0*Mz*Mz - M(1, 2);
    A1 = Var_z*Mz + 4.0*Cov_xy*Mz - M(0, 2) * M(0, 2) - M(1, 1) * M(1, 1);
    A0 = M(0, 2) *(M(0, 2) * M(1, 0) - M(1, 1) * M(0, 1)) + M(1, 1) * (M(1, 1) * M(0, 0) - M(0, 2) * M(0, 1)) - Var_z*Cov_xy;
    A22 = A2 + A2;

//    finding the root of the characteristic polynomial
//    using Newton's method starting at x=0
//     (it is guaranteed to converge to the right root)

    for (x=0.,y=A0,iter=0; iter<IterMAX; iter++)  // usually, 4-6 iterations are enough
    {
        Dy = A1 + x*(A22 + 16.*x*x);
        xnew = x - y/Dy;
        if (xnew == x || !finite(xnew)) break;
        ynew = A0 + xnew*(A1 + xnew*(A2 + 4.0*xnew*xnew));
        if (abs(ynew)>=abs(y))  break;
        x = xnew;  y = ynew;
    }

//    computing paramters of the fitting circle

    DET = x*x - x*Mz + Cov_xy;
    Xcenter = (M(0, 2) * (M(1, 0) - x) - M(1, 1) * M(0, 1))/DET/2.0;
    Ycenter = (M(1, 1) * (M(0, 0) - x) - M(0, 2) * M(0, 1))/DET/2.0;

//       assembling the output

    circle.a = Xcenter + mean.x();
    circle.b = Ycenter + mean.y();
    circle.r = sqrt(Xcenter*Xcenter + Ycenter*Ycenter + Mz - x - x);
    //circle.s = Sigma(data,circle);
    circle.i = 0;
    circle.j = iter;  //  return the number of iterations, too

    Circle<float> resultCircle(Vector2f(circle.a,circle.b),circle.r);

    return resultCircle;
}


void MiddleCircleDetection::initCorrectCircle() {
    VecVector2f planePoints;
    Vector2f planePoint;

    pixelToRobot(middleCirclePoints_, planePoints);

    Circle<float> candidateCircle = circleFitByHyper(planePoints);

    if (circleIsValid(candidateCircle) && controlCircleBorder(candidateCircle) > 0.75){
        foundCircleData.circle.center = candidateCircle.center;
            foundCircleData.circle.radius = candidateCircle.radius;

        generateCircleSurroundPoints(candidateCircle);

    }
    else{
        foundCircleData.circle.radius = -1;
    }
}


bool MiddleCircleDetection::circleIsValid(const Circle<float>& circle) {
    const int MIN_DETECT_POINTS_AMOUNT = 10;

    if(circle.radius < (fieldDimensions_->fieldCenterCircleDiameter / 2 - fieldDimensions_->fieldCenterCircleDiameter / 2 * radiusTolerance_()) ||
       circle.radius > (fieldDimensions_->fieldCenterCircleDiameter / 2 + fieldDimensions_->fieldCenterCircleDiameter / 2 * radiusTolerance_()) ||
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
    if (imageSegments_->verticalSegmentAt(pixelCoordsCenter, segment)) {
    	return segment.startEdgeType == EdgeType::RISING && segment.endEdgeType == EdgeType::FALLING;
    }
    return true;

}

double MiddleCircleDetection::controlCircleBorder(const Circle<float>& circle) {
    double amount = 0;
    VecVector2f planePoints;
    pixelToRobot(middleCirclePoints_, planePoints);
    for(auto& point : planePoints){
        double dist = (point - circle.center).norm();

        if(!(dist < circle.radius-(circle.radius*radiusTolerance_()) || dist > circle.radius+(circle.radius*radiusTolerance_())) ){
            amount++;
        }
    }
    return amount/planePoints.size();
}

/*
 * This Method creates the
 * points to draw on screen for the
 * circle appearance
 */
void MiddleCircleDetection::generateCircleSurroundPoints(const Circle<float>& circle) {
    circleBorderPoints_.clear();

    for (int angle = -180; angle < 181; angle++) {
        float x = circle.radius*cos(angle * M_PI / 180) + foundCircleData.circle.center.x();
        float y = circle.radius*sin(angle * M_PI / 180) + foundCircleData.circle.center.y();

        circleBorderPoints_.push_back(Vector2f(x,y));
    }
}

void MiddleCircleDetection::pixelToRobot(const VecVector2i& screenPoints, VecVector2f &planePoints) const {
    Vector2f planePoint;

    for(auto& point : screenPoints){
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


            /// Draw circleBorderPoints_
            Vector2i pixelCoords;
            for (const Vector2f point : circleBorderPoints_) {
                // both (x,y) and (x,-y) are points on the half-circle
                if (cameraMatrix_->robotToPixel(point, pixelCoords)) {
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
        }
        debug().sendImage(mount_ + "." + imageData_->identification + "_image_circle", image);

    }
}

