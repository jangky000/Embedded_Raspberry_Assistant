#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMainWindow>
#include<QTimer>
#include<QDateTime>

#include <qfile.h>
#include <QFileSystemWatcher>
#include<QtDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //타이머
    QTimer* timer = new QTimer(this); // 타이머 생성
    connect(timer, SIGNAL(timeout()), this, SLOT(tupdate())); // 타이머가 타임아웃이 되면 tupdate()를 호출
    timer->start(1000); // 1000은 밀리세컨드 단위. 타임아웃을 몇 ms로 할것인지 정함. 여기서는 1초마다 한번씩.

    //파일
    QString ApplicationPath=QApplication::applicationDirPath();
    //qDebug() << ApplicationPath;

    //text 파일 변경 감지
    QFileSystemWatcher *responsewatcher = new QFileSystemWatcher;
    //histwatcher->addPath(ApplicationPath+"/history.txt");
    responsewatcher->addPath(ApplicationPath+"/schedule.txt");
    responsewatcher->addPath(ApplicationPath+"/news.txt");
    responsewatcher->addPath(ApplicationPath+"/weather.txt");
    QObject::connect(responsewatcher,SIGNAL(fileChanged(QString)),this,SLOT(responseModified(QString)));

    QFileSystemWatcher *histwatcher = new QFileSystemWatcher;
    histwatcher->addPath(ApplicationPath+"/history.txt");
    QObject::connect(histwatcher,SIGNAL(fileChanged(QString)),this,SLOT(histModified(QString)));

    QFileSystemWatcher *calenderwatcher = new QFileSystemWatcher;
    calenderwatcher->addPath(ApplicationPath+"/calender.txt");
    QObject::connect(calenderwatcher,SIGNAL(fileChanged(QString)),this,SLOT(calenderModified(QString)));


    //Write(ApplicationPath+"/test.txt", "helloworld\nasdas\nasdasd");
    QDate calDate = ui->calendarWidget->selectedDate();
    calenderRead(ApplicationPath+"/calender.txt", calDate.year(), calDate.month());
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::tupdate()
{
    QDateTime local(QDateTime::currentDateTime());
    ui->time->setText("Time: "+local.time().toString());
    return 0;
}

void MainWindow::responseRead(QString path)
{
    QFile file(path); // path 형식 "C://dir//hi.txt"
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "not open file";
        return;
    } else {
            QTextStream stream(&file);
            ui->response->setHtml(stream.readAll());
            file.close();
    }
}

void MainWindow::histRead(QString path)
{
    QFile file(path); // path 형식 "C://dir//hi.txt"
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "not open file";
        return;
    } else {
            QTextStream stream(&file);
            ui->history->setText(stream.readAll());
            ui->history->moveCursor(QTextCursor::End);
            file.close();
    }
}

void MainWindow::calenderRead(QString path, int year, int month)
{
    QFile file(path); // path 형식 "C://dir//hi.txt"
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "not open file";
        return;
    } else {
            QTextStream stream(&file);

            QString ConfigText;
            QTextCharFormat fm = QTextCharFormat();
            //fm.setForeground(Qt::red);

            //QDateTime local(QDateTime::currentDateTime());
            //qDebug() << local;
            //qDebug() << QString::number(local.date().year()) + QString::number(local.date().month()) + QString::number(local.date().day());
            QString calender = QString::number(year) + QString::number(month);
            fm.setBackground(Qt::white);

            QDate tmp = QDate(year, month, 1);
            for(int i=1; i<31; i++)
                ui->calendarWidget->setDateTextFormat(tmp.addDays(i), fm);
            while(!stream.atEnd())  // 파일 끝까지 읽어서
            {
                 ConfigText=stream.readLine(); // 한라인씩 읽어서 변수에 적용
                 //qDebug() << ConfigText;
                 if(QString::compare(calender, ConfigText.left(7).right(6)) == 0){
                     fm.setBackground(Qt::yellow);
                     ui->calendarWidget->setDateTextFormat(QDate::fromString(ConfigText.left(9).right(8), "yyyyMMdd"), fm);
                     //ui->response->append(ConfigText.left(9).right(8));
                 }
            }
            file.close();
    }
}

void MainWindow::Write(QString path , QString Data)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "not open file";
        return;
    } else {
            qDebug() <<QFile::exists(path);
            QTextStream out(&file);
            out << Data;
    }
}

void MainWindow::responseModified(const QString &str)
{
      // 파라미터를 사용하지 않을 때 Q_UNUSED(str);
       qDebug() << str << "파일에 접근하였습니다.";
       responseRead(str);
}

void MainWindow::histModified(const QString &str)
{
      // 파라미터를 사용하지 않을 때 Q_UNUSED(str);
       qDebug() << str << "파일에 접근하였습니다.";
       histRead(str);
}

void MainWindow::calenderModified(const QString &str)
{
      // 파라미터를 사용하지 않을 때 Q_UNUSED(str);
       qDebug() << str << "파일에 접근하였습니다.";
       QDate tmp = ui->calendarWidget->selectedDate();
       calenderRead(str, tmp.year(), tmp.month());
}

void MainWindow::on_calendarWidget_currentPageChanged(int year, int month)
{
    calenderRead(QApplication::applicationDirPath() + "/calender.txt", year, month);
}
