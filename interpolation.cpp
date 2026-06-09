#include "interpolation.h"
#include <QtMath>

QVector<Interpolation::Step> Interpolation::linear(double xe, double ye, double stepSize)
{
    QVector<Step> steps;
    if (xe == 0 && ye == 0)
        return steps;

    double xi = 0, yi = 0;
    double F = 0; // F = Yi*Xe - Xi*Ye, initially 0

    double xDir = (xe >= 0) ? 1.0 : -1.0;
    double yDir = (ye >= 0) ? 1.0 : -1.0;

    // Q1/Q3 (same sign): F>=0 → step X, F<0 → step Y
    // Q2/Q4 (diff sign): F>=0 → step Y, F<0 → step X
    bool sameSign = (xe >= 0) == (ye >= 0);

    int maxSteps = static_cast<int>((qAbs(xe) + qAbs(ye)) / stepSize) + 100;

    for (int i = 0; i < maxSteps; ++i) {
        if (qAbs(xi - xe) <= 1e-6 && qAbs(yi - ye) <= 1e-6)
            break;

        steps.append({QPointF(xi, yi), F});

        bool xDone = qAbs(xi - xe) <= 1e-6;
        bool yDone = qAbs(yi - ye) <= 1e-6;

        // Decide whether to step X or Y
        bool stepX;
        if (xDone) {
            stepX = false;
        } else if (yDone) {
            stepX = true;
        } else if (sameSign) {
            stepX = (F >= 0);
        } else {
            stepX = (F < 0);
        }

        if (stepX) {
            xi += stepSize * xDir;
            if ((xDir > 0 && xi > xe) || (xDir < 0 && xi < xe))
                xi = xe;
        } else {
            yi += stepSize * yDir;
            if ((yDir > 0 && yi > ye) || (yDir < 0 && yi < ye))
                yi = ye;
        }
        F = yi * xe - xi * ye;
    }

    // Append final point
    steps.append({QPointF(xe, ye), 0.0});
    return steps;
}

double Interpolation::normalizeAngle(double rad)
{
    rad = fmod(rad, 2.0 * M_PI);
    if (rad < 0)
        rad += 2.0 * M_PI;
    return rad;
}

QVector<Interpolation::Step> Interpolation::circularArc(
    double xc, double yc, double radius,
    double startAngleDeg, double endAngleDeg,
    bool clockwise, double stepSize)
{
    QVector<Step> steps;

    double startRad = degToRad(startAngleDeg);
    double endRad = degToRad(endAngleDeg);

    // Starting point on circle (relative to center)
    double relX = radius * cos(startRad);
    double relY = radius * sin(startRad);

    // Deviation: F = relX^2 + relY^2 - R^2
    double F = relX * relX + relY * relY - radius * radius;

    // End point (relative to center)
    double endRelX = radius * cos(endRad);
    double endRelY = radius * sin(endRad);

    int maxSteps = static_cast<int>(2.0 * M_PI * radius / stepSize) + 1000;
    bool reached = false;

    for (int i = 0; i < maxSteps; ++i) {
        steps.append({QPointF(xc + relX, yc + relY), F});

        // Check if we've reached the end point (after at least a few steps)
        if (i > 5) {
            double dx = relX - endRelX;
            double dy = relY - endRelY;
            if (dx * dx + dy * dy <= stepSize * stepSize * 2.0) {
                reached = true;
                break;
            }
        }

        // Quadrant-aware stepping for 逐点比较法
        // Quadrant determined by sign of (relX, relY) relative to center
        if (clockwise) {
            // CW: Q1→-Y/+X, Q2→+X/+Y, Q3→+Y/-X, Q4→-X/-Y
            if (relX >= 0 && relY >= 0) {
                // Q1: tangent direction is (+X, -Y)
                if (F >= 0) {
                    F += -2.0 * relY * stepSize + stepSize * stepSize;
                    relY -= stepSize;
                } else {
                    F += 2.0 * relX * stepSize + stepSize * stepSize;
                    relX += stepSize;
                }
            } else if (relX < 0 && relY >= 0) {
                // Q2: tangent direction is (+X, +Y)
                if (F >= 0) {
                    F += 2.0 * relX * stepSize + stepSize * stepSize;
                    relX += stepSize;
                } else {
                    F += 2.0 * relY * stepSize + stepSize * stepSize;
                    relY += stepSize;
                }
            } else if (relX < 0 && relY < 0) {
                // Q3: tangent direction is (-X, +Y)
                if (F >= 0) {
                    F += 2.0 * relY * stepSize + stepSize * stepSize;
                    relY += stepSize;
                } else {
                    F += -2.0 * relX * stepSize + stepSize * stepSize;
                    relX -= stepSize;
                }
            } else {
                // Q4: tangent direction is (-X, -Y)
                if (F >= 0) {
                    F += -2.0 * relX * stepSize + stepSize * stepSize;
                    relX -= stepSize;
                } else {
                    F += -2.0 * relY * stepSize + stepSize * stepSize;
                    relY -= stepSize;
                }
            }
        } else {
            // CCW: Q1→-X/+Y, Q2→-Y/-X, Q3→+X/-Y, Q4→+Y/+X
            if (relX >= 0 && relY >= 0) {
                // Q1: tangent direction is (-X, +Y)
                if (F >= 0) {
                    F += -2.0 * relX * stepSize + stepSize * stepSize;
                    relX -= stepSize;
                } else {
                    F += 2.0 * relY * stepSize + stepSize * stepSize;
                    relY += stepSize;
                }
            } else if (relX < 0 && relY >= 0) {
                // Q2: tangent direction is (-X, -Y)
                if (F >= 0) {
                    F += -2.0 * relY * stepSize + stepSize * stepSize;
                    relY -= stepSize;
                } else {
                    F += -2.0 * relX * stepSize + stepSize * stepSize;
                    relX -= stepSize;
                }
            } else if (relX < 0 && relY < 0) {
                // Q3: tangent direction is (+X, -Y)
                if (F >= 0) {
                    F += 2.0 * relX * stepSize + stepSize * stepSize;
                    relX += stepSize;
                } else {
                    F += -2.0 * relY * stepSize + stepSize * stepSize;
                    relY -= stepSize;
                }
            } else {
                // Q4: tangent direction is (+X, +Y)
                if (F >= 0) {
                    F += 2.0 * relY * stepSize + stepSize * stepSize;
                    relY += stepSize;
                } else {
                    F += 2.0 * relX * stepSize + stepSize * stepSize;
                    relX += stepSize;
                }
            }
        }
    }

    // Append final point
    if (!reached) {
        steps.append({QPointF(xc + endRelX, yc + endRelY), 0.0});
    } else {
        steps.append({QPointF(xc + endRelX, yc + endRelY), 0.0});
    }

    return steps;
}
