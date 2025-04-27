#include "control_tab.h"
#include <QMutex>
#include <QDebug>
#include <QBitmap>

extern std::atomic<bool> safeStop;
// extern QMutex safeStopMutex;

ControlTab::ControlTab(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::ControlTab();
	ui->setupUi(this);

	addIcons();
	materialSelectButton = new SwitchButton(this);
	materialSelectButton->SetSize(600, 60);
	materialSelectButton->SetStateTexts({"M1", "M2", "M3", "Agent"});
	materialSelectButton->SetBackgoundColor(QColor("#3A3A3A"));
	materialSelectButton->SetSlideColor(QColor("#3f59fa"));
	materialSelectButton->SetState(0);
	ui->materialSelectionRow->insertWidget(0, materialSelectButton);

	magnitudeSelectButton = new SwitchButton(this);
	magnitudeSelectButton->SetSize(600, 60);
	magnitudeSelectButton->SetStateTexts({"0.1", "1", "10"});
	magnitudeSelectButton->SetBackgoundColor(QColor("#3A3A3A"));
	magnitudeSelectButton->SetSlideColor(QColor("#3f59fa"));
	magnitudeSelectButton->SetState(0);
	ui->magnitudeSelectionRow->insertWidget(0, magnitudeSelectButton);

	controlThread = new ControlThread(this);
	plateMotor = controlThread->motorManager->PlateMotor();
	glassMotor = controlThread->motorManager->GlassMotor();
	rotateMotor = controlThread->motorManager->RotateMotor();

	ConfigManager &config = ConfigManager::instance();
	plateTargetPos = config.getGroup("motor").value("plate_max_position").toDouble();
	glassTargetPos = config.getGroup("motor").value("glass_min_position").toDouble();

	ui->plateMotorTargetPositionLabel->setText(QString::number(plateTargetPos));
	ui->glassMotorTargetPositionLabel->setText(QString::number(glassTargetPos));

	glassMotorMonitorThread = new MotorMonitorThread();
	plateMotorMonitorThread = new MotorMonitorThread();
	rotateMotorMonitorThread = new MotorMonitorThread();
	initMonitorThread();

	projectorManager = ProjectorManager::getInstance();
	ui->projectorOnOffButton->setCheckable(true);
	ui->currentDisplay->setText(QString::number(projectorManager->getCurrent()));

	connect(ui->runCmdButton, &QPushButton::clicked, this, &ControlTab::sendCommand);
	connect(this, &ControlTab::transitCommand, controlThread, &ControlThread::receiveCommand);

	controlThread->start();

	plateMotorMonitorThread->start();
	glassMotorMonitorThread->start();
	rotateMotorMonitorThread->start();

	connect(ui->projectorOnOffButton, &QPushButton::clicked, this, &ControlTab::toggleProjectorOnOffButtonClicked);
	connect(ui->reduceCurrentButton, &QPushButton::clicked, this, &ControlTab::reduceCurrentButtonClicked);
	connect(ui->increaseCurrentButton, &QPushButton::clicked, this, &ControlTab::increaseCurrentButtonClicked);

	connect(plateMotorMonitorThread, &MotorMonitorThread::sendPosition, this, &ControlTab::receivePosition);
	connect(glassMotorMonitorThread, &MotorMonitorThread::sendPosition, this, &ControlTab::receivePosition);
	connect(rotateMotorMonitorThread, &MotorMonitorThread::sendPosition, this, &ControlTab::receivePosition);

	connect(plateMotorMonitorThread, &MotorMonitorThread::stopSignal, this, &ControlTab::receiveStopSingal);
	connect(glassMotorMonitorThread, &MotorMonitorThread::stopSignal, this, &ControlTab::receiveStopSingal);
	connect(rotateMotorMonitorThread, &MotorMonitorThread::stopSignal, this, &ControlTab::receiveStopSingal);

	connect(plateMotor, &InnfosMotor::motorErrorCode, this, &ControlTab::receiveErrorCode);
	connect(glassMotor, &InnfosMotor::motorErrorCode, this, &ControlTab::receiveErrorCode);
	connect(rotateMotor, &InnfosMotor::motorErrorCode, this, &ControlTab::receiveErrorCode);

	connect(plateMotor, &InnfosMotor::targetPosition, this, &ControlTab::receiveTargetPosition);
	connect(glassMotor, &InnfosMotor::targetPosition, this, &ControlTab::receiveTargetPosition);
	connect(rotateMotor, &InnfosMotor::targetPosition, this, &ControlTab::receiveTargetPosition);
}
void ControlTab::initMonitorThread()
{
	ConfigManager &config = ConfigManager::instance();

	int glassMotorID = config.getGroup("motor").value("glass_motor_id").toInt();
	int plateMotorID = config.getGroup("motor").value("plate_motor_id").toInt();
	int rotateMotorID = config.getGroup("motor").value("rotate_motor_id").toInt();

	glassMotorMonitorThread->setMotorID(glassMotorID);
	plateMotorMonitorThread->setMotorID(plateMotorID);
	rotateMotorMonitorThread->setMotorID(rotateMotorID);

	double glassSafeCurrent = config.getGroup("motor").value("glass_safe_current_limit").toDouble();
	double glassSafeVelocity = config.getGroup("motor").value("glass_safe_velocity_limit").toDouble();
	double plateSafeCurrent = config.getGroup("motor").value("plate_safe_current_limit").toDouble();
	double plateSafeVelocity = config.getGroup("motor").value("plate_safe_velocity_limit").toDouble();
	double rotateSafeCurrent = config.getGroup("motor").value("rotate_safe_current_limit").toDouble();
	double rotateSafeVelocity = config.getGroup("motor").value("rotate_safe_velocity_limit").toDouble();

	glassMotorMonitorThread->setThresholds(glassSafeCurrent, glassSafeVelocity);
	plateMotorMonitorThread->setThresholds(plateSafeCurrent, plateSafeVelocity);
	rotateMotorMonitorThread->setThresholds(rotateSafeCurrent, rotateSafeVelocity);
}
void ControlTab::sendCommand()
{
	QString command = ui->cmdLineEdit->text();
	qDebug() << "send command: " + command;
	emit transitCommand(command);
}
ControlTab::~ControlTab()
{
	delete ui;
}
void ControlTab::on_platformButton_clicked()
{
	isTurnableSelected = false;
	addIcon(ui->turntableButton, turnableIconPath);
	QRect rect = ui->platformButton->rect();

	int centerY = rect.top() + rect.height() / 2;
	QPoint clickPos = QCursor::pos();
	QPoint buttonPos = ui->platformButton->mapFromGlobal(clickPos);

	if (buttonPos.y() < centerY)
	{
		if (!isGrowthPlatSelected)
		{
			addIcon(ui->platformButton, growthPlatSelectedIconPath);
			isGrowthPlatSelected = true;
			isGlassPlatSelected = false;
			if (isPlateMotorEnable.load())
			{
				ui->enableButton->setChecked(true);
			}
			else
			{
				ui->enableButton->setChecked(false);
			}
		}
		else
		{
			addIcon(ui->platformButton, platformIconPath);
			isGrowthPlatSelected = false;
		}
	}
	else
	{
		if (!isGlassPlatSelected)
		{
			addIcon(ui->platformButton, glassPlatSelectedIconPath);
			isGlassPlatSelected = true;
			isGrowthPlatSelected = false;
			if (isGlassMotorEnable.load())
			{
				ui->enableButton->setChecked(true);
			}
			else
			{
				ui->enableButton->setChecked(false);
			}
		}
		else
		{
			addIcon(ui->platformButton, platformIconPath);
			isGlassPlatSelected = false;
		}
	}
}

