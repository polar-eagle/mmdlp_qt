#pragma once

#include "ui_control_tab.h"
#include <QButtonGroup>
#include <qmessagebox.h>
#include "ControlThread.h"
#include "MotorMonitorThread.h"
#include "PrintThread.h"
#include "ConfigManager.h"
#include <QLineEdit>
#include <QPushButton>
#include "print_tab.h"
#include "SwitchButton.h"

class ControlTab : public QWidget
{
	Q_OBJECT

public:
	ControlTab(QWidget *parent = Q_NULLPTR);
	~ControlTab();

	void addIcons();
	void addIcon(QPushButton *button, QString path);

	void initMonitorThread();
	ControlThread *controlThread;

	InnfosMotor *plateMotor;
	InnfosMotor *glassMotor;
	InnfosMotor *rotateMotor;

	double plateTargetPos = 0.0;
	double glassTargetPos = 0.0;
	double rotateTargetPos = 0.0;
public slots:
	void sendCommand();

signals:
	void transitCommand(const QString &command);
	void printTabShow(const QString &imagePath);

private:
	Ui::ControlTab *ui;

	MotorMonitorThread *glassMotorMonitorThread;
	MotorMonitorThread *plateMotorMonitorThread;
	MotorMonitorThread *rotateMotorMonitorThread;

	SwitchButton *materialSelectButton;
	SwitchButton *magnitudeSelectButton;

	ProjectorManager *projectorManager;

	QString magnitudeSelected;
	QString matrialSelected;

	bool isTurnableSelected = false;
	bool isGlassPlatSelected = false;
	bool isGrowthPlatSelected = false;

	std::atomic<bool> isPlateMotorEnable = false;
	std::atomic<bool> isGlassMotorEnable = false;
	std::atomic<bool> isRotateMotorEnable = false;
	bool plateEn = false;
	bool glassEn = false;
	bool rotateEn = false;

	const QString turnableIconPath = "../../../interface/icon/turnable.png";
	const QString turnableSelectedIconPath = "../../../interface/icon/turnable_selected.png";
	const QString leftTurnIconPath = "../../../interface/icon/leftTurn.png";
	const QString leftTurnPressedIconPath = "../../../interface/icon/leftTurn_pressed.png";
	const QString rightTurnIconPath = "../../../interface/icon/rightTurn.png";
	const QString rightTurnPressedIconPath = "../../../interface/icon/rightTurn_pressed.png";
	const QString platformIconPath = "../../../interface/icon/platform.png";
	const QString glassPlatSelectedIconPath = "../../../interface/icon/glassPlat_selected.png";
	const QString growthPlatSelectedIconPath = "../../../interface/icon/growthPlat_selected.png";
	const QString raiseIconPath = "../../../interface/icon/raise.png";
	const QString raisePressedIconPath = "../../../interface/icon/raise_pressed.png";
	const QString lowerIconPath = "../../../interface/icon/lower.png";
	const QString lowerPressedIconPath = "../../../interface/icon/lower_pressed.png";

private slots:
	void on_platformButton_clicked();
	void on_turntableButton_clicked();
	void on_raiseButton_pressed();
	void on_raiseButton_released();
	void on_lowerButton_pressed();
	void on_lowerButton_released();
	void on_leftTurnButton_pressed();
	void on_leftTurnButton_released();
	void on_rightTurnButton_pressed();
	void on_rightTurnButton_released();
	void on_fanOnButton_clicked();
	void on_cleaningOnButton_clicked();
	void on_refluxButton_clicked();
	void on_feedButton_clicked();
	void on_enableButton_clicked();
	void on_clearExceptionButton_clicked();
	void on_setOriginButton_clicked();

	void on_loadImageButton_clicked();
	void reduceCurrentButtonClicked();
	void increaseCurrentButtonClicked();
	void toggleProjectorOnOffButtonClicked(bool checked);

	void receivePosition(int motorId, double pos);
	void receiveErrorCode(int id, uint32_t errorCode);
	void receiveTargetPosition(int motorId, double pos);

	void receiveStopSingal();
};
