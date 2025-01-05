#include "control_tab.h"
#include <QMutex>
#include <QDebug>
#include <QBitmap>

extern std::atomic<bool> safeStop;
// extern QMutex safeStopMutex;
/**
 * @brief 格式化double类型数据为7位字符，防止数据显示跳动
 * @param value double类型数据
 * @return QString 格式化后的字符串
 */
QString formatToSevenChars(double value)
{
	QString formattedStr = QString::number(value, 'f', 3);
	if (formattedStr.indexOf('.') == 2)
	{
		formattedStr = " " + formattedStr;
	}
	return formattedStr;
}
ControlTab::ControlTab(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::ControlTab();
	ui->setupUi(this);

	addIcons();
	// 设置材料选择按钮
	materialSelectButton = new SwitchButton(this);
	materialSelectButton->SetSize(600, 60);
	materialSelectButton->SetStateTexts({"M1", "M2", "M3", "Agent"});
	materialSelectButton->SetBackgoundColor(QColor("#3A3A3A"));
	materialSelectButton->SetSlideColor(QColor("#3f59fa"));
	materialSelectButton->SetState(0);
	ui->materialSelectionRow->insertWidget(0, materialSelectButton);
	// 设置量程选择按钮
	magnitudeSelectButton = new SwitchButton(this);
	magnitudeSelectButton->SetSize(600, 60);
	magnitudeSelectButton->SetStateTexts({"0.1", "1", "10"});
	magnitudeSelectButton->SetBackgoundColor(QColor("#3A3A3A"));
	magnitudeSelectButton->SetSlideColor(QColor("#3f59fa"));
	magnitudeSelectButton->SetState(0);
	ui->magnitudeSelectionRow->insertWidget(0, magnitudeSelectButton);

	// 初始化controlThread
	controlThread = new ControlThread(this);
	// 初始化电机
	plateMotor = controlThread->motorManager->PlateMotor();
	glassMotor = controlThread->motorManager->GlassMotor();
	rotateMotor = controlThread->motorManager->RotateMotor();

	// 读取配置文件中plate电机的最大位置和glass电机的最小位置（保证电机增减时不撞机）
	ConfigManager &config = ConfigManager::instance();
	plateTargetPos = config.getGroup("motor").value("plate_max_position").toDouble();
	glassTargetPos = config.getGroup("motor").value("glass_min_position").toDouble();

	// 设置电机目标位置显示
	ui->plateMotorTargetPositionLabel->setText(formatToSevenChars(plateTargetPos));
	ui->glassMotorTargetPositionLabel->setText(formatToSevenChars(glassTargetPos));

	// 初始化电机监控线程
	glassMotorMonitorThread = new MotorMonitorThread();
	plateMotorMonitorThread = new MotorMonitorThread();
	rotateMotorMonitorThread = new MotorMonitorThread();
	// 初始化电机监控线程ID和阈值
	initMonitorThread();

	// 初始化投影仪管理器，显示当前投影仪电流
	projectorManager = ProjectorManager::getInstance();
	ui->projectorOnOffButton->setCheckable(true);
	ui->currentDisplay->setText(QString::number(projectorManager->getCurrent()));

	// 手动输入命令时，点击按钮发送命令给controlThread
	connect(ui->runCmdButton, &QPushButton::clicked, this, &ControlTab::sendCommand);
	connect(this, &ControlTab::transitCommand, controlThread, &ControlThread::receiveCommand);

	// 启动ControlThread
	controlThread->start();

	// 启动电机监控线程
	plateMotorMonitorThread->start();
	glassMotorMonitorThread->start();
	rotateMotorMonitorThread->start();

	// 按钮点击事件
	connect(ui->projectorOnOffButton, &QPushButton::clicked, this, &ControlTab::toggleProjectorOnOffButtonClicked);
	connect(ui->reduceCurrentButton, &QPushButton::clicked, this, &ControlTab::reduceCurrentButtonClicked);
	connect(ui->increaseCurrentButton, &QPushButton::clicked, this, &ControlTab::increaseCurrentButtonClicked);

	// 接收电机监控线程发送的电机位置
	connect(plateMotorMonitorThread, &MotorMonitorThread::sendPosition, this, &ControlTab::receivePosition);
	connect(glassMotorMonitorThread, &MotorMonitorThread::sendPosition, this, &ControlTab::receivePosition);
	connect(rotateMotorMonitorThread, &MotorMonitorThread::sendPosition, this, &ControlTab::receivePosition);

	// 接收电机监控线程发送的点击停止信号，将按钮状态设置为未选中
	connect(plateMotorMonitorThread, &MotorMonitorThread::stopSignal, this, &ControlTab::receiveStopSingal);
	connect(glassMotorMonitorThread, &MotorMonitorThread::stopSignal, this, &ControlTab::receiveStopSingal);
	connect(rotateMotorMonitorThread, &MotorMonitorThread::stopSignal, this, &ControlTab::receiveStopSingal);

	// 接收电机监控线程发送的电机错误码，根据错误改变按钮颜色
	connect(plateMotor, &InnfosMotor::motorErrorCode, this, &ControlTab::receiveErrorCode);
	connect(glassMotor, &InnfosMotor::motorErrorCode, this, &ControlTab::receiveErrorCode);
	connect(rotateMotor, &InnfosMotor::motorErrorCode, this, &ControlTab::receiveErrorCode);

	// 当电机接受到消息需要移动到目标位置时，将目标位置显示在界面上
	connect(plateMotor, &InnfosMotor::targetPosition, this, &ControlTab::receiveTargetPosition);
	connect(glassMotor, &InnfosMotor::targetPosition, this, &ControlTab::receiveTargetPosition);
	connect(rotateMotor, &InnfosMotor::targetPosition, this, &ControlTab::receiveTargetPosition);
}
/**
 * @brief 初始化电机监控线程，设置电机ID和阈值
 * @return void
 */
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
/**
 * @brief 将command输入框中的命令发送给ControlThread
 */
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
/**
 * @brief 点击到的平台图片，根据点击的y坐标判断点击的是哪个平台(plate/glass)，并改变按钮的图标，更新选择状态
 * @return void
 */
