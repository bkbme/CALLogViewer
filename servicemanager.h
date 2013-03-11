#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <QObject>

class QNetworkAccessManager;

class ServiceManager : public QObject
{
	Q_OBJECT
public:
	explicit ServiceManager(QNetworkAccessManager *netMgr, QObject *parent = 0);

	QString host() const { return m_host; }

signals:
	void serviceStarted(const QString &service);
	void serviceStopped(const QString &service);
	void statusMessage(const QString &msg, int timeout);

public slots:
	void setHost(const QString &host) { m_host = host; }
	void invoke(const QString &service, const QString &command);

private slots:
	void requestFinished();

private:
	QNetworkAccessManager *m_net;
	QString m_host;
};

#endif // SERVICEMANAGER_H
