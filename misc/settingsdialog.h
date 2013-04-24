#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QAbstractButton;
class QListWidgetItem;
class AbstractSettingsPage;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit SettingsDialog(QWidget *parent = 0);
	~SettingsDialog();

public slots:
	void accept();
	void reject();
	void apply();
	void reset();
	void buttonClicked(QAbstractButton *button);
	void addPage(AbstractSettingsPage *page);
	void changePage(QListWidgetItem *current, QListWidgetItem *previous);

private:
	Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
