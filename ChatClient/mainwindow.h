#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_sendButton_clicked();
    void onMessageReceived();

private:
    Ui::MainWindow* ui;
    QTcpSocket* socket;
};

#endif // MAINWINDOW_H
