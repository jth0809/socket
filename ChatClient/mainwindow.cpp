#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHostAddress>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    socket(new QTcpSocket(this)) {
    ui->setupUi(this);
    socket->connectToHost(QHostAddress::LocalHost, 12345);

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onMessageReceived);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_sendButton_clicked() {
    QString message = ui->messageInput->text();
    if (message.isEmpty()) return;
    socket->write(message.toUtf8());
    ui->messageInput->clear();
}

void MainWindow::onMessageReceived() {
    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();
        ui->messageList->addItem(line);
    }
}
