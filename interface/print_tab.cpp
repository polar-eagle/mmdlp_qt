#include "print_tab.h"
#include <qbitmap.h>
#include <iostream>
#include <QDebug>

PrintTab::PrintTab(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::PrintTab();
	ui->setupUi(this);

	ui->pauseButton->setEnabled(false);
	ui->stopButton->setEnabled(false);

	printThread = new PrintThread(this);

	connect(printThread, &PrintThread::finished, this, &PrintTab::printFinished);
}
PrintTab::~PrintTab()
{
	delete ui;
}

void PrintTab::on_runButton_clicked()
{
	ui->codeExecutionTextEdit->clear();
	QString path = QFileDialog::getExistingDirectory(this, "Open Directory", "D://");
	QString fileName = path + "/run.gcode";
	if (!QFile::exists(fileName))
	{
		return;
	}
	ui->runButton->setEnabled(false);
	ui->pauseButton->setEnabled(true);
	printThread->isPrinting = true;
	printThread->setSliceName(path);
	printThread->start();
}

void PrintTab::on_pauseButton_clicked()
{
    printThread->stopMutex.lock();
    printThread->stopRequested = !printThread->stopRequested;
    ui->stopButton->setEnabled(printThread->stopRequested);
    if (!printThread->stopRequested)
    {
		printThread->stopMutex.unlock();
        printThread->sendCommand();
    }
	else
	{
		printThread->stopMutex.unlock();
	}
}

void PrintTab::on_stopButton_clicked()
{
	ui->stopButton->setText("取消");
	ui->pauseButton->setEnabled(false);
	ui->stopButton->setEnabled(false);
	// ui->runButton->setEnabled(true);

	printThread->mutex.lock();
	printThread->commandIndex = printThread->commandList.size();
	printThread->mutex.unlock();
	printThread->stopMutex.lock();
	printThread->stopRequested = false;
	printThread->stopMutex.unlock();
	printThread->sendCommand();
}

void PrintTab::showProj(const QString &imagePath)
{
	if (imagePath == "closeWindow")
	{
		return;
	}
	else
	{
		QPixmap pixmap(imagePath);
		if (pixmap.isNull())
		{
			QMessageBox::warning(this, "Error", "Could not load image.");
		}
		else
		{
			ui->imageLabel->setPixmap(pixmap.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
			emit ProjectorShow(imagePath);
		}
	}
}

void PrintTab::commandDisplay(const QString &command)
{
	ui->codeExecutionTextEdit->append(command + "\n");
}

void PrintTab::printFinished()
{
	ui->runButton->setEnabled(true);
	ui->pauseButton->setEnabled(false);
	ui->stopButton->setEnabled(false);
    printThread->isPrinting = false;
	QMessageBox::information(this, "Print Finished", "Print Finished");
}

void PrintTab::timeOutError()
{
	ui->stopButton->setText("停止");
	ui->pauseButton->setEnabled(false);
	ui->stopButton->setEnabled(false);
	ui->runButton->setEnabled(true);

	printThread->mutex.lock();
	printThread->commandIndex = printThread->commandList.size();
	printThread->mutex.unlock();
	QMessageBox::warning(this, "Error", "Time out error");
}