void ControlTab::on_turntableButton_clicked()
{
	isGrowthPlatSelected = false;
	isGlassPlatSelected = false;
	addIcon(ui->platformButton, platformIconPath);

	if (!isTurnableSelected)
	{
		addIcon(ui->turntableButton, turnableSelectedIconPath);
		isTurnableSelected = true;
		if (isRotateMotorEnable.load())
		{
			ui->enableButton->setChecked(true);
		}
		else
		{
			ui->enableButton->setChecked(false);
		}
	}
	else
	{
		addIcon(ui->turntableButton, turnableIconPath);
		isTurnableSelected = false;
	}
}

void ControlTab::on_fanOnButton_clicked()
{
	if (ui->fanOnButton->isChecked())
	{
		emit transitCommand("fan open");
	}
	else
	{
		emit transitCommand("fan close");
	}
}

void ControlTab::on_cleaningOnButton_clicked()
{
	if (ui->cleaningOnButton->isChecked())
	{
		emit transitCommand("clean open");
	}
	else
	{
		emit transitCommand("clean close");
	}
}

void ControlTab::on_feedButton_clicked()
{
	matrialSelected = materialSelectButton->GetState();
	if (matrialSelected == "M1")
	{
		emit transitCommand("AMS 0 feed 30");
	}
	else if (matrialSelected == "M2")
	{
		emit transitCommand("AMS 1 feed 30");
	}
	else if (matrialSelected == "M3")
	{
		emit transitCommand("AMS 2 feed 30");
	}
	else if (matrialSelected == "Agent")
	{
		emit transitCommand("ASS input 300");
	}
}

