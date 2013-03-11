#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

const QString APPLICATION_NAME = "CALLogView";
const QString ORGANIZATION_NAME = "TPV-ARGES";

const int VERSION_MAJOR = 1;
const int VERSION_MINOR = 0;

#ifdef Q_WS_WIN
	const QString TAR_EXEC = "tar.exe";
	const QString GZIP_EXEC = "gzip.exe";
#else
	const QString TAR_EXEC = "/bin/tar";
#endif

#endif // CONFIG_H
