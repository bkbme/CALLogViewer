#ifndef CALSESSIONMODEL_H
#define CALSESSIONMODEL_H

#include <QList>
#include <calsession.h>
#include <QAbstractTableModel>

class SysLogParser;
class QFontMetrics;

class CALSessionModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit CALSessionModel(SysLogParser *parser, QObject *parent = 0);
	virtual ~CALSessionModel();

	virtual QStringList mimeTypes () const;
	virtual Qt::ItemFlags flags(const QModelIndex &index ) const;

	int rowCount(const QModelIndex &parent = QModelIndex()) const ;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	CALSession* sessionAtRow(int row) const;

	bool isContinuous() const { return m_continuous; }
	int columnMinSize(int column, const QFontMetrics &metrics) const;

public slots:
	void clear();
	void startSession();
	void endSession(int exitCode);
	void appendLogMessage(const LogMessage &log);
	void emergencyShutdown(int errorCode, const QString &message);

protected:
	void appendSession(CALSession *session);
	void setSessions(const QList<CALSession*> &sessions);

signals:
	void finishedLoading();

private slots:
	void onLogClose();
	void setIsContinuous(bool continuous) { m_continuous = continuous; }

private:
	QList<CALSession*> m_sessions;
	bool m_continuous;
};

#endif // CALSESSIONMODEL_H
