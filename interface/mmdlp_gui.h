#ifndef MMDLP_GUI_H
#define MMDLP_GUI_H

#include <QWidget>
#include "print_tab.h"
#include "control_tab.h"
#include "ui_mmdlp_gui.h"
#include "projector_gui.h"
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QMessageBox>

namespace Ui
{
	class MmdlpGui;
}

class MmdlpGui : public QWidget
{
	Q_OBJECT

public:
	explicit MmdlpGui(QWidget *parent = nullptr,ProjectorGui *projectorGui_ = nullptr);
	~MmdlpGui();
	void initForm();
	PrintTab *print_tab = nullptr;
	ProjectorGui *projectorGui = nullptr;

private:
	Ui::MmdlpGui *ui;
	ControlTab *control_tab = nullptr;

private slots:
	void checkScreen();
};

#endif // MMDLP_GUI_H
