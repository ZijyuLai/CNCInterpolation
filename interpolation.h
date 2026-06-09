#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include <QVector>
#include <QPointF>
#include <cmath>

class Interpolation
{
public:
    enum Type { Linear, ArcCW, ArcCCW };

    struct Step {
        QPointF pos;      // math coordinates
        double deviation; // current deviation value
    };

    // Linear interpolation: start at (0,0), end at (xe, ye)
    static QVector<Step> linear(double xe, double ye, double stepSize);

    // Circular arc interpolation
    // angles in degrees, math coordinate system (CCW positive)
    static QVector<Step> circularArc(double xc, double yc, double radius,
                                     double startAngleDeg, double endAngleDeg,
                                     bool clockwise, double stepSize);

private:
    static double degToRad(double deg) { return deg * M_PI / 180.0; }
    static double normalizeAngle(double rad);
};

#endif // INTERPOLATION_H
