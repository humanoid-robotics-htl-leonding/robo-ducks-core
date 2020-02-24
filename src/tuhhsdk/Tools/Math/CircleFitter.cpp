//
// Created by marc on 2/18/20.
//

#include "CircleFitter.hpp"

/* This program is freely distributable without licensing fees
   and is provided without guarantee or warrantee expressed or
   implied. This program is -not- in the public domain. */

/* This is a small interactive demo of Dave Eberly's algorithm
   that fits a circle boundary to a set of 2D points. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
/// sudo apt-get install freeglut3-dev

/* Some <math.h> files do not define M_PI... */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


/****************************************************************************
   Least squares fit of circle to set of points.
   by Dave Eberly (eberly@cs.unc.edu or eberly@ndl.com)
   ftp://ftp.cs.unc.edu/pub/users/eberly/magic/circfit.c
  ---------------------------------------------------------------------------
   Input:  (x_i,y_i), 1 <= i <= N, where N >= 3 and not all points
           are collinear
   Output:  circle center (a,b) and radius r

   Energy function to be minimized is

      E(a,b,r) = sum_{i=1}^N (L_i-r)^2

   where L_i = |(x_i-a,y_i-b)|, the length of the specified vector.
   Taking partial derivatives and setting equal to zero yield the
   three nonlinear equations

   E_r = 0:  r = Average(L_i)
   E_a = 0:  a = Average(x_i) + r * Average(dL_i/da)
   E_b = 0:  b = Average(y_i) + r * Average(dL_i/db)

   Replacing r in the last two equations yields

     a = Average(x_i) + Average(L_i) * Average(dL_i/da) = F(a,b)
     b = Average(y_i) + Average(L_i) * Average(dL_i/db) = G(a,b)

   which can possibly be solved by fixed point iteration as

     a_{n+1} = F(a_n,b_n),  b_{n+a} = G(a_n,b_n)

   with initial guess a_0 = Average(x_i) and b_0 = Average(y_i).
   Derivative calculations show that

     dL_i/da = (a-x_i)/L_i,  dL_i/db = (b-y_i)/L_i.

  ---------------------------------------------------------------------------
   WARNING.  I have not analyzed the convergence properties of the fixed
   point iteration scheme.  In a few experiments it seems to converge
   just fine, but I do not guarantee convergence in all cases.
 ****************************************************************************/

int
CircleFitter::circleFit(int N, VecVector2f P, double *pa, double *pb, double *pr)
{
    /* user-selected parameters */
    const int maxIterations = 256;
    const double tolerance = 1e-06;

    double a, b, r;

    /* compute the average of the data points */
    int i, j;
    double xAvr = 0.0;
    double yAvr = 0.0;

    for (i = 0; i < N; i++) {
        xAvr += P.at(i).x();
        yAvr += P.at(i).y();
    }
    xAvr /= N;
    yAvr /= N;

    /* initial guess */
    a = xAvr;
    b = yAvr;

    for (j = 0; j < maxIterations; j++) {
        /* update the iterates */
        double a0 = a;
        double b0 = b;

        /* compute average L, dL/da, dL/db */
        double LAvr = 0.0;
        double LaAvr = 0.0;
        double LbAvr = 0.0;

        for (i = 0; i < N; i++) {
            double dx = P.at(i).x() - a;
            double dy = P.at(i).y() - b;
            double L = sqrt(dx * dx + dy * dy);
            if (fabs(L) > tolerance) {
                LAvr += L;
                LaAvr -= dx / L;
                LbAvr -= dy / L;
            }
        }
        LAvr /= N;
        LaAvr /= N;
        LbAvr /= N;

        a = xAvr + LAvr * LaAvr;
        b = yAvr + LAvr * LbAvr;
        r = LAvr;

        if (fabs(a - a0) <= tolerance && fabs(b - b0) <= tolerance)
            break;
    }

    *pa = a;
    *pb = b;
    *pr = r;

    return (j < maxIterations ? j : -1);
}

enum {
    M_SHOW_CIRCLE, M_CIRCLE_INFO, M_RESET_POINTS, M_QUIT
};

#define MAX_POINTS 100

int num = 0;
VecVector2i list/*[MAX_POINTS]*/;
int circleFitNeedsRecalc = 0;
int showCircle = 1;
int circleInfo = 0;
int windowHeight;
double a, b, r = 0.0;   /* X, Y, and radius of best fit circle.
                         */

void
CircleFitter::drawCircle(float x, float y, float r2)
{
    double angle;

    glPushMatrix();
    glTranslatef(x, y, 0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (angle = 0.0; angle <= 2 * M_PI; angle += M_PI / 24) {
        glVertex2f(r2 * cos(angle), r2 * sin(angle));
    }
    glEnd();
    glPopMatrix();
}


void
CircleFitter::reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    windowHeight = h;
}

void
CircleFitter::addPoint(double x, double y)
{
    if (num + 1 >= MAX_POINTS) {
        fprintf(stderr, "circlefit: limited to only %d points\n", MAX_POINTS);
        return;
    }
    list.at(num).x() = x;
    list.at(num).y() = y;
    num++;
    circleFitNeedsRecalc = 1;
    glutPostRedisplay();
}

void
CircleFitter::mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        addPoint(x, windowHeight - y);
    }
}

void
CircleFitter::menu(int value)
{
    switch (value) {
        case M_SHOW_CIRCLE:
            showCircle = !showCircle;
            break;
        case M_CIRCLE_INFO:
            circleInfo = !circleInfo;
            break;
        case M_RESET_POINTS:
            num = 0;
            r = 0.0;
            break;
        case M_QUIT:
            exit(0);
    }
    glutPostRedisplay();
}

/*int
main(int argc, char **argv)
{
    glutInitWindowSize(400, 400);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutCreateWindow("Least squares fit of circle to set of points");

    printf("\n");
    printf("Least squares fit of circle to set of points\n");
    printf("--------------------------------------------\n");
    printf("Click left mouse button to position points.  The\n");
    printf("program then shows the circle whose boundary best\n");
    printf("fits the set of points specified.  Try clicking\n");
    printf("points in a near circle.\n");
    printf("\n");

    glClearColor(125.0 / 256.0, 158.0 / 256.0, 192.0 / 256.0, 1.0);
    glPointSize(3.0);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutDisplayFunc(display);
    glutCreateMenu(menu);
    glutAddMenuEntry("Show/hide circle", M_SHOW_CIRCLE);
    glutAddMenuEntry("Toggle info printing", M_CIRCLE_INFO);
    glutAddMenuEntry("Reset points", M_RESET_POINTS);
    glutAddMenuEntry("Quit", M_QUIT);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutMainLoop();
    return 0;              ANSI C requires main to return int.
}*/