void ControlTab::on_platformButton_clicked()
{
	isRotatePlatformSelected = false;
	addIcon(ui->turntableButton, rotatePlatformIconPath);
	QRect rect = ui->platformButton->rect();

	int centerY = rect.top() + rect.height() / 2;
	QPoint clickPos = QCursor::pos();
	QPoint buttonPos = ui->platformButton->mapFromGlobal(clickPos);

	if (buttonPos.y() < centerY)
	{
		if (!isPlatePlatformSelected)
		{
			addIcon(ui->platformButton, platePlatformSelectedIconPath);
			isPlatePlatformSelected = true;
			isGlassPlatformSelected = false;
			if (isPlatePlatformEnable.load())
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
			isPlatePlatformSelected = false;
		}
	}
	else
	{
		if (!isGlassPlatformSelected)
		{
			addIcon(ui->platformButton, glassPlatformSelectedIconPath);
			isGlassPlatformSelected = true;
			isPlatePlatformSelected = false;
			if (isGlassPlatformEnable.load())
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
			isGlassPlatformSelected = false;
		}
	}
}
/**
 * @brief 点击到的旋转平台图片，改变按钮的图标，更新选择状态
 * @return void
 */
void ControlTab::on_turntableButton_clicked()
{
	isPlatePlatformSelected = false;
	isGlassPlatformSelected = false;
	addIcon(ui->platformButton, platformIconPath);

	if (!isRotatePlatformSelected)
	{
		addIcon(ui->turntableButton, rotatePlatformSelectedIconPath);
		isRotatePlatformSelected = true;
		if (isRotatePlatformEnable.load())
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
		addIcon(ui->turntableButton, rotatePlatformIconPath);
		isRotatePlatformSelected = false;
	}
}
/**
 * @brief 风扇开关按钮，根据按钮状态发送风扇开关命令
 * @return void
 */
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
/**
 * @brief 清洗开关按钮，根据按钮状态发送清洗开关命令
 * @return void
 */
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
/**
 * @brief 给料按钮，根据选择的材料发送给料命令
 * @return void
 */
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
/**
 * @brief 回流按钮，根据选择的材料发送回流命令
 * @return void
 */
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
/**
 * @brief 上升按钮，根据选择的平台（plate/glass）来进行上升操作
 * @return void
 */
void ControlTab::on_raiseButton_released()
{
	addIcon(ui->raiseButton, raiseIconPath);
	magnitudeSelected = magnitudeSelectButton->GetState();
	double step = magnitudeSelected.toDouble();
	if (isPlatePlatformSelected)
	{
		double targetPosition = plateTargetPos + step;
		emit transitCommand("plate " + QString::number(targetPosition));
	}
	else if (isGlassPlatformSelected)
	{
		double targetPosition = glassTargetPos + step;
		emit transitCommand("glass " + QString::number(targetPosition));
	}
}
/**
 * @brief 上升按钮按下后，改变按钮的图标
 * @return void
 */
