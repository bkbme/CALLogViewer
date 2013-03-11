#include "animatedsplitter.h"

#include <QList>
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QPropertyAnimation>

AnimatedSplitter::AnimatedSplitter(QWidget *parent) :
	QSplitter(parent)
{
	qApp->installEventFilter(this);
}

void AnimatedSplitter::setSplitterPosition(int position)
{
	moveSplitter(position, 1);
}

int AnimatedSplitter::splitterPosition() const
{
	return (sizes().count() ? sizes().at(0) : 0);
}

bool AnimatedSplitter::isVisible(int index)
{
	return (splitterPosition() != (index ? bottom() : 0));
}

int AnimatedSplitter::bottom() const
{
	int totalSize = 0;
	foreach (int size, sizes())
	{
		totalSize += size;
	}

	return totalSize;
}

void AnimatedSplitter::setWidgetVisible(int index, bool visible)
{
	if (sizes().count() < 2)
	{
		return;
	}

	if (!visible)
	{
		m_lastSplitterPosition = splitterPosition();
	}

	QPropertyAnimation *ani = new QPropertyAnimation(this, "splitterPosition");
	ani->setStartValue(visible ? (index ? bottom() : 0) : m_lastSplitterPosition);
	ani->setEndValue(visible ? m_lastSplitterPosition : (index ? bottom() : 0));
	ani->setDuration(150);
	ani->setEasingCurve(visible ? QEasingCurve::OutExpo : QEasingCurve::InExpo);
	ani->start(QAbstractAnimation::DeleteWhenStopped);
	//qDebug() << "start: " << ani->startValue() << " end: " << ani->endValue();
}

bool AnimatedSplitter::eventFilter(QObject *obj, QEvent *event)
 {
	if (event && event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		switch (keyEvent->key())
		{
			case '`':
				setWidgetVisible(0, !isVisible(0));
				break;
			default:
				break;
		}
	}

	return QSplitter::eventFilter(obj, event);
 }
