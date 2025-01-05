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
/**
 * @brief 运行按钮回调，选择文件夹并开始打印
 * @return void
 */
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
/** 
 * @brief 暂停按钮回调，暂停打印
 * @return void
*/
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
/** 
 * @brief 停止按钮回调，停止打印
 * @return void
*/
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
/** 
 * @brief 同步投影仪图像
 * @param imagePath 图片路径
 */
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
/** 
 * @brief 显示命令，所有的命令都来自于ControlThread的work函数received到的命令
 * @param command 命令
 * @return void
 */
void PrintTab::commandDisplay(const QString &command)
{
	ui->codeExecutionTextEdit->append(command + "\n");
}
/**
 * @brief 打印结束后接收到信号更新按钮状态，弹出提示框
 * @return void
 */
void PrintTab::printFinished()
{
	ui->runButton->setEnabled(true);
	ui->pauseButton->setEnabled(false);
	ui->stopButton->setEnabled(false);
    printThread->isPrinting = false;
	QMessageBox::information(this, "Print Finished", "Print Finished");
}
/**
 * @brief 超时出错后接收到消息，更新按钮状态，弹出提示框
 * @return void
 */
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