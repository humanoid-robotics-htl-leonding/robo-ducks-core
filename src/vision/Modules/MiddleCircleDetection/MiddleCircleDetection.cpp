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

class Data
{
public:

    int n;
    float *X;		//space is allocated in the constructors
    float *Y;		//space is allocated in the constructors
    float meanX, meanY;

    // constructors
    Data();
    Data(int N);
    Data(int N, float X[], float Y[]);

    // routines
    void means(void);
    void center(void);
    void scale(void);
    void print(void);

    // destructors
    ~Data();
};


/************************************************************************
			BODY OF THE MEMBER ROUTINES
************************************************************************/
// Default constructor
Data::Data()
{
    n=0;
    X = new float[n];
    Y = new float[n];
    for (int i=0; i<n; i++)
    {
        X[i]=0.;
        Y[i]=0.;
    }
}

// Constructor with assignment of the field N
Data::Data(int N)
{
    n=N;
    X = new float[n];
    Y = new float[n];

    for (int i=0; i<n; i++)
    {
        X[i]=0.;
        Y[i]=0.;
    }
}

// Constructor with assignment of each field
Data::Data(int N, float dataX[], float dataY[])
{
    n=N;
    X = new float[n];
    Y = new float[n];

    for (int i=0; i<n; i++)
    {
        X[i]=dataX[i];
        Y[i]=dataY[i];
    }
}

// Routine that computes the x- and y- sample means (the coordinates of the centeroid)

void Data::means(void)
{
    meanX=0.; meanY=0.;

    for (int i=0; i<n; i++)
    {
        meanX += X[i];
        meanY += Y[i];
    }
    meanX /= n;
    meanY /= n;
}

// Routine that centers the data set (shifts the coordinates to the centeroid)

void Data::center(void)
{
    float sX=0.,sY=0.;
    int i;

    for (i=0; i<n; i++)
    {
        sX += X[i];
        sY += Y[i];
    }
    sX /= n;
    sY /= n;

    for (i=0; i<n; i++)
    {
        X[i] -= sX;
        Y[i] -= sY;
    }
    meanX = 0.;
    meanY = 0.;
}

// Routine that scales the coordinates (makes them of order one)

void Data::scale(void)
{
    float sXX=0.,sYY=0.,scaling;
    int i;

    for (i=0; i<n; i++)
    {
        sXX += X[i]*X[i];
        sYY += Y[i]*Y[i];
    }
    scaling = sqrt((sXX+sYY)/n/2.0);

    for (i=0; i<n; i++)
    {
        X[i] /= scaling;
        Y[i] /= scaling;
    }
}

// Destructor
Data::~Data()
{
    delete[] X;
    delete[] Y;
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

    /*
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
    }*/
}

