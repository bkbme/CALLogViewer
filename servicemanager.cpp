#include "servicemanager.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

const QString URL_FORMAT_SERVICE = "http://%1:81/service.sh";

ServiceManager::ServiceManager(QNetworkAccessManager *netMgr, QObject *parent) :
	QObject(parent),
	m_net(netMgr)
{
}

void ServiceManager::requestFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	if (!reply)
	{
		return;
	}

	QString service = reply->request().url().queryItemValue("service");
	QString command = reply->request().url().queryItemValue("command");

	if (reply->error() != QNetworkReply::NoError)
	{
		statusMessage(QString("Failed to %1 service %2: %3")
					  .arg((command == "status" ? "get status of" : command), service, reply->errorString()), 5000);
	}

	QString result = reply->readAll();
	reply->deleteLater();

	if (result.contains("Stopping ") || result.contains("is not running"))
	{
		emit serviceStopped(service);
		return;
	}

	if (result.contains("Starting ") || result.contains("is running."))
	{
		emit serviceStarted(service);
		return;
	}

	if (result.contains("Restarting "))
	{
		emit serviceStopped(service);
		emit serviceStarted(service);
	}
}

void ServiceManager::invoke(const QString &service, const QString &command)
{
	if (m_host.isEmpty())
	{
		return;
	}

	QUrl url(QString(URL_FORMAT_SERVICE).arg(m_host));
	url.addQueryItem("service", service);
	url.addQueryItem("command", command);

	QNetworkReply *reply = m_net->get(QNetworkRequest(url));
	connect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
}
