#include "testerstatuswidget.h"
#include <femtotester.h>
#include <autodock.h>
#include <QLabel>
#include <QHBoxLayout>

const QString CSS_DOCK				= "border: 1px solid LightSlateGray; border-radius: 5px;";
const QString CSS_FS_INTERMEDIATE	= "border: 1px solid LightSlateGray; border-radius: 5px; background-color: #FFFF4D";
const QString CSS_FS_RELEASED		= "border: 1px solid LightSlateGray; border-radius: 5px; text-align:center";
const QString CSS_FS_PRESSED		= "border: 1px solid LightSlateGray; border-radius: 5px; background-color: #66FF33";
const QString CSS_FS_ERROR			= "border: 1px solid LightSlateGray; border-radius: 5px; background-color: #CC0000";

const QString TEXT_FS_INTERMEDIATE	= "Intermed.";
const QString TEXT_FS_RELEASED		= "PedalUp";
const QString TEXT_FS_PRESSED		= "PedalDown";
const QString TEXT_FS_ERROR			= "Error";

const QString FORMAT_TEXT_FORCE		= "Z-Force: %1%2";

TesterStatusWidget::TesterStatusWidget(FemtoTester *tester, AutoDock *dock, QWidget *parent) :
	QWidget(parent),
	m_tester(tester),
	m_dock(dock),
	m_lFS(new QLabel(this)),
	m_lForce(new QLabel(this))
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(m_lForce);
	layout->addWidget(m_lFS);

	m_lFS->setMinimumWidth(m_lFS->fontMetrics().width(TEXT_FS_PRESSED) + 10);
	m_lForce->setMinimumWidth(m_lForce->fontMetrics().width(QString(FORMAT_TEXT_FORCE).arg("999.9", "g")) + 10);

	m_lFS->setText(TEXT_FS_RELEASED);
	m_lFS->setStyleSheet(CSS_FS_RELEASED);
	m_lFS->setAlignment(Qt::AlignHCenter);

	m_lForce->setStyleSheet(CSS_DOCK);

	connect(m_tester, SIGNAL(connectedStateChanged(bool)), this, SLOT(setVisible(bool)));
	connect(m_tester, SIGNAL(connectedStateChanged(bool)), m_lForce, SLOT(hide()));
	connect(m_tester, SIGNAL(footswitchState(ProcedureFootswitch::FootswitchState)), this, SLOT(setFootswitchState(ProcedureFootswitch::FootswitchState)));
	connect(m_dock, SIGNAL(zForceChanged(qreal)), this, SLOT(setZForce(qreal)));

	hide();
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

void TesterStatusWidget::setZForce(qreal force)
{
	if (m_lForce->isHidden())
	{
		m_lForce->show();
	}

	m_lForce->setText(QString(FORMAT_TEXT_FORCE).arg(QString::number(force, 'f', 1)).arg(m_dock->forceIsSteady() ? 'g' : ' '));
}

