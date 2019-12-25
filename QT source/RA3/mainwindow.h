#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void responseRead(QString path);
    void histRead(QString path);
    void calenderRead(QString path, int year, int month);
    void Write(QString path , QString Data);

private slots:
    int tupdate();
    void responseModified(const QString &str);
    void histModified(const QString &str);
    void calenderModified(const QString &str);

    void on_calendarWidget_currentPageChanged(int year, int month);
};
#endif // MAINWINDOW_H
