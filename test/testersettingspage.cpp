#include "testersettingspage.h"

#include <ui_testersettingspage.h>
#include <qextserialenumerator.h>
#include <procedurefootswitch.h>
#include <femtotester.h>
//#include <femtectester.h>

TesterSettingsPage::TesterSettingsPage(FemtoTester *tester, ProcedureFootswitch *fs, QWidget *parent) :
	AbstractSettingsPage(parent),
	ui(new Ui::TesterSettingsPage()),
	m_tester(tester),
	m_fs(fs)
{
	Q_ASSERT(tester);
	Q_ASSERT(fs);

	ui->setupUi(this);
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
	if (m_fs)
	{
		ui->dsIntermediateMin->setValue(m_fs->intermediateFsStateLength().first);
		ui->dsIntermediateMax->setValue(m_fs->intermediateFsStateLength().second);
		ui->dsPauseDelayMin->setValue(m_fs->pauseDelay().first);
		ui->dsPauseDelayMax->setValue(m_fs->pauseDelay().second);
		ui->dsPauseLengthMin->setValue(m_fs->pauseLength().first);
		ui->dsPauseLengthMax->setValue(m_fs->pauseLength().second);
		ui->dsTrmDelayMin->setValue(m_fs->treatmentDelay().first);
		ui->dsTrmDelayMax->setValue(m_fs->treatmentDelay().second);
		ui->cbFakeTrmPause->setChecked(m_fs->fakeTreatmentPause());
	}

	if (m_tester)
	{
		ui->cbComPort->clear();
		foreach(const QextPortInfo port, QextSerialEnumerator::getPorts())
		{
			ui->cbComPort->addItem(port.portName);
			if (m_tester->port() == port.portName)
			{
				ui->cbComPort->setCurrentIndex(ui->cbComPort->count() - 1);
			}
		}
	}
}

void TesterSettingsPage::saveSettings()
{
	if (m_fs)
	{
		m_fs->setTreatmentDelay(TimingLimits(ui->dsTrmDelayMin->value(), ui->dsTrmDelayMax->value()));
		m_fs->setPauseDelay(TimingLimits(ui->dsPauseDelayMin->value(), ui->dsPauseDelayMax->value()));
		m_fs->setPauseLength(TimingLimits(ui->dsPauseLengthMin->value(), ui->dsPauseLengthMax->value()));
		m_fs->setIntermediateFsStateLength(TimingLimits(ui->dsIntermediateMin->value(), ui->dsIntermediateMax->value()));
		m_fs->setFakeTreatmentPause(ui->cbFakeTrmPause->isChecked());

		m_fs->saveSettings();
	}

	if (m_tester)
	{
		m_tester->setPort(ui->cbComPort->currentText());
		m_tester->saveSettings();
	}
}
