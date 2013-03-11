#ifndef CALSESSIONVIEW_H
#define CALSESSIONVIEW_H

#include <QTreeView>

class QUrl;
class QDropEvent;
class CALSession;
class QDragEnterEvent;

class CALSessionView : public QTreeView
{
	Q_OBJECT
public:
	explicit CALSessionView(QWidget *parent = 0);

	CALSession* currentSession() const;
	bool hasCurrentSession() const;
	virtual void setModel(QAbstractItemModel *newModel);

public slots:
	void selectLastSession();

signals:
	void supportInfoDropped(const QUrl& url);
	void currentSessionChanged(CALSession *session);

protected:
	virtual void currentChanged(const QModelIndex & current, const QModelIndex & previous);
	virtual void rowsInserted(const QModelIndex & parent, int start, int end);
	virtual int sizeHintForColumn(int column) const;
	virtual void dropEvent(QDropEvent *ev);
};

#endif // CALSESSIONVIEW_H
