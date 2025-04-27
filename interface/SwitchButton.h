#pragma once

#include <QWidget>
#include <QVector>
#include <QStringList>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QMouseEvent>

class Slider;

class SwitchButton : public QWidget
{
	Q_OBJECT

public:
	explicit SwitchButton( QWidget *parent = nullptr);
	~SwitchButton();

	void SetSize(int nWidth, int nHeight);
	void SetStateTexts(QStringList stateTexts);
	void SetBackgoundColor(const QColor& color);
	void SetSlideColor(const QColor& color);
	void SetSlideTextFont(const QFont& font);
	void SetState(int state);
	QString GetState() const;

protected:
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;

private:
	bool m_bClicked;                // 是否被点击
	int m_nStateCount;              // 状态数量
	int m_currentState;             // 当前状态
	QColor m_stateColors;           // 状态颜色
	QFont m_textFont;				// 文字颜色
	QStringList m_stateTexts;       // 状态文本
	QVector<int> m_statePositions;   // 状态位置
	int m_nArcRadius;               // 圆角半径
	int m_nRectWidth;               // 矩形宽度
	const int m_nDuration = 100;    // 动画持续时间
	Slider* m_pSlider;              // 滑块

signals:
	void Clicked(int state);
};

class Slider : public QWidget
{
	Q_OBJECT

public:
	explicit Slider(QWidget* parent = nullptr);
	~Slider();
	void SetSliderColor(const QColor& color);
	void SetText(const QString &text);
	void SetTextFont(const QFont &font);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	QString m_text;                 // 滑块文本
	QColor m_sliderColor;           // 滑块颜色
	QFont m_textFont;				// 文字颜色
};