void ControlTab::on_raiseButton_pressed()
{
	addIcon(ui->raiseButton, raisePressedIconPath);
}
/**
 * @brief 下降按钮，根据选择的平台（plate/glass）来进行下降操作
 * @return void
 */
void ControlTab::on_lowerButton_released()
{
	addIcon(ui->lowerButton, lowerIconPath);
	magnitudeSelected = magnitudeSelectButton->GetState();
	double step = magnitudeSelected.toDouble();
	if (isPlatePlatformSelected)
	{
		double targetPosition = plateTargetPos - step;
		emit transitCommand("plate " + QString::number(targetPosition));
	}
	else if (isGlassPlatformSelected)
	{
		double targetPosition = glassTargetPos - step;
		emit transitCommand("glass " + QString::number(targetPosition));
	}
}
/**
 * @brief 下降按钮按下后，改变按钮的图标
 * @return void
 */
void ControlTab::on_lowerButton_pressed()
{
	addIcon(ui->lowerButton, lowerPressedIconPath);
}
/**
 * @brief 左转按钮按下后，改变按钮的图标
 * @return void
 */
void ControlTab::on_leftTurnButton_pressed()
{
	addIcon(ui->leftTurnButton, leftTurnPressedIconPath);
}
/**
 * @brief 左转按钮松开后，改变按钮的图标，并发送左转命令
 * @return void
 */
void ControlTab::on_leftTurnButton_released()
{
	addIcon(ui->leftTurnButton, leftTurnIconPath);
	if (isRotatePlatformSelected)
	{
		double targetPosition = rotateTargetPos - 1.0;
		emit transitCommand("tank " + QString::number(targetPosition));
	}
}
/**
 * @brief 右转按钮按下后，改变按钮的图标
 * @return void
 */
void ControlTab::on_rightTurnButton_pressed()
{
	addIcon(ui->rightTurnButton, rightTurnPressedIconPath);
}
/**
 * @brief 右转按钮松开后，改变按钮的图标，并发送右转命令
 * @return void
 */
void ControlTab::on_rightTurnButton_released()
{
	addIcon(ui->rightTurnButton, rightTurnIconPath);
	if (isRotatePlatformSelected)
	{
		double targetPosition = rotateTargetPos + 1.0;
		emit transitCommand("tank " + QString::number(targetPosition));
	}
}
/**
 * @brief 电机使能按钮，根据选择的平台（plate/glass/rotate）来进行使能/失能操作
 * @return void
 */
void ControlTab::on_enableButton_clicked()
{
	qDebug() << ui->enableButton->isChecked();
	if (ui->enableButton->isChecked())
	{
		qDebug() << "enable button clicked";
		safeStop.store(false);
		if (isPlatePlatformSelected)
		{
			emit transitCommand("plateEnable");
		}
		else if (isGlassPlatformSelected)
		{
			emit transitCommand("glassEnable");
		}
		else if (isRotatePlatformSelected)
		{
			emit transitCommand("rotateEnable");
		}
	}
	else
	{
		qDebug() << "disable button clicked";
		safeStop.store(true);
		if (isPlatePlatformSelected)
		{
			emit transitCommand("plateDisable");
			isPlatePlatformEnable.store(false);
		}
		else if (isGlassPlatformSelected)
		{
			emit transitCommand("glassDisable");
			isGlassPlatformEnable.store(false);
		}
		else if (isRotatePlatformSelected)
		{
			emit transitCommand("rotateDisable");
			isRotatePlatformEnable.store(false);
		}
	}
}
/**
 * @brief 清除异常按钮，根据选择的平台（plate/glass/rotate）来进行清除异常操作
 * @return void
 */
void ControlTab::on_clearExceptionButton_clicked()
{
	if (isPlatePlatformSelected)
	{
		plateMotor->clearError();
	}
	else if (isGlassPlatformSelected)
	{
		glassMotor->clearError();
	}
	else if (isRotatePlatformSelected)
	{
		rotateMotor->clearError();
	}
}
/**
 * @brief 设置原点按钮，根据选择的平台（plate/glass/rotate）来进行设置原点操作
 * @return void
 */
