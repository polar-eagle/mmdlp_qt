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
	bool m_bClicked;                // �Ƿ񱻵��
	int m_nStateCount;              // ״̬����
	int m_currentState;             // ��ǰ״̬
	QColor m_stateColors;           // ״̬��ɫ
	QFont m_textFont;				// ������ɫ
	QStringList m_stateTexts;       // ״̬�ı�
	QVector<int> m_statePositions;   // ״̬λ��
	int m_nArcRadius;               // Բ�ǰ뾶
	int m_nRectWidth;               // ���ο��
	const int m_nDuration = 100;    // ��������ʱ��
	Slider* m_pSlider;              // ����

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
	QString m_text;                 // �����ı�
	QColor m_sliderColor;           // ������ɫ
	QFont m_textFont;				// ������ɫ
};

