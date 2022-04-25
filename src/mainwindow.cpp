#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QVBoxLayout>

#include "Widget1.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	auto layout = new QVBoxLayout;
	layout->addWidget(new Widget1);
	
	QWidget *w = new QWidget(this);
	w->setLayout(layout);

	setCentralWidget(w);
}

MainWindow::~MainWindow()
{
	delete ui;
}

