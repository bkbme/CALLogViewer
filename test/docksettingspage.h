#ifndef DOCKSETTINGSPAGE_H
#define DOCKSETTINGSPAGE_H

#include <abstractsettingspage.h>
#include <autodock.h>

namespace Ui {
class DockSettingsPage;
}

class DockSettingsPage : public AbstractSettingsPage
{
	Q_OBJECT
	
public:
	explicit DockSettingsPage(AutoDock *dock, QWidget *parent = 0);
	~DockSettingsPage();

	QIcon icon() const;

	void accept();
	void reject();
	void apply();
	void reset();

	void loadSettings();
	void saveSettings();

public slots:
	void setZForce(qreal force);

private slots:
	void on_pbBottom_clicked();
	void on_pbUndock_clicked();
	void on_pbDock_clicked();
	void on_pbGrabRegular_clicked();
	void on_pbGrabSoft_clicked();
	void on_pbTare_clicked();

private:
	Ui::DockSettingsPage *ui;
	AutoDock *m_dock;
	AutoDock::DockingLimits m_zeroLimits;
	AutoDock::DockingLimits m_softLimits;
	AutoDock::DockingLimits m_regularLimits;
	AutoDock::DockingMode m_dockingMode;
};

#endif // DOCKSETTINGSPAGE_H
