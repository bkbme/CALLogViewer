#ifndef DOCKSETTINGSPAGE_H
#define DOCKSETTINGSPAGE_H

#include <abstractsettingspage.h>

namespace Ui {
class DockSettingsPage;
}

class FemtoTester;

class DockSettingsPage : public AbstractSettingsPage
{
	Q_OBJECT
	
public:
	explicit DockSettingsPage(FemtoTester *tester, QWidget *parent = 0);
	~DockSettingsPage();

	QIcon icon() const;

	void loadSettings();
	void saveSettings();

public slots:
	void setDockingForce(quint16 dms, quint16 ref);

private slots:
	void on_pbBottom_clicked();
	
	void on_pbUndock_clicked();
	
	void on_pbDock_clicked();
	
private:
	Ui::DockSettingsPage *ui;
	FemtoTester *m_tester;
};

#endif // DOCKSETTINGSPAGE_H
