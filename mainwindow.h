#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QVector>
#include "canvas.h"
#include "interpolation.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onInterpolationTypeChanged();
    void onDrawContour();
    void onStartInterpolation();
    void onClear();
    void onArcInputMethodChanged(int index);

private:
    QWidget *createControlPanel();

    // Helper: generate ideal path for display
    QVector<QPointF> generateIdealLinearPath(double xe, double ye, int numPoints);
    QVector<QPointF> generateIdealArcPath(double xc, double yc, double radius,
                                           double startAngle, double endAngle,
                                           bool clockwise, int numPoints);

    Canvas *m_canvas;

    // Interpolation type radio buttons
    QRadioButton *m_radioLinear;
    QRadioButton *m_radioArcCW;
    QRadioButton *m_radioArcCCW;

    // Step size
    QComboBox *m_comboStep;

    // Linear input
    QGroupBox *m_groupLinear;
    QDoubleSpinBox *m_spinXe;
    QDoubleSpinBox *m_spinYe;

    // Arc input
    QGroupBox *m_groupArc;
    QDoubleSpinBox *m_spinRadius;
    QDoubleSpinBox *m_spinStartAngle;
    QComboBox *m_comboArcMethod;
    QDoubleSpinBox *m_spinEndAngle;    // method 1: end angle
    QDoubleSpinBox *m_spinRotAngle;    // method 2: rotation angle
    QLabel *m_labelEndAngle;
    QLabel *m_labelRotAngle;

    // Current interpolation data
    QVector<Interpolation::Step> m_currentSteps;
    QVector<QPointF> m_currentIdealPath;
};

#endif // MAINWINDOW_H
