#pragma once

#include "ui_print_tab.h"
#include <QFileDialog>
#include <QMessageBox>
#include "PrintThread.h"
#include "ProjectorManager.h"

class PrintTab : public QWidget
{
	Q_OBJECT

public:
	PrintTab(QWidget *parent = Q_NULLPTR);
	~PrintTab();
	PrintThread *printThread;

public slots:
	void showProj(const QString &imagePath);
	void commandDisplay(const QString &command);
	void timeOutError();
signals:
	void ProjectorShow(const QString &imagePath);
	void transmitSliceName(const QString sliceName);

private:
	Ui::PrintTab *ui;

private slots:
	void on_runButton_clicked();
	void on_pauseButton_clicked();
	void on_stopButton_clicked();
	void printFinished();
};
