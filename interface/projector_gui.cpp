#include "projector_gui.h"
#include "QDebug"

ProjectorGui::ProjectorGui(QWidget *parent)
	: QWidget(parent), ui(new Ui::ProjectorGui)

{
	ui->setupUi(this);
	setFixedSize(3840, 2160);
	setStyleSheet("background-color: black;");

	imageLabel = new QLabel(this);
	imageLabel->setAlignment(Qt::AlignCenter);

	imageLabel->setFixedSize(3840, 2160);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(imageLabel);
	layout->setAlignment(Qt::AlignCenter);
	setLayout(layout);
}

ProjectorGui::~ProjectorGui()
{
	delete ui;
}

void ProjectorGui::showImage(const QString &imagePath)
{
	qDebug() << "Loading image from path:" << imagePath;
	QPixmap pixmap(imagePath);
	if (!pixmap.isNull())
	{
		imageLabel->setPixmap(pixmap);
		imageLabel->show();
	}
	else
	{
		qWarning() << "Failed to load image from path:" << imagePath;
	}
}