void ControlTab::on_setOriginButton_clicked()
{

	if (isPlatePlatformSelected)
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
	else if (isGlassPlatformSelected)
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
	else if (isRotatePlatformSelected)
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
/**
 * @brief 接收电机监控线程发送的电机位置，更新界面上的电机位置显示
 * @return void
 */
void ControlTab::receivePosition(int motorId, double pos)
{
	// std::cout<<"motor id: " << motorId << " pos: " << pos << std::endl;
	if (motorId == controlThread->motorManager->plateMotorID)
	{
		plateMotor->setPosition(pos);
		double relPos = (pos - plateMotor->Home()) * plateMotor->Lead();
		ui->plateMotorPositionLabel->setText(formatToSevenChars(relPos));
		isPlatePlatformEnable.store(true);
	}
	else if (motorId == controlThread->motorManager->glassMotorID)
	{
		glassMotor->setPosition(pos);
		double relPos = (pos - glassMotor->Home()) * glassMotor->Lead();
		ui->glassMotorPositionLabel->setText(formatToSevenChars(relPos));
		isGlassPlatformEnable.store(true);
	}
	else if (motorId == controlThread->motorManager->rotateMotorID)
	{
		rotateMotor->setPosition(pos);
		double relPos = (pos - rotateMotor->Home()) * rotateMotor->Lead();
		ui->rotateMotorPositionLabel->setText(formatToSevenChars(relPos));
		isRotatePlatformEnable.store(true);
	}
}
/**
 * @brief 接收电机监控线程发送的电机目标位置，更新界面上的电机目标位置显示
 * @return void
 */
void ControlTab::receiveTargetPosition(int motorId, double pos)
{
	if (motorId == controlThread->motorManager->plateMotorID)
	{
		plateTargetPos = pos;
		ui->plateMotorTargetPositionLabel->setText(formatToSevenChars(plateTargetPos));
	}
	else if (motorId == controlThread->motorManager->glassMotorID)
	{
		glassTargetPos = pos;
		ui->glassMotorTargetPositionLabel->setText(formatToSevenChars(glassTargetPos));
	}
	else if (motorId == controlThread->motorManager->rotateMotorID)
	{
		rotateTargetPos = pos;
		ui->rotateMotorTargetPositionLabel->setText(formatToSevenChars(rotateTargetPos));
	}
}
/**
 * @brief 接收电机监控线程发送的电机错误码，根据错误改变按钮颜色
 * @return void
 */
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
/**
 * @brief 添加按钮图标
 * @return void
 */
void ControlTab::addIcons()
{
	addIcon(ui->turntableButton, rotatePlatformIconPath);
	addIcon(ui->leftTurnButton, leftTurnIconPath);
	addIcon(ui->rightTurnButton, rightTurnIconPath);
	addIcon(ui->platformButton, platformIconPath);
	addIcon(ui->raiseButton, raiseIconPath);
	addIcon(ui->lowerButton, lowerIconPath);
}
/**
 * @brief 添加按钮图标,根据按钮大小缩放图片
 * @param button 按钮
 * @param path 图片路径
 */
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
/**
 * @brief 减小投影仪电流按钮
 * @return void
 */
void ControlTab::reduceCurrentButtonClicked()
{
	qDebug() << "current: " << (int)projectorManager->getCurrent();
	U8 current = projectorManager->getCurrent() - 10;
	qBound<U8>(0, current, 100);
	qDebug() << "set current: " << (int)current;
	projectorManager->setCurrent(current);
	ui->currentDisplay->setText(QString::number(projectorManager->getCurrent()));
}
/**
 * @brief 增加投影仪电流按钮
 * @return void
 * @note 投影仪电流范围0-100
 */
void ControlTab::increaseCurrentButtonClicked()
{
	qDebug() << "current: " << (int)projectorManager->getCurrent();
	U8 current = projectorManager->getCurrent() + 10;
	qBound<U8>(0, current, 100);
	qDebug() << "set current: " << (int)current;
	projectorManager->setCurrent(current);
	ui->currentDisplay->setText(QString::number(projectorManager->getCurrent()));
}
/**
 * @brief 投影仪开关按钮，根据按钮状态发送投影仪开关命令
 * @param checked 按钮状态
 */
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
/**
 * @brief 加载图片按钮，打开文件选择对话框，选择图片文件
 * @return void
 */
void ControlTab::on_loadImageButton_clicked()
{
	QString imagePath = QFileDialog::getOpenFileName(this, tr("Open Image"), ".", tr("Image Files(*.png *.jpg *.bmp)"));
	if (imagePath.isEmpty())
	{
		return;
	}
	// 同步图片显示
	emit loadImageForPrint(imagePath);
}
/**
 * @brief 接收电机监控线程发送的点击停止信号，将按钮状态设置为未选中
 * @return void
 */
void ControlTab::receiveStopSingal()
{
	ui->enableButton->setChecked(false);
	isPlatePlatformEnable.store(false);
	isGlassPlatformEnable.store(false);
	isRotatePlatformEnable.store(false);
}