#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->btnClearText->setIcon(QIcon(":/image/peiqi.png"));
    ui->btnClearText->setIconSize(QSize(30, 30));
    connect(ui->btnClearText, &QPushButton::clicked, this, &MainWindow::onClearText);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onClearText()
{
    ui->textEdit->clear();
}