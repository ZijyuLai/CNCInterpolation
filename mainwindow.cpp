#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    // Left: Canvas
    m_canvas = new Canvas(this);
    mainLayout->addWidget(m_canvas, 1);

    // Right: Control panel
    mainLayout->addWidget(createControlPanel(), 0);

    setCentralWidget(central);

    // Default state
    onInterpolationTypeChanged();
    onArcInputMethodChanged(0);
}

QWidget *MainWindow::createControlPanel()
{
    QWidget *panel = new QWidget(this);
    QVBoxLayout *panelLayout = new QVBoxLayout(panel);

    // === Group 1: Basic Parameters ===
    QGroupBox *groupBasic = new QGroupBox(tr("基本参数"));
    QVBoxLayout *basicLayout = new QVBoxLayout(groupBasic);

    m_radioLinear = new QRadioButton(tr("直线插补"));
    m_radioArcCW = new QRadioButton(tr("圆弧插补（顺时针）"));
    m_radioArcCCW = new QRadioButton(tr("圆弧插补（逆时针）"));
    m_radioLinear->setChecked(true);

    basicLayout->addWidget(m_radioLinear);
    basicLayout->addWidget(m_radioArcCW);
    basicLayout->addWidget(m_radioArcCCW);

    QHBoxLayout *stepLayout = new QHBoxLayout;
    stepLayout->addWidget(new QLabel(tr("步长：")));
    m_comboStep = new QComboBox;
    m_comboStep->addItems({"0.1", "0.2", "0.5", "1.0", "2.0", "5.0"});
    m_comboStep->setCurrentText("1.0");
    stepLayout->addWidget(m_comboStep);
    basicLayout->addLayout(stepLayout);

    panelLayout->addWidget(groupBasic);

    // === Group 2: Linear Endpoint ===
    m_groupLinear = new QGroupBox(tr("终点坐标"));
    QFormLayout *linearLayout = new QFormLayout(m_groupLinear);

    m_spinXe = new QDoubleSpinBox;
    m_spinXe->setRange(-10000, 10000);
    m_spinXe->setValue(400);
    m_spinXe->setDecimals(1);
    linearLayout->addRow("X:", m_spinXe);

    m_spinYe = new QDoubleSpinBox;
    m_spinYe->setRange(-10000, 10000);
    m_spinYe->setValue(300);
    m_spinYe->setDecimals(1);
    linearLayout->addRow("Y:", m_spinYe);

    panelLayout->addWidget(m_groupLinear);

    // === Group 3: Arc Parameters ===
    m_groupArc = new QGroupBox(tr("圆弧参数"));
    QFormLayout *arcLayout = new QFormLayout(m_groupArc);

    m_spinRadius = new QDoubleSpinBox;
    m_spinRadius->setRange(1, 10000);
    m_spinRadius->setValue(200);
    m_spinRadius->setDecimals(1);
    arcLayout->addRow(tr("半径："), m_spinRadius);

    m_spinStartAngle = new QDoubleSpinBox;
    m_spinStartAngle->setRange(0, 360);
    m_spinStartAngle->setValue(0);
    m_spinStartAngle->setSuffix(tr("°"));
    arcLayout->addRow(tr("起始角："), m_spinStartAngle);

    // Arc input method switch
    m_comboArcMethod = new QComboBox;
    m_comboArcMethod->addItem(tr("方法1：起始角 + 终止角"));
    m_comboArcMethod->addItem(tr("方法2：起始角 + 转过角度"));
    arcLayout->addRow(tr("输入方法："), m_comboArcMethod);

    // Method 1: end angle
    m_labelEndAngle = new QLabel(tr("终止角："));
    m_spinEndAngle = new QDoubleSpinBox;
    m_spinEndAngle->setRange(0, 360);
    m_spinEndAngle->setValue(90);
    m_spinEndAngle->setSuffix(tr("°"));
    arcLayout->addRow(m_labelEndAngle, m_spinEndAngle);

    // Method 2: rotation angle
    m_labelRotAngle = new QLabel(tr("转过角度："));
    m_spinRotAngle = new QDoubleSpinBox;
    m_spinRotAngle->setRange(-360, 360);
    m_spinRotAngle->setValue(90);
    m_spinRotAngle->setSuffix(tr("°"));
    arcLayout->addRow(m_labelRotAngle, m_spinRotAngle);

    panelLayout->addWidget(m_groupArc);

    // === Control Buttons ===
    QGroupBox *groupButtons = new QGroupBox(tr("控制"));
    QVBoxLayout *btnLayout = new QVBoxLayout(groupButtons);

    QPushButton *btnDraw = new QPushButton(tr("绘制轮廓"));
    QPushButton *btnClear = new QPushButton(tr("清屏"));
    QPushButton *btnStart = new QPushButton(tr("开始插补"));
    QPushButton *btnEnd = new QPushButton(tr("结束"));

    btnLayout->addWidget(btnDraw);
    btnLayout->addWidget(btnClear);
    btnLayout->addWidget(btnStart);
    btnLayout->addWidget(btnEnd);

    panelLayout->addWidget(groupButtons);
    panelLayout->addStretch();

    // === Connections ===
    connect(m_radioLinear, &QRadioButton::toggled, this, &MainWindow::onInterpolationTypeChanged);
    connect(m_radioArcCW, &QRadioButton::toggled, this, &MainWindow::onInterpolationTypeChanged);
    connect(m_radioArcCCW, &QRadioButton::toggled, this, &MainWindow::onInterpolationTypeChanged);
    connect(m_comboArcMethod, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onArcInputMethodChanged);
    connect(btnDraw, &QPushButton::clicked, this, &MainWindow::onDrawContour);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onClear);
    connect(btnStart, &QPushButton::clicked, this, &MainWindow::onStartInterpolation);
    connect(btnEnd, &QPushButton::clicked, this, &QWidget::close);

    return panel;
}

