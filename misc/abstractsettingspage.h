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

	virtual void loadSettings() = 0;
	virtual void saveSettings() = 0;
};

#endif // ABSTRACTSETTINGSPAGE_H
