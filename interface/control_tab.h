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

	InnfosMotor *plateMotor;  // plate电机
	InnfosMotor *glassMotor;  // glass电机
	InnfosMotor *rotateMotor; // rotate电机

	double plateTargetPos = 0.0;  // plate目标位置
	double glassTargetPos = 0.0;  // glass目标位置
	double rotateTargetPos = 0.0; // rotate目标位置
public slots:
	void sendCommand();

signals:
	void transitCommand(const QString &command);
	void loadImageForPrint(const QString &imagePath);

private:
	Ui::ControlTab *ui;

	MotorMonitorThread *glassMotorMonitorThread;  // glass电机监控线程
	MotorMonitorThread *plateMotorMonitorThread;  // plate电机监控线程
	MotorMonitorThread *rotateMotorMonitorThread; // rotate电机监控线程

	SwitchButton *materialSelectButton;	 // 材料选择按钮
	SwitchButton *magnitudeSelectButton; // 量程选择按钮

	ProjectorManager *projectorManager; // 投影仪管理器

	QString matrialSelected;   // 选择的材料
	QString magnitudeSelected; // 选择的量程

	bool isPlatePlatformSelected = false;  // 是否选择了plate平台
	bool isGlassPlatformSelected = false;  // 是否选择了glass平台
	bool isRotatePlatformSelected = false; // 是否选择了rotate平台

	std::atomic<bool> isPlatePlatformEnable = false;  // plate电机是否使能
	std::atomic<bool> isGlassPlatformEnable = false;  // glass电机是否使能
	std::atomic<bool> isRotatePlatformEnable = false; // rotate电机是否使能

	const QString rotatePlatformIconPath = "../../../interface/icon/rotatePlatform.png";				  // 旋转平台按钮图片
	const QString rotatePlatformSelectedIconPath = "../../../interface/icon/rotatePlatform_selected.png"; // 选择旋转平台按钮图片
	const QString platformIconPath = "../../../interface/icon/platform.png";							  // 平台按钮图片
	const QString glassPlatformSelectedIconPath = "../../../interface/icon/glassPlatform_selected.png";	  // 选择玻璃平台按钮图片
	const QString platePlatformSelectedIconPath = "../../../interface/icon/platePlatform_selected.png";	  // 选择平板平台按钮图片
	const QString raiseIconPath = "../../../interface/icon/raise.png";									  // 上升按钮图片
	const QString raisePressedIconPath = "../../../interface/icon/raise_pressed.png";					  // 上升按钮按下图片
	const QString lowerIconPath = "../../../interface/icon/lower.png";									  // 下降按钮图片
	const QString lowerPressedIconPath = "../../../interface/icon/lower_pressed.png";					  // 下降按钮按下图片
	const QString leftTurnIconPath = "../../../interface/icon/leftTurn.png";							  // 左转按钮图片
	const QString leftTurnPressedIconPath = "../../../interface/icon/leftTurn_pressed.png";				  // 左转按钮按下图片
	const QString rightTurnIconPath = "../../../interface/icon/rightTurn.png";							  // 右转按钮图片
	const QString rightTurnPressedIconPath = "../../../interface/icon/rightTurn_pressed.png";			  // 右转按钮按下图片

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
