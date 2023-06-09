
#include "mainwindow.h"
#include "ui_mainwindow.h"

// Must be double (write 80.0 instead of 80)
#define POINT_COUNT 80.0

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->monot->setEnabled(false);
    inverse = false;

    // Setup data for plot
    x = QVector<double>(101);
    y = QVector<double>(101);

    // Information on plot
    ui->graph->legend->setVisible(true);

    // Main graph and dots
    ui->graph->addGraph();
    ui->graph->addGraph();
    ui->graph->addGraph();
    ui->graph->addGraph();
    ui->graph->addGraph(ui->graph->yAxis, ui->graph->xAxis);

    ui->graph->graph(0)->setPen(QColor(255, 0, 0, 100));
    ui->graph->graph(0)->setName("f(x) = sin(x) – x^2/2");

    // Make graph look like dots
    ui->graph->graph(1)->setPen(QColor(0, 150, 255, 255));
    ui->graph->graph(1)->setLineStyle(QCPGraph::lsNone);
    ui->graph->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    ui->graph->graph(1)->setName("Ближайшие узлы");

    ui->graph->graph(2)->setPen(QColor(0, 0, 0, 255));
    ui->graph->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->graph->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
    ui->graph->graph(2)->setName("Остальные узлы");

    // Point of interpolation
    ui->graph->graph(3)->setPen(QColor(255, 0, 0, 255));
    ui->graph->graph(3)->setLineStyle(QCPGraph::lsNone);
    ui->graph->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 6));
    ui->graph->graph(3)->setName("Точка интерпол.");

    ui->graph->graph(4)->setPen(QColor(0, 0, 255, 100));
    ui->graph->graph(4)->setName("Обратная ф.");


    // Additional properties
    ui->graph->xAxis->setLabel("x");
    ui->graph->yAxis->setLabel("y");
    ui->graph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    // Layout
    ui->graph->xAxis->setRange(ui->rightBound->value(), ui->leftBound->value());
    replot();
    rescaleAndMatch();
}

void MainWindow::rescaleAndMatch() {
    ui->graph->graph(0)->rescaleValueAxis();
    double currentStart = ui->graph->xAxis->range().lower;
    ui->graph->yAxis->setRange(currentStart, currentStart + ui->graph->xAxis->range().size());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::calculate() {
    inverse = ui->inverse->isChecked();
    ui->graph->graph(4)->setVisible(inverse);

    const double x0 = ui->xInter->value();

    // Values validation
    const int nodesCount = ui->nodesCount->value();
    const int power = ui->power->value();
    if (power >= nodesCount) {
        ui->power->setStyleSheet("border: 1px solid red;");
        return;
    } else {
        ui->power->setStyleSheet("");
    }

    const double a = ui->leftBound->value();
    const double b = ui->rightBound->value();
    if (a >= b) {
        ui->rightBound->setStyleSheet("border: 1px solid red;");
        ui->leftBound->setStyleSheet("border: 1px solid red;");
        return;
    } else {
        ui->rightBound->setStyleSheet("");
        ui->leftBound->setStyleSheet("");
    }


    // Dots
    auto table = generateTable(a, b, nodesCount, inverse && ui->monot->isChecked(), func);

    if (inverse && ui->monot->isChecked() && !isMonot(table)) {
        alert("Функция не строго монотонна на заданном отрезке, уберите "
              "галочку монотонности или выберите другой интервал",
              QMessageBox::Warning);
        return;
    }

    // Sort by distance from x0
    sort(table.begin(), table.end(), CloserTo(x0));

    // Display all known points
    QVector<double> x1, y1;
    ui->nodes->clear();
    for (int i = 0; i < power + 1; ++i) {
        x1.push_back(table.at(i).first);
        y1.push_back(table.at(i).second);
        ui->nodes->addItem(QString::fromStdString("X =  ") +
                           QString::number(table.at(i).first) +
                           QString::fromStdString("; Y =  ") +
                           QString::number(table.at(i).second));
    }
    ui->graph->graph(1)->setData(x1, y1);
    x1.clear();
    y1.clear();
    ui->nodes->addItem(QString::fromStdString(" "));
    for (uint16_t i = power + 1; i < table.size(); ++i) {
        x1.push_back(table.at(i).first);
        y1.push_back(table.at(i).second);
        ui->nodes->addItem(QString::fromStdString("X =  ") +
                           QString::number(table.at(i).first) +
                           QString::fromStdString("; Y =  ") +
                           QString::number(table.at(i).second));
    }
    ui->graph->graph(2)->setData(x1, y1);
    x1.clear();
    y1.clear();

    // Interpolation itself
    if (inverse && !ui->monot->isChecked()) {
        auto roots = findFirstRoot(a, b, power, table, func, x0, std::pow(10, -8));
        if (roots.size() == 0) {
            replot();
            alert(QString::fromStdString("Обратная функция не определена в точке ")
                      + QString::number(x0),
                  QMessageBox::Warning);
            return;
        }
        QString rootsText = QString::fromStdString("Корни f(x) = y:\n");
        for (auto root : roots) {
            x1.push_back(x0);
            y1.push_back(root.first);
            rootsText += QString::number(root.first) +
                         QString::fromStdString(" (Невязка = ") +
                         QString::number(root.second)+
                         QString::fromStdString(")\n");
        }
        ui->result->setText(rootsText);
    } else {
        auto result1 = lagrange(x0, table, power, func, inverse);
        auto result2 = newton(x0, table, power, func, inverse);
        ui->result->setText(QString::fromStdString("Форма Лагранжа: ") + QString::number(result1.first) +
                            QString::fromStdString("\nПогрешность: ") + QString::number(result1.second) +
                            QString::fromStdString("\nФорма Ньютона: ") + QString::number(result2.first) +
                            QString::fromStdString("\nПогрешность: ") + QString::number(result2.second));
        x1.push_back(x0);
        y1.push_back(result1.first);
    }

    // Plotting result
    ui->graph->graph(3)->setData(x1, y1);
    replot();
}

void MainWindow::replot() {
    auto axisX = ui->graph->xAxis->range();
    auto axisY = ui->graph->yAxis->range();

    // Main graph
    for (int i=0; i < POINT_COUNT+1; ++i)
    {
        x[i] = axisX.lower + i/POINT_COUNT * axisX.size();
        y[i] = sin(x[i]) - x[i]*x[i]/2;
    }
    ui->graph->graph(0)->setData(x, y);

    if (inverse) {
        for (int i=0; i < POINT_COUNT+1; ++i)
        {
            x[i] = axisY.lower + i/POINT_COUNT * axisY.size();
            y[i] = sin(x[i]) - x[i]*x[i]/2;
        }
        ui->graph->graph(4)->setData(x, y);
    }

    ui->graph->replot();
}

void MainWindow::inverseOnToggle() {
    ui->monot->setEnabled(!ui->monot->isEnabled());
}

void MainWindow::alert(const QString prompt, QMessageBox::Icon icon)
{
    msgBox.setText(prompt);
    msgBox.setIcon(icon);
    msgBox.exec();
}
