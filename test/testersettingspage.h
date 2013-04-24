#ifndef TESTERSETTINGSPAGE_H
#define TESTERSETTINGSPAGE_H

#include <abstractsettingspage.h>

//class FemtecTester;
class FemtoTester;
class ProcedureFootswitch;

namespace Ui {
	class TesterSettingsPage;
}

class TesterSettingsPage : public AbstractSettingsPage
{
	Q_OBJECT

public:
	//explicit TesterSettingsPage(FemtecTester *tester, QWidget *parent = 0);
	explicit TesterSettingsPage(FemtoTester *tester, ProcedureFootswitch *fs, QWidget *parent = 0);
	~TesterSettingsPage();

	QIcon icon() const;

	void accept();
	void reject();
	void apply();
	void reset();

	void loadSettings();
	void saveSettings();

private:
	Ui::TesterSettingsPage *ui;
	FemtoTester *m_tester;
	ProcedureFootswitch *m_fs;
};

#endif // TESTERSETTINGSPAGE_H