void ControlTab::on_refluxButton_clicked()
{
	matrialSelected = materialSelectButton->GetState();
	if (matrialSelected == "M1")
	{
		emit transitCommand("AMS 0 backflow 30");
	}
	else if (matrialSelected == "M2")
	{
		emit transitCommand("AMS 1 backflow 30");
	}
	else if (matrialSelected == "M3")
	{
		emit transitCommand("AMS 2 backflow 30");
	}
	else if (matrialSelected == "Agent")
	{
		emit transitCommand("ASS output 300");
	}
}

void ControlTab::on_raiseButton_released()
{
	addIcon(ui->raiseButton, raiseIconPath);
	magnitudeSelected = magnitudeSelectButton->GetState();
	double step = magnitudeSelected.toDouble();
	if (isGrowthPlatSelected)
	{
		double targetPosition = plateTargetPos + step;
		emit transitCommand("plate " + QString::number(targetPosition));
	}
	else if (isGlassPlatSelected)
	{
		double targetPosition = glassTargetPos + step;
		emit transitCommand("glass " + QString::number(targetPosition));
	}
}
void ControlTab::on_raiseButton_pressed()
{
	addIcon(ui->raiseButton, raisePressedIconPath);
}
void ControlTab::on_lowerButton_released()
{
	addIcon(ui->lowerButton, lowerIconPath);
	magnitudeSelected = magnitudeSelectButton->GetState();
	double step = magnitudeSelected.toDouble();
	if (isGrowthPlatSelected)
	{
		double targetPosition = plateTargetPos - step;
		emit transitCommand("plate " + QString::number(targetPosition));
	}
	else if (isGlassPlatSelected)
	{
		double targetPosition = glassTargetPos - step;
		emit transitCommand("glass " + QString::number(targetPosition));
	}
}
void ControlTab::on_lowerButton_pressed()
{
	addIcon(ui->lowerButton, lowerPressedIconPath);
}

void ControlTab::on_leftTurnButton_pressed()
{
	addIcon(ui->leftTurnButton, leftTurnPressedIconPath);
}
void ControlTab::on_leftTurnButton_released()
{
	addIcon(ui->leftTurnButton, leftTurnIconPath);
	if (isTurnableSelected)
	{
		double targetPosition = rotateTargetPos - 1.0;
		emit transitCommand("tank " + QString::number(targetPosition));
	}
}
void ControlTab::on_rightTurnButton_pressed()
{
	addIcon(ui->rightTurnButton, rightTurnPressedIconPath);
}
void ControlTab::on_rightTurnButton_released()
{
	addIcon(ui->rightTurnButton, rightTurnIconPath);
	if (isTurnableSelected)
	{
		double targetPosition = rotateTargetPos + 1.0;
		emit transitCommand("tank " + QString::number(targetPosition));
	}
}

void ControlTab::on_enableButton_clicked()
{
	qDebug()<<ui->enableButton->isChecked();
	if (ui->enableButton->isChecked())
	{
		qDebug() << "enable button clicked";
		safeStop.store(false);
		if (isGrowthPlatSelected)
		{
			emit transitCommand("plateEnable");
		}
		else if (isGlassPlatSelected)
		{
			emit transitCommand("glassEnable");
		}
		else if (isTurnableSelected)
		{
			emit transitCommand("rotateEnable");
		}
	}
	else
	{
		qDebug() << "disable button clicked";
		safeStop.store(true);
		if (isGrowthPlatSelected)
		{
			emit transitCommand("plateDisable");
			isPlateMotorEnable.store(false);
		}
		else if (isGlassPlatSelected)
		{
			emit transitCommand("glassDisable");
			isGlassMotorEnable.store(false);
		}
		else if (isTurnableSelected)
		{
			emit transitCommand("rotateDisable");
			isRotateMotorEnable.store(false);
		}
	}
}

