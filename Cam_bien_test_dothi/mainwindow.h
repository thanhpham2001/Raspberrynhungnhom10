#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QMessageBox>
#include <QString>
#include <QSerialPort>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QChar>
#include <QChartView>
#include <QLineSeries>
#include <QDateTime>
#include <QValueAxis>
#include <QtCharts>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void Read_Data();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void updateChart(int& ECG_val);

private:
    Ui::MainWindow *ui;
    QSerialPort* COMPORT;
    QString Dara_From_SerialPort;
    bool IS_Data_Recevied = false;
    void processJsonString(const QString& jsonString);
    QChart *chart;
    QLineSeries *series;
    QLabel *label;
};
#endif // MAINWINDOW_H