void MainWindow::onInterpolationTypeChanged()
{
    bool isLinear = m_radioLinear->isChecked();
    m_groupLinear->setVisible(isLinear);
    m_groupArc->setVisible(!isLinear);
}

void MainWindow::onArcInputMethodChanged(int index)
{
    bool isMethod1 = (index == 0);
    m_labelEndAngle->setVisible(isMethod1);
    m_spinEndAngle->setVisible(isMethod1);
    m_labelRotAngle->setVisible(!isMethod1);
    m_spinRotAngle->setVisible(!isMethod1);
}

void MainWindow::onDrawContour()
{
    m_canvas->clearCanvas();

    if (m_radioLinear->isChecked()) {
        double xe = m_spinXe->value();
        double ye = m_spinYe->value();

        m_currentIdealPath = generateIdealLinearPath(xe, ye, 200);

        double maxCoord = qMax(qAbs(xe), qAbs(ye)) * 1.2;
        m_canvas->setDataRange(maxCoord);
        m_canvas->setIdealPath(m_currentIdealPath);

    } else {
        double radius = m_spinRadius->value();
        double startAngle = m_spinStartAngle->value();
        double endAngle;

        if (m_comboArcMethod->currentIndex() == 0) {
            endAngle = m_spinEndAngle->value();
        } else {
            double rotAngle = m_spinRotAngle->value();
            if (m_radioArcCW->isChecked()) {
                endAngle = startAngle - rotAngle;
            } else {
                endAngle = startAngle + rotAngle;
            }
        }

        bool clockwise = m_radioArcCW->isChecked();
        double xc = 0, yc = 0;

        m_currentIdealPath = generateIdealArcPath(xc, yc, radius,
                                                    startAngle, endAngle,
                                                    clockwise, 200);

        double maxCoord = radius * 1.3;
        m_canvas->setDataRange(maxCoord);
        m_canvas->setIdealPath(m_currentIdealPath);
    }
}

void MainWindow::onStartInterpolation()
{
    double stepSize = m_comboStep->currentText().toDouble();

    if (m_radioLinear->isChecked()) {
        double xe = m_spinXe->value();
        double ye = m_spinYe->value();

        m_currentSteps = Interpolation::linear(xe, ye, stepSize);

        double maxCoord = qMax(qAbs(xe), qAbs(ye)) * 1.2;
        m_canvas->setDataRange(maxCoord);

    } else {
        double radius = m_spinRadius->value();
        double startAngle = m_spinStartAngle->value();
        double endAngle;

        if (m_comboArcMethod->currentIndex() == 0) {
            endAngle = m_spinEndAngle->value();
        } else {
            double rotAngle = m_spinRotAngle->value();
            if (m_radioArcCW->isChecked()) {
                endAngle = startAngle - rotAngle;
            } else {
                endAngle = startAngle + rotAngle;
            }
        }

        bool clockwise = m_radioArcCW->isChecked();
        double xc = 0, yc = 0;

        m_currentSteps = Interpolation::circularArc(xc, yc, radius,
                                                      startAngle, endAngle,
                                                      clockwise, stepSize);

        double maxCoord = radius * 1.3;
        m_canvas->setDataRange(maxCoord);
    }

    m_canvas->startInterpolation(m_currentSteps);
}

void MainWindow::onClear()
{
    m_canvas->clearCanvas();
    m_currentSteps.clear();
    m_currentIdealPath.clear();
}

QVector<QPointF> MainWindow::generateIdealLinearPath(double xe, double ye, int numPoints)
{
    QVector<QPointF> path;
    for (int i = 0; i <= numPoints; ++i) {
        double t = static_cast<double>(i) / numPoints;
        path.append(QPointF(t * xe, t * ye));
    }
    return path;
}

QVector<QPointF> MainWindow::generateIdealArcPath(
    double xc, double yc, double radius,
    double startAngleDeg, double endAngleDeg,
    bool clockwise, int numPoints)
{
    QVector<QPointF> path;

    double startRad = startAngleDeg * M_PI / 180.0;
    double endRad = endAngleDeg * M_PI / 180.0;

    // Compute total arc angle
    double arcAngle;
    if (clockwise) {
        arcAngle = startRad - endRad;
        if (arcAngle < 0) arcAngle += 2.0 * M_PI;
    } else {
        arcAngle = endRad - startRad;
        if (arcAngle < 0) arcAngle += 2.0 * M_PI;
    }

    for (int i = 0; i <= numPoints; ++i) {
        double t = static_cast<double>(i) / numPoints;
        double angle;
        if (clockwise) {
            angle = startRad - t * arcAngle;
        } else {
            angle = startRad + t * arcAngle;
        }
        path.append(QPointF(xc + radius * cos(angle),
                            yc + radius * sin(angle)));
    }
    return path;
}
