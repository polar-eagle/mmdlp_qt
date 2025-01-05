#include "mmdlp_gui.h"

MmdlpGui::MmdlpGui(QWidget *parent, ProjectorGui *projectorGui_)
    : QWidget(parent), ui(new Ui::MmdlpGui), projectorGui(projectorGui_)

{
    ui->setupUi(this);
    initForm();

    // ControlThread接收到proj的command信号后，print_tab显示图片并同步
    connect(this->control_tab->controlThread, &ControlThread::proj, this->print_tab, &PrintTab::showProj);
    connect(this->print_tab, &PrintTab::ProjectorShow, this->projectorGui, &ProjectorGui::showImage);
    // control_tab点击加载图片按钮后，同步两个显示界面
    connect(this->control_tab, &ControlTab::loadImageForPrint, this->print_tab, &PrintTab::showProj);

    // print_tab点击运行后，printThread顺序发送命令，命令完成后继续让printThread发送命令
    connect(this->print_tab->printThread, &PrintThread::transitCommand, this->control_tab->controlThread, &ControlThread::receiveCommand, Qt::QueuedConnection);
    connect(this->control_tab->controlThread, &ControlThread::commandFinished, this->print_tab->printThread, &PrintThread::sendCommand);

    //printThread 打开gcode文件后，发送sliceName给controlThread
    connect(this->print_tab->printThread, &PrintThread::transSliceName, this->control_tab->controlThread, &ControlThread::transSliceName);

    // controlThread执行各个地方来的命令（来自gcode，手动输入命令，加载图片，打开风扇等）在执行前将这条命令显示在print_tab的命令显示框中
    connect(this->control_tab->controlThread, &ControlThread::commandDisplay, this->print_tab, &PrintTab::commandDisplay);
    // connect(this->print_tab, &PrintTab::newProject, this->control_tab->controlThread, &ControlTab::newProject);// capture index reset

    // 当各个点击出现超时时，显示超时错误
    connect(this->control_tab->controlThread->plateMotor, &InnfosMotor::timeOutError, this->print_tab, &PrintTab::timeOutError);
    connect(this->control_tab->controlThread->glassMotor, &InnfosMotor::timeOutError, this->print_tab, &PrintTab::timeOutError);
    connect(this->control_tab->controlThread->rotateMotor, &InnfosMotor::timeOutError, this->print_tab, &PrintTab::timeOutError);
}

MmdlpGui::~MmdlpGui()
{
    delete ui;
}
/**
 * @brief 初始化界面，实例化打印和控制tab
 * @return void
 */
void MmdlpGui::initForm()
{
    print_tab = new PrintTab(this);
    ui->tabWidget->addTab(print_tab, QString("投影"));
    control_tab = new ControlTab(this);
    ui->tabWidget->addTab(control_tab, QString("控制"));
}