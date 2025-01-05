#ifndef RPOJECTOR_GUI_H
#define RPOJECTOR_GUI_H

#include <QWidget>
#include "ui_projector_gui.h"
#include <QLabel>
#include <QImage>

namespace Ui
{
	class ProjectorGui;
}

class ProjectorGui : public QWidget
{
	Q_OBJECT

public:
	explicit ProjectorGui(QWidget *parent = nullptr);
	~ProjectorGui();
	QLabel *imageLabel = nullptr;

public slots:
	void showImage(const QString &imagePath);

private:
	Ui::ProjectorGui *ui;
};

#endif // RPOJECTOR_GUI_H
