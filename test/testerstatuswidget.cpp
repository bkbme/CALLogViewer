#include "testerstatuswidget.h"

#include <QLabel>
#include <QHBoxLayout>

const QString CSS_DMS				= "border: 1px solid LightSlateGray; border-radius: 5px;";
const QString CSS_FS_INTERMEDIATE	= "border: 1px solid LightSlateGray; border-radius: 5px; background-color: #FFFF4D";
const QString CSS_FS_RELEASED		= "border: 1px solid LightSlateGray; border-radius: 5px; text-align:center";
const QString CSS_FS_PRESSED		= "border: 1px solid LightSlateGray; border-radius: 5px; background-color: #66FF33";
const QString CSS_FS_ERROR			= "border: 1px solid LightSlateGray; border-radius: 5px; background-color: #CC0000";

const QString TEXT_FS_INTERMEDIATE	= "Intermed.";
const QString TEXT_FS_RELEASED		= "PedalUp";
const QString TEXT_FS_PRESSED		= "PedalDown";
const QString TEXT_FS_ERROR			= "Error";

const QString FORMAT_TEXT_DMS		= "Z-Force: %1";

TesterStatusWidget::TesterStatusWidget(QWidget *parent) :
	QWidget(parent),
	m_lFS(new QLabel(this)),
	m_lDMS(new QLabel(this))
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(m_lDMS);
	layout->addWidget(m_lFS);

	m_lFS->setMinimumWidth(m_lFS->fontMetrics().width(TEXT_FS_PRESSED) + 10);

	m_lFS->setText(TEXT_FS_RELEASED);
	m_lFS->setStyleSheet(CSS_FS_RELEASED);
	m_lFS->setAlignment(Qt::AlignHCenter);

	m_lDMS->setStyleSheet(CSS_DMS);
	m_lDMS->hide();
}

void TesterStatusWidget::setFootswitchState(ProcedureFootswitch::FootswitchState state)
{
	switch (state)
	{
		case ProcedureFootswitch::FSIntermediate:
			m_lFS->setText(TEXT_FS_INTERMEDIATE);
			m_lFS->setStyleSheet(CSS_FS_INTERMEDIATE);
			break;
		case ProcedureFootswitch::FSPedalUp:
			m_lFS->setText(TEXT_FS_RELEASED);
			m_lFS->setStyleSheet(CSS_FS_RELEASED);
			break;
		case ProcedureFootswitch::FSPedalDown:
			m_lFS->setText(TEXT_FS_PRESSED);
			m_lFS->setStyleSheet(CSS_FS_PRESSED);
			break;
		default:
			m_lFS->setText(TEXT_FS_ERROR);
			m_lFS->setStyleSheet(CSS_FS_ERROR);
			break;
	}
}

void TesterStatusWidget::setDockingPressure(double dms)
{
	m_lDMS->setText(QString(FORMAT_TEXT_DMS).arg(QString::number(dms, 'f', 2)));
}

void TesterStatusWidget::setDockingStatusEnabled(bool enabled)
{
	m_lDMS->setVisible(enabled);
}
