#include "mmdlp_gui.h"

MmdlpGui::MmdlpGui(QWidget *parent, ProjectorGui *projectorGui_)
    : QWidget(parent), ui(new Ui::MmdlpGui), projectorGui(projectorGui_)

{
    ui->setupUi(this);
    initForm();

    connect(this->print_tab, &PrintTab::ProjectorShow, this->projectorGui, &ProjectorGui::showImage);

    connect(this->control_tab->controlThread, &ControlThread::proj, this->print_tab, &PrintTab::showProj);

    connect(this->print_tab->printThread, &PrintThread::transitCommand, this->control_tab->controlThread, &ControlThread::receiveCommand, Qt::QueuedConnection);
    connect(this->control_tab->controlThread, &ControlThread::commandFinished, this->print_tab->printThread, &PrintThread::sendCommand);

    connect(this->print_tab->printThread, &PrintThread::transSliceName, this->control_tab->controlThread, &ControlThread::transSliceName);

    connect(this->control_tab, &ControlTab::printTabShow, this->print_tab, &PrintTab::showProj);

    connect(this->control_tab->controlThread, &ControlThread::commandDisplay, this->print_tab, &PrintTab::commandDisplay);
    // connect(this->print_tab, &PrintTab::newProject, this->control_tab->controlThread, &ControlTab::newProject);// capture index reset

    connect(this->control_tab->controlThread->plateMotor, &InnfosMotor::timeOutError, this->print_tab, &PrintTab::timeOutError);
    connect(this->control_tab->controlThread->glassMotor, &InnfosMotor::timeOutError, this->print_tab, &PrintTab::timeOutError);
    connect(this->control_tab->controlThread->rotateMotor, &InnfosMotor::timeOutError, this->print_tab, &PrintTab::timeOutError);
    connect(qApp, &QGuiApplication::screenAdded, this, &MmdlpGui::checkScreen);
    connect(qApp, &QGuiApplication::screenRemoved, this, &MmdlpGui::checkScreen);
}

void MmdlpGui::checkScreen()
{
    const int screenCount = QGuiApplication::screens().count();
    if (screenCount < 2) {
        // QMessageBox::critical(nullptr, "错误", "检测到屏幕数量不足，应用即将退出！");
        QCoreApplication::quit(); // 退出应用程序
    }
}

MmdlpGui::~MmdlpGui()
{
    delete ui;
}

void MmdlpGui::initForm()
{
    print_tab = new PrintTab(this);
    ui->tabWidget->addTab(print_tab, QString("投影"));
    control_tab = new ControlTab(this);
    ui->tabWidget->addTab(control_tab, QString("控制"));
}