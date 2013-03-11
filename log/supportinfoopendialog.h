#ifndef SUPPORTINFOOPENDIALOG_H
#define SUPPORTINFOOPENDIALOG_H

#include <QUrl>
#include <QDialog>
#include <QString>
#include <QStringList>

class SysLogParser;
class QItemSelection;

namespace Ui {
class SupportInfoOpenDialog;
}

class SupportInfoOpenDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit SupportInfoOpenDialog(SysLogParser *parser, const QUrl &url, QWidget *parent = 0);
	~SupportInfoOpenDialog();

	QStringList selectedLogfiles() const;
	QString selectedSupportInfoFile() const;

public slots:
	void loadSyslogFileList(const QString& supportInfoFileName);

private slots:
	void on_pbBrowse_clicked();
	void onReadyReadFileList();
	void onFileListFinished();
	void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	
private:
	Ui::SupportInfoOpenDialog *ui;
	SysLogParser *m_parser;
};

#endif // SUPPORTINFOOPENDIALOG_H
