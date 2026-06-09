#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QVector>
#include <QPointF>
#include "interpolation.h"

class Canvas : public QWidget
{
    Q_OBJECT

public:
    explicit Canvas(QWidget *parent = nullptr);

    void clearCanvas();

    // Set ideal path (smooth curve)
    void setIdealPath(const QVector<QPointF> &path);

    // Start interpolation animation
    void startInterpolation(const QVector<Interpolation::Step> &steps);

    // Stop animation
    void stopAnimation();

    // Set the data range for auto-scaling
    void setDataRange(double maxCoord);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onAnimationTick();

private:
    void drawBackground(QPainter &painter);
    void drawAxes(QPainter &painter);
    void drawIdealPath(QPainter &painter);
    void drawInterpolationPath(QPainter &painter);
    void drawCurrentPosition(QPainter &painter);

    // Coordinate transform: math → screen
    QPointF toScreen(const QPointF &mathPt) const;

    QVector<QPointF> m_idealPath;
    QVector<QPointF> m_interpPath;  // accumulated interpolation points
    QVector<Interpolation::Step> m_interpSteps;
    int m_currentStep;
    double m_maxCoord;
    double m_scale;
    QPointF m_origin;  // screen position of math origin

    QTimer *m_animTimer;
    bool m_hasData;
};

#endif // CANVAS_H
