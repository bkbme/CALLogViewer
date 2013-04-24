#ifndef LOGSETTINGSPAGE_H
#define LOGSETTINGSPAGE_H

#include <QIcon>
#include <QWidget>
#include <abstractsettingspage.h>

namespace Ui {
class LogSettingsPage;
}

class LogSettingsPage : public AbstractSettingsPage
{
	Q_OBJECT

public:
	explicit LogSettingsPage(QWidget *parent = 0);
	~LogSettingsPage();

	QIcon icon() const;

	void accept();
	void reject();
	void apply();
	void reset();

	void loadSettings();
	void saveSettings();

private:
	Ui::LogSettingsPage *ui;
};

#endif // LOGSETTINGSPAGE_H
