#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>

namespace Ui {
class SearchWidget;
}

class QStringListModel;

enum SearchDirection { SearchNext, SearchPrevious };

class SearchWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SearchWidget(QWidget *parent = 0);
	~SearchWidget();

public slots:
	void findNext();
	void findPrevious();
	void loadHistory();
	void saveHistory();

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);

signals:
	void closed();
	void newSearch(const QString& term, Qt::CaseSensitivity cs);
	void continueSearch(SearchDirection direction);

private slots:
	void on_cbSearch_textChanged(const QString &arg1);
	void clearLastSearch();
	
private:
	Ui::SearchWidget *ui;
	QString m_lastSearch;
	Qt::CaseSensitivity m_lastCs;
	QStringListModel *m_historyModel;
};

#endif // SEARCHWIDGET_H
