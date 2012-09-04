#ifndef FEMTECTESTERDIALOG_H
#define FEMTECTESTERDIALOG_H

#include <QDialog>
#include "femtectester.h"

namespace Ui {
class FemtecTesterDialog;
}

class FemtecTesterDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FemtecTesterDialog(FemtecTester *tester, QWidget *parent = 0);
	~FemtecTesterDialog();
	
public slots:
	virtual void accept();

private:
	void load();
	void store();
	
	Ui::FemtecTesterDialog *ui;
	FemtecTester *m_tester;
};

#endif // FEMTECTESTERDIALOG_H
