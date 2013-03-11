#include "calsessionview.h"

#include <calsession.h>
#include <calsessionmodel.h>

#include <QUrl>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QScrollBar>
#include <QItemSelection>
#include <QItemSelectionModel>

#include <QDebug>

CALSessionView::CALSessionView(QWidget *parent) :
	QTreeView(parent)
{
}

void CALSessionView::setModel(QAbstractItemModel *newModel)
{
	CALSessionModel *sessionModel = qobject_cast<CALSessionModel*>(newModel);
	if (sessionModel)
	{
		connect(sessionModel, SIGNAL(finishedLoading()), this, SLOT(selectLastSession()));
		QTreeView::setModel(sessionModel);

		for (int i = 0; i < (sessionModel->columnCount() - 1); ++i) // resize columns except for last column using all remaining space
		{
			resizeColumnToContents(i);
		}

		setAcceptDrops(true);
		setDropIndicatorShown(true);
		viewport()->setAcceptDrops(true);
		setDragDropMode(QAbstractItemView::DropOnly);
	}
}

void CALSessionView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
	CALSessionModel *sessionModel = qobject_cast<CALSessionModel*>(model());
	if (sessionModel && current.row() < sessionModel->rowCount())
	{
		emit currentSessionChanged(sessionModel->sessionAtRow(current.row())); // select last session when done loading support info / disconnected
	}
	
	QTreeView::currentChanged(current, previous);
}

void CALSessionView::rowsInserted(const QModelIndex & parent, int start, int end)
{
	bool wasAtBottom = (verticalScrollBar()->sliderPosition() == verticalScrollBar()->maximum());
	CALSessionModel *sessionModel = qobject_cast<CALSessionModel*>(model());
	if (!sessionModel)
	{
		return;
	}

	if (sessionModel->isContinuous() && (start == sessionModel->rowCount() - 1) && (currentIndex().row() == (sessionModel->rowCount() - 2)))
	{
		setCurrentIndex(model()->index(model()->rowCount() - 1, 0)); // keep track of active session...
	}

	if (!sessionModel->isContinuous() && !currentIndex().isValid())
	{
		setCurrentIndex(model()->index(0, 0)); // select first row when loading support info file
	}

	QTreeView::rowsInserted(parent, start, end);
	
	if (wasAtBottom)
	{
		scrollToBottom();
	}
}

CALSession *CALSessionView::currentSession() const
{
	if (model() && currentIndex().isValid())
	{
		CALSessionModel *sessionModel = qobject_cast<CALSessionModel*>(model());
		return sessionModel->sessionAtRow(currentIndex().row());
	}

	return 0;
}

bool CALSessionView::hasCurrentSession() const
{
	return (model() && currentIndex().isValid());
}

void CALSessionView::selectLastSession()
{
	qDebug() << "CALSessionView::selectLastSession()";
	if (model() && model()->rowCount())
	{
		setCurrentIndex(model()->index(model()->rowCount() - 1, 0));
	}
}

int CALSessionView::sizeHintForColumn(int column) const
{
	CALSessionModel *sessionModel = qobject_cast<CALSessionModel*>(model());
	if (!sessionModel)
	{
		return -1;
	}

	return sessionModel->columnMinSize(column, fontMetrics()) + 10;
}

void CALSessionView::dropEvent(QDropEvent* ev)
{
	if(ev->mimeData()->hasUrls()) {
		qDebug() << "file dropped: " << ev->mimeData()->urls().at(0).toString();
		emit supportInfoDropped(ev->mimeData()->urls().at(0));
	}
}
