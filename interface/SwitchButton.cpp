#include "SwitchButton.h"

SwitchButton::SwitchButton(QWidget *parent):
	QWidget(parent)
	
{
	m_bClicked = true;
	m_currentState = 0;
	m_stateTexts = QStringList({ "0", "1" });
	m_nStateCount = m_stateTexts.size();
	m_textFont = QFont("Arial", 16, QFont::Bold);
	m_pSlider = new Slider(this);
	m_pSlider->SetTextFont(m_textFont);

	SetSize(120, 40);
	SetStateTexts(m_stateTexts);

	m_stateColors = Qt::gray;
	setCursor(QCursor(Qt::PointingHandCursor));
}

SwitchButton::~SwitchButton()
{
}

void SwitchButton::SetSize(int nWidth, int nHeight)
{
	resize(nWidth, nHeight);
	setFixedSize(nWidth, nHeight);
	
	m_pSlider->resize((width() + height()/2) / m_nStateCount, height());
	m_pSlider->move(0, 0);
	m_nArcRadius = std::min(width(), height());
	m_nRectWidth = width() - m_nArcRadius;
	SetStateTexts(m_stateTexts);
}

void SwitchButton::SetStateTexts(QStringList stateTexts)
{
	int count = stateTexts.size();
	if (count < 2) return;
	m_nStateCount = count;
	m_statePositions.resize(m_nStateCount);

	m_pSlider->resize((width() + height()/2) / m_nStateCount, height());
	m_pSlider->move(0, 0);

	for (int i = 0; i < m_nStateCount; ++i)
	{
		m_statePositions[i] = i * ((width() - m_pSlider->width()) / (m_nStateCount - 1));
	}

	m_stateTexts = stateTexts;
	update();
}

void SwitchButton::SetBackgoundColor(const QColor& color)
{
	m_stateColors = color;
	update();
}

void SwitchButton::SetSlideColor(const QColor& color)
{
	m_pSlider->SetSliderColor(color);
}

void SwitchButton::SetSlideTextFont(const QFont& font) {
	m_pSlider->SetTextFont(font);
}


void SwitchButton::SetState(int state)
{
	if (state < 0 || state >= m_nStateCount) return;
	m_currentState = state;

	m_pSlider->SetText("");

	QPropertyAnimation* pAnimation = new QPropertyAnimation(m_pSlider, "geometry");
	pAnimation->setDuration(m_nDuration);
	pAnimation->setStartValue(m_pSlider->geometry());
	pAnimation->setEndValue(QRect(
		m_statePositions[m_currentState],
		0,
		m_pSlider->width(),
		m_pSlider->height()
	));

	connect(pAnimation, &QPropertyAnimation::finished, this, [&] {
		m_bClicked = true;
		m_pSlider->SetText(m_stateTexts[m_currentState]);
	});

	connect(pAnimation, &QPropertyAnimation::valueChanged, this, [&](const QVariant&) {
		update();
	});

	pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
	emit Clicked(m_currentState);
}

QString SwitchButton::GetState() const
{
	return m_stateTexts[m_currentState];
}

void SwitchButton::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setPen(Qt::NoPen);
	p.setBrush(QBrush(m_stateColors));

	QPainterPath leftPath;
	leftPath.addEllipse(0, 0, m_nArcRadius, m_nArcRadius);

	QPainterPath middlePath;
	middlePath.addRect(m_nArcRadius / 2, 0, m_nRectWidth, m_nArcRadius);

	QPainterPath rightPath;
	rightPath.addEllipse(m_nRectWidth, 0, m_nArcRadius, m_nArcRadius);

	QPainterPath path = leftPath + middlePath + rightPath;

	p.drawPath(path);

	p.setPen(Qt::white);
	p.setFont(m_textFont);
	for (int i = 0; i < m_nStateCount; ++i)
	{
		QRect textRect(m_statePositions[i], 0, m_pSlider->width(), height());
		p.drawText(textRect, Qt::AlignCenter, m_stateTexts[i]);
	}
}

void SwitchButton::mousePressEvent(QMouseEvent *event)
{
	int clickX = event->pos().x();
	for (int i = 0; i < m_nStateCount; ++i)
	{
		if (clickX < m_statePositions[i] + m_pSlider->width() && clickX > m_statePositions[i])
		{
			SetState(i);
			QWidget::mousePressEvent(event);
		}
	}
}

Slider::Slider(QWidget *parent) : QWidget(parent)
{
	m_sliderColor = Qt::blue;
}

Slider::~Slider()
{
}

void Slider::SetSliderColor(const QColor &color)
{
	m_sliderColor = color;
	update();
}

void Slider::SetText(const QString &text) {
	m_text = text;
	update();
}

void Slider::SetTextFont(const QFont &font) {
	m_textFont = font;
	update();
}

void Slider::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	p.fillRect(rect(), Qt::transparent);
	p.setBrush(m_sliderColor);
	p.setPen(Qt::NoPen);
	p.drawRoundedRect(rect(), height() / 2, height() / 2);

	p.setPen(Qt::white);
	p.setFont(m_textFont);
	p.drawText(rect(), Qt::AlignCenter, m_text);
	QWidget::paintEvent(event);
}