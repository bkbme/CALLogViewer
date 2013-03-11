#ifndef ANIMATEDSPLITTER_H
#define ANIMATEDSPLITTER_H

#include <QSplitter>

class AnimatedSplitter : public QSplitter
{
	Q_OBJECT
public:
	explicit AnimatedSplitter(QWidget *parent = 0);

	void setSplitterPosition(int position);
	int splitterPosition() const;
	bool isVisible(int index);
	int bottom() const;

	Q_PROPERTY(int splitterPosition
			   READ splitterPosition
			   WRITE setSplitterPosition)

public slots:
	void setWidgetVisible(int index, bool visible);

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);

private:
	int m_lastSplitterPosition;
};

#endif // ANIMATEDSPLITTER_H
