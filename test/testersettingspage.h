#ifndef TESTERSETTINGSPAGE_H
#define TESTERSETTINGSPAGE_H

#include <abstractsettingspage.h>

class FemtecTester;

namespace Ui {
	class TesterSettingsPage;
}

class TesterSettingsPage : public AbstractSettingsPage
{
	Q_OBJECT

public:
	explicit TesterSettingsPage(FemtecTester *tester, QWidget *parent = 0);
	~TesterSettingsPage();

	QIcon icon() const;

	void loadSettings();
	void saveSettings();

signals:
	
public slots:

private:
	Ui::TesterSettingsPage *ui;
	FemtecTester *m_tester;
};

#endif // TESTERSETTINGSPAGE_H
