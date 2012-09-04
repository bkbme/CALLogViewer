#include "femtectesterdialog.h"
#include "ui_femtectesterdialog.h"
#include "qextserialenumerator.h"

FemtecTesterDialog::FemtecTesterDialog(FemtecTester *tester, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FemtecTesterDialog),
	m_tester(tester)
{
	ui->setupUi(this);

	foreach(const QextPortInfo port, QextSerialEnumerator::getPorts())
	{
		ui->cbComPort->addItem(port.portName);
		if (m_tester->port() == port.portName)
		{
			ui->cbComPort->setCurrentIndex(ui->cbComPort->count() - 1);
		}
	}

	load();
	setWindowTitle("Femtec Tester Settings");
}

FemtecTesterDialog::~FemtecTesterDialog()
{
	delete ui;
}

void FemtecTesterDialog::accept()
{
	store();
	setResult(QDialog::Accepted);
	close();
}

void FemtecTesterDialog::load()
{
	ui->dsIntermediateMin->setValue(m_tester->intermediateFsStateLength().first);
	ui->dsIntermediateMax->setValue(m_tester->intermediateFsStateLength().second);
	ui->dsPauseDelayMin->setValue(m_tester->pauseDelay().first);
	ui->dsPauseDelayMax->setValue(m_tester->pauseDelay().second);
	ui->dsPauseLengthMin->setValue(m_tester->pauseLength().first);
	ui->dsPauseLengthMax->setValue(m_tester->pauseLength().second);
	ui->dsTrmDelayMin->setValue(m_tester->treatmentDelay().first);
	ui->dsTrmDelayMax->setValue(m_tester->treatmentDelay().second);
	ui->cbFakeTrmPause->setChecked(m_tester->fakeTreatmentPause());
}

void FemtecTesterDialog::store()
{
	m_tester->setTreatmentDelay(TimingLimits(ui->dsTrmDelayMin->value(), ui->dsTrmDelayMax->value()));
	m_tester->setPauseDelay(TimingLimits(ui->dsPauseDelayMin->value(), ui->dsPauseDelayMax->value()));
	m_tester->setPauseLength(TimingLimits(ui->dsPauseLengthMin->value(), ui->dsPauseLengthMax->value()));
	m_tester->setIntermediateFsStateLength(TimingLimits(ui->dsIntermediateMin->value(), ui->dsIntermediateMax->value()));
	m_tester->setFakeTreatmentPause(ui->cbFakeTrmPause->isChecked());
	m_tester->setPort(ui->cbComPort->currentText());
	
	m_tester->saveSettings();
}
