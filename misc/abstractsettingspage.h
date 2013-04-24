#ifndef ABSTRACTSETTINGSPAGE_H
#define ABSTRACTSETTINGSPAGE_H

#include <QWidget>
#include <QString>
#include <QIcon>

class AbstractSettingsPage : public QWidget
{
	Q_OBJECT

public:
	explicit AbstractSettingsPage(QWidget *parent = 0) : QWidget(parent) {}

	virtual QIcon icon() const = 0;

	virtual void accept() = 0;
	virtual void reject() = 0;
	virtual void apply() = 0;
	virtual void reset() = 0;
};

#endif // ABSTRACTSETTINGSPAGE_H
