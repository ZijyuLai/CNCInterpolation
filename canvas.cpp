#include "canvas.h"
#include <QPainterPath>
#include <QtMath>

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
    , m_currentStep(0)
    , m_maxCoord(500)
    , m_scale(1.0)
    , m_hasData(false)
{
    setMinimumSize(400, 400);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(60, 60, 60));
    setPalette(pal);

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(60); // ~60ms per step
    connect(m_animTimer, &QTimer::timeout, this, &Canvas::onAnimationTick);
}

void Canvas::clearCanvas()
{
    m_animTimer->stop();
    m_idealPath.clear();
    m_interpPath.clear();
    m_interpSteps.clear();
    m_currentStep = 0;
    m_hasData = false;
    update();
}

void Canvas::setIdealPath(const QVector<QPointF> &path)
{
    m_idealPath = path;
    m_hasData = true;
    update();
}

void Canvas::startInterpolation(const QVector<Interpolation::Step> &steps)
{
    m_interpSteps = steps;
    m_interpPath.clear();
    m_currentStep = 0;
    m_hasData = true;

    if (!m_interpSteps.isEmpty()) {
        m_interpPath.append(m_interpSteps[0].pos);
        m_currentStep = 1;
        m_animTimer->start();
    }
    update();
}

void Canvas::stopAnimation()
{
    m_animTimer->stop();
}

void Canvas::setDataRange(double maxCoord)
{
    m_maxCoord = maxCoord;
    update();
}

void Canvas::onAnimationTick()
{
    if (m_currentStep < m_interpSteps.size()) {
        m_interpPath.append(m_interpSteps[m_currentStep].pos);
        m_currentStep++;
        update();
    } else {
        m_animTimer->stop();
    }
}

QPointF Canvas::toScreen(const QPointF &mathPt) const
{
    // Math origin is at bottom-left area of canvas
    // Screen: Y goes down, so we flip Y
    double sx = m_origin.x() + mathPt.x() * m_scale;
    double sy = m_origin.y() - mathPt.y() * m_scale;
    return QPointF(sx, sy);
}

void Canvas::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Compute scale and origin - center origin for all quadrants
    double margin = 40.0;
    double drawW = width() - 2 * margin;
    double drawH = height() - 2 * margin;
    m_scale = qMin(drawW, drawH) / (2.0 * (m_maxCoord > 0 ? m_maxCoord : 500));
    m_origin = QPointF(width() / 2.0, height() / 2.0);

    drawBackground(painter);
    drawAxes(painter);

    if (m_hasData) {
        drawIdealPath(painter);
        drawInterpolationPath(painter);
        drawCurrentPosition(painter);
    }
}

void Canvas::drawBackground(QPainter &painter)
{
    painter.fillRect(rect(), QColor(60, 60, 60));
}

void Canvas::drawAxes(QPainter &painter)
{
    painter.setPen(QPen(Qt::white, 1.5));

    // X axis: from left edge to right edge through origin
    QPointF xLeft(10, m_origin.y());
    QPointF xRight(width() - 10, m_origin.y());
    painter.drawLine(xLeft, xRight);
    // Right arrow
    painter.drawLine(xRight, QPointF(xRight.x() - 8, xRight.y() - 4));
    painter.drawLine(xRight, QPointF(xRight.x() - 8, xRight.y() + 4));
    painter.drawText(QPointF(xRight.x() + 2, xRight.y() - 5), "X");

    // Y axis: from bottom to top through origin (Y up in math = up on screen)
    QPointF yBottom(m_origin.x(), height() - 10);
    QPointF yTop(m_origin.x(), 10);
    painter.drawLine(yBottom, yTop);
    // Top arrow
    painter.drawLine(yTop, QPointF(yTop.x() - 4, yTop.y() + 8));
    painter.drawLine(yTop, QPointF(yTop.x() + 4, yTop.y() + 8));
    painter.drawText(QPointF(yTop.x() + 8, yTop.y() + 5), "Y");

    // Draw tick marks and labels for all quadrants
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    painter.setPen(QPen(QColor(180, 180, 180), 1));

    double tickStep = 50;
    if (m_maxCoord > 1000) tickStep = 200;
    else if (m_maxCoord > 500) tickStep = 100;

    for (double v = -m_maxCoord; v <= m_maxCoord; v += tickStep) {
        if (qAbs(v) < 1e-6) continue; // skip origin
        // X ticks
        QPointF pt = toScreen(QPointF(v, 0));
        if (pt.x() > 10 && pt.x() < width() - 10) {
            painter.drawLine(QPointF(pt.x(), m_origin.y() - 3),
                             QPointF(pt.x(), m_origin.y() + 3));
            painter.drawText(QPointF(pt.x() - 10, m_origin.y() + 15),
                             QString::number(v, 'g', 4));
        }
        // Y ticks
        QPointF ptY = toScreen(QPointF(0, v));
        if (ptY.y() > 10 && ptY.y() < height() - 10) {
            painter.drawLine(QPointF(m_origin.x() - 3, ptY.y()),
                             QPointF(m_origin.x() + 3, ptY.y()));
            painter.drawText(QPointF(m_origin.x() - 35, ptY.y() + 4),
                             QString::number(v, 'g', 4));
        }
    }
}

void Canvas::drawIdealPath(QPainter &painter)
{
    if (m_idealPath.size() < 2)
        return;

    painter.setPen(QPen(QColor(220, 60, 60), 2)); // Red, thin line
    QPainterPath path;
    QPointF first = toScreen(m_idealPath[0]);
    path.moveTo(first);
    for (int i = 1; i < m_idealPath.size(); ++i) {
        path.lineTo(toScreen(m_idealPath[i]));
    }
    painter.drawPath(path);
}

void Canvas::drawInterpolationPath(QPainter &painter)
{
    if (m_interpPath.size() < 2)
        return;

    painter.setPen(QPen(QColor(80, 220, 80), 3)); // Green, thicker line
    QPainterPath path;
    QPointF first = toScreen(m_interpPath[0]);
    path.moveTo(first);
    for (int i = 1; i < m_interpPath.size(); ++i) {
        path.lineTo(toScreen(m_interpPath[i]));
    }
    painter.drawPath(path);
}

void Canvas::drawCurrentPosition(QPainter &painter)
{
    if (m_interpPath.isEmpty())
        return;

    QPointF pos = toScreen(m_interpPath.last());
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 0)); // Yellow dot
    painter.drawEllipse(pos, 4, 4);
}