// void ControlTab::on_enableButton_clicked()
// {
// 	if (isGrowthPlatSelected){
// 		if (plateEn){
// 			emit transitCommand("plateDisable");
// 			plateEn = false;
// 		}
// 		else{
// 			emit transitCommand("plateEnable");
// 			plateEn = true;
// 		}
// 	}else if (isGlassPlatSelected){
// 		if (glassEn){
// 			emit transitCommand("glassDisable");
// 			glassEn = false;
// 		}
// 		else{
// 			emit transitCommand("glassEnable");
// 			glassEn = true;
// 		}
// 	}else if (isTurnableSelected){
// 		if (rotateEn){
// 			emit transitCommand("rotateDisable");
// 			rotateEn = false;
// 		}
// 		else{
// 			emit transitCommand("rotateEnable");
// 			rotateEn = true;
// 		}
// 	}
// }

void ControlTab::on_clearExceptionButton_clicked()
{
	if (isGrowthPlatSelected)
	{
		plateMotor->clearError();
	}
	else if (isGlassPlatSelected)
	{
		glassMotor->clearError();
	}
	else if (isTurnableSelected)
	{
		rotateMotor->clearError();
	}
}

void ControlTab::on_setOriginButton_clicked()
{

	if (isGrowthPlatSelected)
	{
		// plateMotor->setOrinPosition(plateMotor->Position());
		ConfigManager &config = ConfigManager::instance();
		double currentPosition = plateMotor->Position();
		if (config.updateValue("motor/plate_home", currentPosition))
		{

			plateMotor->setMaxPosition(plateMotor->MaxPosition() + (currentPosition - plateMotor->Home()));
			plateMotor->setMinPosition(plateMotor->MinPosition() + (currentPosition - plateMotor->Home()));
			plateMotor->setHome(currentPosition);
			qDebug() << "set origin: " << currentPosition;
		}
		else
		{
			qDebug() << "set origin fail";
		}
	}
	else if (isGlassPlatSelected)
	{
		// glassMotor->setOrinPosition(glassMotor->Position());
		ConfigManager &config = ConfigManager::instance();
		double currentPosition = glassMotor->Position();
		if (config.updateValue("motor/glass_home", currentPosition))
		{

			glassMotor->setMaxPosition(glassMotor->MaxPosition() + (currentPosition - glassMotor->Home()));
			glassMotor->setMinPosition(glassMotor->MinPosition() + (currentPosition - glassMotor->Home()));
			glassMotor->setHome(currentPosition);
			qDebug() << "set origin: " << currentPosition;
		}
		else
		{
			qDebug() << "set origin fail";
		}
	}
	else if (isTurnableSelected)
	{
		// rotateMotor->setOrinPosition(rotateMotor->Position());
		ConfigManager &config = ConfigManager::instance();
		double currentPosition = rotateMotor->Position();
		if (config.updateValue("motor/rotate_home", currentPosition))
		{

			rotateMotor->setMaxPosition(rotateMotor->MaxPosition() + (currentPosition - rotateMotor->Home()));
			rotateMotor->setMinPosition(rotateMotor->MinPosition() + (currentPosition - rotateMotor->Home()));
			rotateMotor->setHome(currentPosition);
			qDebug() << "set origin: " << currentPosition;
		}
		else
		{
			qDebug() << "set origin fail";
		}
	}
}

void ControlTab::receivePosition(int motorId, double pos)
{
	// std::cout<<"motor id: " << motorId << " pos: " << pos << std::endl;
	if (motorId == controlThread->motorManager->plateMotorID)
	{
		plateMotor->setPosition(pos);
		double relPos = (pos - plateMotor->Home()) * plateMotor->Lead();
		ui->plateMotorPositionLabel->setText(QString::number(relPos, 'f', 4));
		isPlateMotorEnable.store(true);
	}
	else if (motorId == controlThread->motorManager->glassMotorID)
	{
		glassMotor->setPosition(pos);
		double relPos = (pos - glassMotor->Home()) * glassMotor->Lead();
		ui->glassMotorPositionLabel->setText(QString::number(relPos, 'f', 4));
		isGlassMotorEnable.store(true);
	}
	else if (motorId == controlThread->motorManager->rotateMotorID)
	{
		rotateMotor->setPosition(pos);
		double relPos = (pos - rotateMotor->Home()) * rotateMotor->Lead();
		ui->rotateMotorPositionLabel->setText(QString::number(relPos, 'f', 4));
		isRotateMotorEnable.store(true);
	}
}

