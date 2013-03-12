#include "config.h"
#include "supportinfoopendialog.h"
#include "ui_supportinfoopendialog.h"
#include <syslogparser.h>

#include <QDebug>
#include <QString>
#include <QProcess>
#include <QFileDialog>
#include <QStringList>
#include <QItemSelection>
#include <QModelIndexList>
#include <QStandardItemModel>
#include <QItemSelectionModel>

SupportInfoOpenDialog::SupportInfoOpenDialog(SysLogParser *parser, const QUrl &url, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SupportInfoOpenDialog),
	m_parser(parser)
{
	ui->setupUi(this);
	
	QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
	if (okButton)
	{
		okButton->setEnabled(false); // disable ok button until at least one logfile is selected
	}

	QStandardItemModel *model = new QStandardItemModel(this);
	model->setColumnCount(3);
	model->setHorizontalHeaderItem(0, new QStandardItem("Date"));
	model->setHorizontalHeaderItem(1, new QStandardItem("Size (kB)"));
	model->setHorizontalHeaderItem(2, new QStandardItem("File"));
	
	ui->tvSyslogFiles->setModel(model);
	connect(ui->tvSyslogFiles->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(onSelectionChanged(QItemSelection,QItemSelection)));

	if (!url.isEmpty() && QFile::exists(url.toLocalFile()))
	{
		ui->leFilename->setText(url.toLocalFile());
		loadSyslogFileList(url.toLocalFile());
	}
}

SupportInfoOpenDialog::~SupportInfoOpenDialog()
{
	delete ui;
}

void SupportInfoOpenDialog::loadSyslogFileList(const QString& supportInfoFileName)
{
	if (!QFile::exists(supportInfoFileName))
	{
		qDebug() << "Can not open support info file: " << supportInfoFileName << " (file doesn't exist)";
		return;
	}

	if (!SysLogParser::canHandleTarArchive())
	{
		qDebug() << "Extracting zipped tar archives is not supported on this system";
		return;
	}

	QStringList args;
	QProcess *tar = new QProcess(this);
	connect(tar, SIGNAL(readyRead()), this, SLOT(onReadyReadFileList()));
	connect(tar, SIGNAL(finished(int)), this, SLOT(onFileListFinished()));

#ifdef Q_WS_WIN
	args << "-tv";

	QStringList gzip_args;
	gzip_args << "-dc" << supportInfoFileName;
	QProcess *gzip = new QProcess(tar);
	gzip->setStandardOutputProcess(tar); // tar.exe doesn't support on-the-fly gzip extraction :(
	gzip->start(QCoreApplication::applicationDirPath() + QDir::separator() + GZIP_EXEC, gzip_args);
	//qDebug() << "starting " << QCoreApplication::applicationDirPath() + QDir::separator() + GZIP_EXEC << gzip_args.join(" ");
#else
	args << "--wildcards" << "-tzvf" << supportInfoFileName;
#endif
	args << "var/log/messages*";

	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tvSyslogFiles->model());
	if (model)
	{
		model->setRowCount(0);
	}

#ifdef Q_WS_WIN
	tar->start(QCoreApplication::applicationDirPath() + QDir::separator() + TAR_EXEC, args);
	//qDebug() << "starting " << QCoreApplication::applicationDirPath() + QDir::separator() + TAR_EXEC << args.join(" ");
#else
	tar->start(TAR_EXEC, args);
#endif
}

void SupportInfoOpenDialog::on_pbBrowse_clicked()
{
	const QString filter("Syslog (Support Info (support_info_*.tar.gz);; All Files (*)");
	QString filename(QFileDialog::getOpenFileName(this, "Open Support Info File", QDir::homePath(), filter));
	
	if (!filename.isEmpty())
	{
		ui->leFilename->setText(filename);
		loadSyslogFileList(filename);
	}
}

void SupportInfoOpenDialog::onReadyReadFileList()
{
	QIODevice *device = qobject_cast<QIODevice*>(sender());
	while (device && device->canReadLine())
	{
		QStringList fileInfo(QString(device->readLine()).split(" ", QString::SkipEmptyParts));
		if(fileInfo.count() < 6)
		{
			qDebug() << "SupportInfoOpenDialog::onReadyReadFileList(): failed to parse tar output!";
			continue;
		}

		QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tvSyslogFiles->model());
		if (model)
		{
			QList<QStandardItem*> row;
			row << new QStandardItem(QString("%1  %2").arg(fileInfo.at(3).trimmed(), fileInfo.at(4).trimmed())) // modification timestamp
			    << new QStandardItem(QString::number(fileInfo.at(2).trimmed().toInt() / 1024))                  // size
			    << new QStandardItem(fileInfo.at(5).trimmed());                                                 // filename
			model->appendRow(row);
		}
	}
}

void SupportInfoOpenDialog::onFileListFinished()
{
	// process loading support info filelist finished...
	sender()->deleteLater();
}

void SupportInfoOpenDialog::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_UNUSED(selected);
	Q_UNUSED(deselected);

	QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
	if (okButton)
	{
		okButton->setEnabled(ui->tvSyslogFiles->selectionModel()->selectedIndexes().count() > 0);
	}
}

QStringList SupportInfoOpenDialog::selectedLogfiles() const
{
	QStringList selectedFiles;
	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tvSyslogFiles->model());
	if (model)
	{
		foreach (const QModelIndex &index, ui->tvSyslogFiles->selectionModel()->selectedRows(2))
		{
				selectedFiles << model->data(index).toString();
		}
	}

	return selectedFiles;
}

QString SupportInfoOpenDialog::selectedSupportInfoFile() const
{
	return ui->leFilename->text();
}
