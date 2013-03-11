#include "searchwidget.h"
#include "ui_searchwidget.h"

#include <QSettings>
#include <QStringList>
#include <QStringListModel>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QLineEdit>
#include <QKeyEvent>

#include <QDebug>

const int HEIGHT = 25;
const int MAX_HISTORY_ASC = 15;

SearchWidget::SearchWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SearchWidget),
	m_lastSearch(),
	m_lastCs(Qt::CaseInsensitive),
	m_historyModel(new QStringListModel(this))
{
	ui->setupUi(this);
	ui->cbSearch->setModel(m_historyModel);

	hide();
	loadHistory();

	connect(ui->cbSearch->lineEdit(), SIGNAL(returnPressed()), ui->pbNext, SIGNAL(clicked()));
	connect(ui->pbClose, SIGNAL(clicked()), this, SIGNAL(closed()));
	connect(ui->pbClose, SIGNAL(clicked()), this, SLOT(clearLastSearch()));

	qApp->installEventFilter(this);
}

SearchWidget::~SearchWidget()
{
	saveHistory();
	delete ui;
}

void SearchWidget::loadHistory()
{
	QSettings settings;
	QStringList history;
	settings.beginGroup("Search");

	ui->cbCase->setChecked(settings.value("CaseSensitive", false).toBool());
	int size = settings.beginReadArray("History");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex(i);
		history << settings.value("term").toString();
	}
	m_historyModel->setStringList(history);

	settings.endArray();
	settings.endGroup();
}

void SearchWidget::saveHistory()
{
	QSettings settings;
	const QStringList history = m_historyModel->stringList();

	settings.beginGroup("Search");
	settings.setValue("CaseSensitive", ui->cbCase->isChecked());
	settings.beginWriteArray("History", history.count());
	for (int i = 0; i < history.count(); ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue("term", history.at(i));
	}

	settings.endArray();
	settings.endGroup();
}

void SearchWidget::findNext()
{
	const QString searchTerm(ui->cbSearch->currentText());
	if (!searchTerm.isEmpty())
	{
		if (!m_historyModel->stringList().contains(searchTerm, (ui->cbCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive)))
		{
			m_historyModel->insertRows(0, 1);
			m_historyModel->setData(m_historyModel->index(0), searchTerm);
			if (m_historyModel->rowCount() > MAX_HISTORY_ASC)
			{
				m_historyModel->removeRows(m_historyModel->rowCount() - 1, 1);
			}
			ui->cbSearch->setCurrentIndex(0);
		}

		Qt::CaseSensitivity cs = (ui->cbCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
		if (m_lastSearch == searchTerm && m_lastCs == cs)
		{
			continueSearch(SearchNext);
			return;
		}

		m_lastSearch = searchTerm;
		m_lastCs = cs;
		emit newSearch(m_lastSearch, cs);
	}
}

void SearchWidget::findPrevious()
{
	const QString searchTerm(ui->cbSearch->currentText());
	if (!searchTerm.isEmpty())
	{
		if (!m_historyModel->stringList().contains(searchTerm, (ui->cbCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive)))
		{
			m_historyModel->insertRows(0, 1);
			m_historyModel->setData(m_historyModel->index(0), searchTerm);
			if (m_historyModel->rowCount() > MAX_HISTORY_ASC)
			{
				m_historyModel->removeRows(m_historyModel->rowCount() - 1, 1);
			}
			ui->cbSearch->setCurrentIndex(0);
		}

		Qt::CaseSensitivity cs = (ui->cbCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
		if (m_lastSearch == searchTerm && m_lastCs == cs)
		{
			continueSearch(SearchPrevious);
			return;
		}

		m_lastSearch = searchTerm;
		m_lastCs = cs;
		emit newSearch(m_lastSearch, cs);
	}
}

void SearchWidget::on_cbSearch_textChanged(const QString &arg1)
{
	ui->pbNext->setDisabled(arg1.isEmpty());
	ui->pbPrevious->setDisabled(arg1.isEmpty());
}

void SearchWidget::clearLastSearch()
{
	m_lastSearch = "";
}

bool SearchWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (event && event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		switch (keyEvent->key())
		{
			case '/':
				show();
				ui->cbSearch->setFocus();
				break;
			case Qt::Key_Escape:
				hide();
				clearLastSearch();
				emit closed();
				break;
			default:
				break;
		}

		if (keyEvent->matches(QKeySequence::Find))
		{
			show();
			ui->cbSearch->setFocus();
			return true;
		}
		if (keyEvent->matches(QKeySequence::FindNext))
		{
			findNext();
			return true;
		}
		if (keyEvent->matches(QKeySequence::FindPrevious))
		{
			findPrevious();
			return true;
		}
	}

	return QWidget::eventFilter(obj, event);
}
