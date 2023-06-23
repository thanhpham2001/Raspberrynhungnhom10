#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
     //Khởi tạo Serial và kết nối USB với cam bien
    ui->setupUi(this);
    COMPORT = new QSerialPort();
    COMPORT->setPortName("ttyUSB0");
    COMPORT->setBaudRate(QSerialPort::Baud115200);
    COMPORT->setParity(QSerialPort::NoParity);
    COMPORT->setDataBits(QSerialPort::DataBits::Data8);
    COMPORT->setStopBits(QSerialPort::StopBits::OneStop);
    COMPORT->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
    COMPORT->open(QIODevice::ReadWrite);

    if(COMPORT->isOpen())
    {
        qDebug() << "Serial Port is Connected";
        qDebug() << COMPORT->error();
    }
    else
    {
        qDebug() << "Serial Port is No Connected";
        qDebug() << COMPORT->error();
    }
    connect(COMPORT,SIGNAL(readyRead()),this,SLOT(Read_Data()));

    //************************************************
    //Khoi tạo đồ thi ECG
    // Tạo dối tượng Qchart
    chart = new QChart();

     // Tạo dữ liệu mẫu cho đồ thị
     series = new QLineSeries();
     chart->addSeries(series);

        // Tạo trục x, y cho đồ thị
     QDateTimeAxis *xAxis = new QDateTimeAxis();
     QValueAxis *yAxis = new QValueAxis();
     yAxis->setRange(0,5000); // Set gia tri mac dinh cho truc
     chart->addAxis(xAxis, Qt::AlignBottom);
     chart->addAxis(yAxis, Qt::AlignLeft);
     series->attachAxis(xAxis);
     series->attachAxis(yAxis);

        //Tạo QChartView đe hien thi du lieu
     QChartView *chartView = new QChartView(chart);
     chartView->setRenderHint(QPainter::Antialiasing);

        // Ðat QChartView vào QLabel lable_3
    // label = new QLabel(this);
     QVBoxLayout *layout = new QVBoxLayout(ui->label_3);


     layout->addWidget(chartView);
     layout->setContentsMargins(0, 0, 0, 0);  // Xóa padding
     layout->setSpacing(0);  // Xóa khoang cách
     chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
     ui->label_3->setLayout(layout);


        // Thêm QLabel vào lable_3 trong file .ui
      //ui->label_3->layout()->addWidget(label);

        // Cap nhat du lieu tho gian
      //QTimer *timer = new QTimer(this);
      //connect(timer, SIGNAL(timeout()), this, SLOT(updateChart()));
      //timer->start(1000); // Cap nhat du lieu moi day
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Read_Data()
{

    if(COMPORT->isOpen())
    {
        while(COMPORT->bytesAvailable())
        {
            Dara_From_SerialPort += COMPORT->readAll();
            if(Dara_From_SerialPort.at(Dara_From_SerialPort.length() - 1) == char(10))
            {
                IS_Data_Recevied = true;
            }
        }

        if(IS_Data_Recevied == true)
        {
            //qDebug() << "Data From Serial Port: " << Dara_From_SerialPort;
            //ui->textEdit2->setText(Dara_From_SerialPort);
            processJsonString(Dara_From_SerialPort);
            Dara_From_SerialPort = "";
            IS_Data_Recevied = false;
        }
    }


}

void MainWindow::on_pushButton_clicked()
{
    if (COMPORT->isOpen()) {
        // Ghi chuoi vao cong Serial
        QByteArray data = "{\"SPO2\":\"1\"}";
        COMPORT->write(data);
        COMPORT->flush();  // Dam bao du lieu duoc gui di ngay lap tuc

    } else {
        qDebug() << "Khong the mo cong Serial.";
    }
}


void MainWindow::on_pushButton_2_clicked()
{
    if (COMPORT->isOpen()) {
        // Ghi chuoi vao cong Serial
        QByteArray data = "{\"ECG\":\"1\"}";
        COMPORT->write(data);
        COMPORT->flush();  // Dam bao du lieu duoc gui di ngay lap tuc

    } else {
        qDebug() << "Khong the mo cong Serial.";
    }
}

void MainWindow::processJsonString(const QString& jsonString)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);

    if (error.error == QJsonParseError::NoError) {
        if (jsonDoc.isObject()) {
            QJsonObject jsonObj = jsonDoc.object();

            // Doi gia tri tung bien thanh so
            if (jsonObj.contains("SPO2") && jsonObj["SPO2"].toInt()) {
                int spo2Value = jsonObj["SPO2"].toInt();
                 qDebug() << "SpO2:" << spo2Value;
                  ui->SPO2->setText(QString::number(spo2Value));
                }
                else {
                    qDebug() << "Loi khi chuyen doi gia tri";
                }

            if (jsonObj.contains("heartRate") && jsonObj["heartRate"].toInt()) {
                int nhiptimValue = jsonObj["heartRate"].toInt();
                    qDebug() << "Nhiptim:" << nhiptimValue;
                    ui->heartRate->setText(QString::number(nhiptimValue));

                } else {
                    qDebug() << "Loi khi chuyen doi gia tri";
                }

            if (jsonObj.contains("ECG") && jsonObj["ECG"].toInt()) {
                int ecgValue = jsonObj["ECG"].toInt();
                    qDebug() << "ECG:" << ecgValue;
                    //ui->label_3->setText(QString::number(ecgValue));
                    updateChart(ecgValue);
                } else {
                    //qDebug() << "Loi khi chuyen doi gia tri ECG?";
                }
        }
    } else {
        //qDebug() << "Loi khi phan tich chuoi JSON" << error.    errorString();
    }
}

void MainWindow::updateChart(int& ECG_val)
{
    //Lay gia tri ECG de ve do thi
    int yValue = ECG_val;

    // Lay du lieu thoi giani
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // Them gia tri vao do thi
    series->append(currentDateTime.toMSecsSinceEpoch(), yValue);

    // Cap nhat truc X cho do thi
    QtCharts::QDateTimeAxis *xAxis = dynamic_cast<QtCharts::QDateTimeAxis *>(chart->axisX());
    if (xAxis) {
        xAxis->setMax(currentDateTime);
        xAxis->setMin(currentDateTime.addSecs(-10)); // Giu gia tri trong khoang 10s
    }

    // Cap nhat do thi
    chart->update();
}