void MiddleCircleDetection::cycle() {
    if (!filteredSegments_->valid) {
        return;
    } else {
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
    MiddleCircle(float aa, float bb, float rr);

    // routines
    void print(void);

    // no destructor we didn't allocate memory by hand.
};


/************************************************************************
			BODY OF THE MEMBER ROUTINES
************************************************************************/
// Default constructor

MiddleCircle::MiddleCircle()
{
    a=0.; b=0.; r=1.; s=0.; i=0; j=0;
}

// Constructor with assignment of the circle parameters only

MiddleCircle::MiddleCircle(float aa, float bb, float rr)
{
    a=aa; b=bb; r=rr;
}


Circle<float> circleFitByHyper(Data &data)
/*
      Circle fit to a given set of data points (in 2D)

      This is an algebraic fit based on the journal article

      A. Al-Sharadqah and N. Chernov, "Error analysis for circle fitting algorithms",
      Electronic Journal of Statistics, Vol. 3, pages 886-911, (2009)

      It is an algebraic circle fit with "hyperaccuracy" (with zero essential bias).
      The term "hyperaccuracy" first appeared in papers by Kenichi Kanatani around 2006

      Input:  data     - the class of data (contains the given points):

	      data.n   - the number of data points
	      data.X[] - the array of X-coordinates
	      data.Y[] - the array of Y-coordinates

     Output:
               circle - parameters of the fitting circle:

	       circle.a - the X-coordinate of the center of the fitting circle
	       circle.b - the Y-coordinate of the center of the fitting circle
 	       circle.r - the radius of the fitting circle
 	       circle.s - the root mean square error (the estimate of sigma)
 	       circle.j - the total number of iterations

     This method combines the Pratt and Taubin fits to eliminate the essential bias.

     It works well whether data points are sampled along an entire circle or
     along a small arc.

     Its statistical accuracy is theoretically higher than that of the Pratt fit
     and Taubin fit, but practically they all return almost identical circles
     (unlike the Kasa fit that may be grossly inaccurate).

     It provides a very good initial guess for a subsequent geometric fit.

       Nikolai Chernov  (September 2012)

*/
{
    int i,iter,IterMAX=99;

    float Xi,Yi,Zi;
    float Mz,Mxy,Mxx,Myy,Mxz,Myz,Mzz,Cov_xy,Var_z;
    float A0,A1,A2,A22;
    float Dy,xnew,x,ynew,y;
    float DET,Xcenter,Ycenter;

    MiddleCircle circle;

    data.means();   // Compute x- and y- sample means (via a function in the class "data")

//     computing moments

    Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;

    for (i=0; i<data.n; i++)
    {
        Xi = data.X[i] - data.meanX;   //  centered x-coordinates
        Yi = data.Y[i] - data.meanY;   //  centered y-coordinates
        Zi = Xi*Xi + Yi*Yi;

        Mxy += Xi*Yi;
        Mxx += Xi*Xi;
        Myy += Yi*Yi;
        Mxz += Xi*Zi;
        Myz += Yi*Zi;
        Mzz += Zi*Zi;
    }
    Mxx /= data.n;
    Myy /= data.n;
    Mxy /= data.n;
    Mxz /= data.n;
    Myz /= data.n;
    Mzz /= data.n;

//    computing the coefficients of the characteristic polynomial

    Mz = Mxx + Myy;
    Cov_xy = Mxx*Myy - Mxy*Mxy;
    Var_z = Mzz - Mz*Mz;

    A2 = 4.0*Cov_xy - 3.0*Mz*Mz - Mzz;
    A1 = Var_z*Mz + 4.0*Cov_xy*Mz - Mxz*Mxz - Myz*Myz;
    A0 = Mxz*(Mxz*Myy - Myz*Mxy) + Myz*(Myz*Mxx - Mxz*Mxy) - Var_z*Cov_xy;
    A22 = A2 + A2;

//    finding the root of the characteristic polynomial
//    using Newton's method starting at x=0
//     (it is guaranteed to converge to the right root)

    for (x=0.,y=A0,iter=0; iter<IterMAX; iter++)  // usually, 4-6 iterations are enough
    {
        Dy = A1 + x*(A22 + 16.*x*x);
        xnew = x - y/Dy;
        if ((xnew == x)||(!isfinite(xnew))) break;
        ynew = A0 + xnew*(A1 + xnew*(A2 + 4.0*xnew*xnew));
        if (abs(ynew)>=abs(y))  break;
        x = xnew;  y = ynew;
    }

//    computing paramters of the fitting circle

    DET = x*x - x*Mz + Cov_xy;
    Xcenter = (Mxz*(Myy - x) - Myz*Mxy)/DET/2.0;
    Ycenter = (Myz*(Mxx - x) - Mxz*Mxy)/DET/2.0;

//       assembling the output

    circle.a = Xcenter + data.meanX;
    circle.b = Ycenter + data.meanY;
    circle.r = sqrt(Xcenter*Xcenter + Ycenter*Ycenter + Mz - x - x);
    //circle.s = Sigma(data,circle);
    circle.i = 0;
    circle.j = iter;  //  return the number of iterations, too

    Circle<float> resultCircle(Vector2f(circle.a,circle.b),circle.r);

    return resultCircle;
}


void MiddleCircleDetection::initCorrectCircle() {
    /*double centerX;
    double centerY;
    double radius;*/



    VecVector2f planePoints;
    Vector2f planePoint;

    pixelToRobot(middleCirclePoints_, planePoints);

    /*int iterations = circleFitter_.circleFit(planePoints.size(), planePoints, &centerX, &centerY, &radius);
    Circle<float> candidateCircle(Vector2<float>(centerX, centerY), radius);*/

    float xData[1000];
    float yData[1000];
    Vector2f currentPoint;
    for (int i = 0; i < (int)planePoints.size(); ++i) {
        currentPoint=planePoints.at(i);
        xData[i]=currentPoint.x();
        yData[i]=currentPoint.y();
    }

    Data currentData(planePoints.size(),xData, yData);
    Circle<float> candidateCircle = circleFitByHyper(currentData);
    int iterations = 200;
    double amount;


    if (circleIsValid(iterations, candidateCircle) && (amount= controlCircleBorder(candidateCircle)) > 0.9){
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