void ControlTab::receiveTargetPosition(int motorId, double pos)
{
	if (motorId == controlThread->motorManager->plateMotorID)
	{
		plateTargetPos = pos;
		ui->plateMotorTargetPositionLabel->setText(QString::number(plateTargetPos, 'f', 4));
	}
	else if (motorId == controlThread->motorManager->glassMotorID)
	{
		glassTargetPos = pos;
		ui->glassMotorTargetPositionLabel->setText(QString::number(glassTargetPos, 'f', 4));
	}
	else if (motorId == controlThread->motorManager->rotateMotorID)
	{
		rotateTargetPos = pos;
		ui->rotateMotorTargetPositionLabel->setText(QString::number(rotateTargetPos, 'f', 4));
	}
}
void ControlTab::receiveErrorCode(int motorId, uint32_t errorCode)
{
	if (motorId == controlThread->motorManager->plateMotorID)
	{
		if (errorCode != 0)
		{
			ui->growthPlatformLabel->setStyleSheet("background-color: red;");
		}
		else
		{
			ui->growthPlatformLabel->setStyleSheet("background-color: green;");
		}
	}
	else if (motorId == controlThread->motorManager->glassMotorID)
	{
		if (errorCode != 0)
		{
			ui->glassPlatformLabel->setStyleSheet("background-color: red;");
		}
		else
		{
			ui->glassPlatformLabel->setStyleSheet("background-color: green;");
		}
	}
	else if (motorId == controlThread->motorManager->rotateMotorID)
	{
		if (errorCode != 0)
		{
			ui->turntableLabel->setStyleSheet("background-color: red;");
		}
		else
		{
			ui->turntableLabel->setStyleSheet("background-color: green;");
		}
	}
}

void ControlTab::addIcons()
{
	addIcon(ui->turntableButton, turnableIconPath);
	addIcon(ui->leftTurnButton, leftTurnIconPath);
	addIcon(ui->rightTurnButton, rightTurnIconPath);
	addIcon(ui->platformButton, platformIconPath);
	addIcon(ui->raiseButton, raiseIconPath);
	addIcon(ui->lowerButton, lowerIconPath);
}

void ControlTab::addIcon(QPushButton *button, QString path)
{
	QIcon icon;
	int buttonWidth = button->size().width();
	int buttonHeight = button->size().height();
	QPixmap pixmap(path);
	pixmap = pixmap.scaled(buttonWidth, buttonHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	icon.addPixmap(pixmap);
	button->setIcon(icon);
	button->setIconSize(QSize(buttonWidth, buttonHeight));
	button->setFixedSize(pixmap.size());
	button->setMask(pixmap.mask());
}

void ControlTab::reduceCurrentButtonClicked()
{
	qDebug() << "current: " << (int)projectorManager->getCurrent();
	U8 current = projectorManager->getCurrent() - 10;
	qBound<U8>(0, current, 100);
	qDebug() << "set current: " << (int)current;
	projectorManager->setCurrent(current);
	ui->currentDisplay->setText(QString::number(projectorManager->getCurrent()));
}

void ControlTab::increaseCurrentButtonClicked()
{
	qDebug() << "current: " << (int)projectorManager->getCurrent();
	U8 current = projectorManager->getCurrent() + 10;
	qBound<U8>(0, current, 100);
	qDebug() << "set current: " << (int)current;
	projectorManager->setCurrent(current);
	ui->currentDisplay->setText(QString::number(projectorManager->getCurrent()));
}

void ControlTab::toggleProjectorOnOffButtonClicked(bool checked)
{

	if (checked)
	{
		projectorManager->ledOn();
	}
	else
	{
		projectorManager->ledOff();
	}
}
void ControlTab::on_loadImageButton_clicked()
{
	QString imagePath = QFileDialog::getOpenFileName(this, tr("Open Image"), ".", tr("Image Files(*.png *.jpg *.bmp)"));
	if (imagePath.isEmpty())
	{
		return;
	}
	emit printTabShow(imagePath);
}

void ControlTab::receiveStopSingal()
{
	ui->enableButton->setChecked(false);
	isPlateMotorEnable.store(false);
	isGlassMotorEnable.store(false);
	isRotateMotorEnable.store(false);
}