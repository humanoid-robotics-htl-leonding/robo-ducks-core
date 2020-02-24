//
// Created by marc on 2/18/20.
//

#ifndef TUHHNAO_CIRCLEFITTER_H
#define TUHHNAO_CIRCLEFITTER_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

typedef struct {
    double x, y;
} Point2;

class CircleFitter {
public:

    void display();
    void reshape(int w, int h);
    void addPoint(double x, double y);
    void mouse(int button, int state, int x, int y);
    void menu(int value);
    int circleFit(int N, VecVector2f P, double *pa, double *pb, double *pr);
    void drawCircle(float x, float y, float r2);

};


#endif //TUHHNAO_CIRCLEFITTER_H
