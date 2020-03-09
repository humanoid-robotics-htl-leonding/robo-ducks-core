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
        , fieldDimensions_(*this)
        , cameraMatrix_(*this)
        , circleData_(*this)
{
}

class Data
{
public:

    VecVector2f data;
    Vector2f mean;


    Data(VecVector2f dataXY);

    // routines
    void means();
    void center();

};


/************************************************************************
			BODY OF THE MEMBER ROUTINES
************************************************************************/

Data::Data(VecVector2f dataXY)
{
    data = dataXY;
}

// Routine that computes the x- and y- sample means (the coordinates of the centeroid)
void Data::means()
{
    mean = {0, 0};

    for (auto d : data) {
    	mean += d;
    }

    mean /= data.size();
}

// Routine that centers the data set (shifts the coordinates to the centeroid)
void Data::center()
{
    means();

    for (auto d : data)
    {
        d -= mean;
    }

    mean = {0, 0};
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
        printf("SizePoint: %zu\n",debugMiddleCirclePoints_.size());
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


Circle<float> circleFitByHyper(Data &data)
{
    int iter,IterMAX=99;

    Vector3f i;
    Eigen::Matrix<float, 2, 3> M;
    float Mz,Cov_xy,Var_z;
    float A0,A1,A2,A22;
    float Dy,xnew,x,ynew,y;
    float DET,Xcenter,Ycenter;

    MiddleCircle circle;

    data.means();   // Compute x- and y- sample means (via a function in the class "data")

//     computing moments

    for (auto d : data.data)
    {
    	i.x() = d.x() - data.mean.x();
    	i.y() = d.y() - data.mean.y();
    	i.z() = i.x() * i.x() + i.y() * i.y();

    	M(0, 0) += i.x() * i.x();
    	M(0, 1) += i.x() * i.y();
    	M(0, 2) += i.x() * i.z();
    	M(1, 0) += i.y() * i.y();
    	M(1, 1) += i.y() * i.z();
    	M(1, 2) += i.z() * i.z();
    }
   	M /= data.data.size();

//    computing the coefficients of the characteristic polynomial

    Mz = M(0, 0) + M(1, 0);
    Cov_xy = M(0, 0) * M(1, 0) - M(0, 1) * M(0, 1);
    Var_z = M(1, 2) - Mz * Mz;

    A2 = 4.0*Cov_xy - 3.0*Mz*Mz - M(1, 2);
    A1 = Var_z*Mz + 4.0*Cov_xy*Mz - M(0, 2) * M(0, 2) - M(1, 1) * M(1, 1);
    A0 = M(0, 2) *(M(0, 2) * M(1, 0) - M(1, 1) * M(0, 0)) + M(1, 1) * (M(1, 1) * M(0, 0) - M(0, 2) * M(0, 1)) - Var_z*Cov_xy;
    A22 = A2 + A2;

//    finding the root of the characteristic polynomial
//    using Newton's method starting at x=0
//     (it is guaranteed to converge to the right root)

    for (x=0.,y=A0,iter=0; iter<IterMAX; iter++)  // usually, 4-6 iterations are enough
    {
        Dy = A1 + x*(A22 + 16.*x*x);
        xnew = x - y/Dy;
        if ((xnew == x)||(!finite(xnew))) break;
        ynew = A0 + xnew*(A1 + xnew*(A2 + 4.0*xnew*xnew));
        if (abs(ynew)>=abs(y))  break;
        x = xnew;  y = ynew;
    }

//    computing paramters of the fitting circle

    DET = x*x - x*Mz + Cov_xy;
    Xcenter = (M(0, 2) * (M(1, 0) - x) - M(1, 1) * M(0, 1))/DET/2.0;
    Ycenter = (M(1, 1) * (M(0, 0) - x) - M(0, 2) * M(0, 1))/DET/2.0;

//       assembling the output

    circle.a = Xcenter + data.mean.x();
    circle.b = Ycenter + data.mean.y();
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

    Data currentData(planePoints);
    Circle<float> candidateCircle = circleFitByHyper(currentData);
    int iterations = 200;


    if (circleIsValid(iterations, candidateCircle) && controlCircleBorder(candidateCircle) > 0.9){
        foundCircleData.circle.center = candidateCircle.center;
            foundCircleData.circle.radius = candidateCircle.radius;

        generateCircleSurroundPoints(candidateCircle);

    }
    else{
        foundCircleData.circle.radius = -1;
    }
}


bool MiddleCircleDetection::circleIsValid(int iterationAmount, Circle<float> circle) {
    const double RADIUS_TOLERANCE = 0.2;
    const int MIN_DETECT_POINTS_AMOUNT = 10;

    if(iterationAmount == -1 ||
       circle.radius < (fieldDimensions_->fieldCenterCircleDiameter / 2 - fieldDimensions_->fieldCenterCircleDiameter / 2 * RADIUS_TOLERANCE) ||
       circle.radius > (fieldDimensions_->fieldCenterCircleDiameter / 2 + fieldDimensions_->fieldCenterCircleDiameter / 2 * RADIUS_TOLERANCE) ||
       middleCirclePoints_.size() < MIN_DETECT_POINTS_AMOUNT
    ){
        printf("Iterations:%d\nAmount:%zu \nRadius:%f\n",iterationAmount,middleCirclePoints_.size(),circle.radius);
        return false;
    }

    Vector2i pixelCoordsCenter;
    if(!(cameraMatrix_->robotToPixel(circle.center, pixelCoordsCenter))){
        foundCircleData.circle.radius=-1;
        printf("2\n");
        return false;
    }
    Segment segment;
    return imageSegments_->verticalSegmentAt(pixelCoordsCenter, segment) && !(segment.startEdgeType != EdgeType::RISING || segment.endEdgeType != EdgeType::FALLING);

}

double MiddleCircleDetection::controlCircleBorder(Circle<float> circle) {
    const double RADIUS_TOLERANCE = 0.15;
    double amount = 0;
    VecVector2f planePoints;
    pixelToRobot(middleCirclePoints_, planePoints);
    for(Vector2f point : planePoints){
        double dist = getVectorDistanceff(circle.center, point);

        if(!(dist < circle.radius-(circle.radius*RADIUS_TOLERANCE) || dist > circle.radius+(circle.radius*RADIUS_TOLERANCE)) ){
            amount++;
        }
    }
    return amount/planePoints.size();
}

double MiddleCircleDetection::getVectorDistancefi(Vector2f firstVec, Vector2i secondVec){
    double xDif = secondVec.x() - firstVec.x();
    double yDif = secondVec.y() - firstVec.y();

    return abs(sqrt((xDif*xDif)+(yDif*yDif)));
}

double MiddleCircleDetection::getVectorDistanceff(Vector2f firstVec, Vector2f secondVec){
    double xDif = secondVec.x() - firstVec.x();
    double yDif = secondVec.y() - firstVec.y();

    return abs(sqrt((xDif*xDif)+(yDif*yDif)));
}

/*
 * This Method creates the
 * points to draw on screen for the
 * circle appearance
 */
void MiddleCircleDetection::generateCircleSurroundPoints(Circle<float> circle) {
    circleBorderPoints_.clear();

    for (double angle = -180; angle < 181; angle++) {
        float x = circle.radius*cos(angle* M_PI/180) + foundCircleData.circle.center.x();
        float y = circle.radius*sin(angle* M_PI/180) + foundCircleData.circle.center.y();

        circleBorderPoints_.push_back(Vector2f(x,y));
    }
}

void MiddleCircleDetection::pixelToRobot(VecVector2i screenPoints, VecVector2f &planePoints){
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


            /// Draw circleBorderPoints_
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
        }
        debug().sendImage(mount_ + "." + imageData_->identification + "_image_circle", image);

    }
}

