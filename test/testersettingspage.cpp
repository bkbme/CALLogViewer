#include "testersettingspage.h"

#include <ui_testersettingspage.h>
#include <qextserialenumerator.h>
#include <femtectester.h>

TesterSettingsPage::TesterSettingsPage(FemtecTester *tester, QWidget *parent) :
	AbstractSettingsPage(parent),
	ui(new Ui::TesterSettingsPage()),
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

	setWindowTitle("Tester Settings");
}

TesterSettingsPage::~TesterSettingsPage()
{
	delete ui;
}

QIcon TesterSettingsPage::icon() const
{
	return QIcon(":/icons/test.png");
}

void TesterSettingsPage::loadSettings()
{
	if (m_tester)
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
}

void TesterSettingsPage::saveSettings()
{
	if (m_tester)
	{
		m_tester->setTreatmentDelay(TimingLimits(ui->dsTrmDelayMin->value(), ui->dsTrmDelayMax->value()));
		m_tester->setPauseDelay(TimingLimits(ui->dsPauseDelayMin->value(), ui->dsPauseDelayMax->value()));
		m_tester->setPauseLength(TimingLimits(ui->dsPauseLengthMin->value(), ui->dsPauseLengthMax->value()));
		m_tester->setIntermediateFsStateLength(TimingLimits(ui->dsIntermediateMin->value(), ui->dsIntermediateMax->value()));
		m_tester->setFakeTreatmentPause(ui->cbFakeTrmPause->isChecked());
		m_tester->setPort(ui->cbComPort->currentText());
		
		m_tester->saveSettings();
	}
}
