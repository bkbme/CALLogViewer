#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

const QString APPLICATION_NAME = "CALLogView";
const QString ORGANIZATION_NAME = "TPV-ARGES";

const int VERSION_MAJOR = 1;
const int VERSION_MINOR = 8;
const bool DEVELOPER_BUILD = true;

#ifdef Q_WS_WIN
const QString TAR_EXEC = "tar.exe";
const QString GZIP_EXEC = "gzip.exe";
#else
const QString TAR_EXEC = "/bin/tar";
#endif

//const QString FEMTO_TEST_VERSION = "130621";
const QString FEMTO_TEST_VERSION = "140108";

const int SERVO_ZAXIS_ID = 0;
const int SERVO_XAXIS_ID = 1;

namespace CAL
{
// CAL exit codes, copied from cal_config.h
enum CAL_APPLICATION_EXITCODE
{
	APP_EXITCODE_NORMAL = 0,
	APP_EXITCODE_RESET = 1,
	APP_EXITCODE_MACHINEHALT = 2,
	APP_EXITCODE_EMERGENCY = 3,
	APP_EXITCODE_INTERNALERROR = 4,
	APP_EXITCODE_DAEMONFAIL = 5,
	APP_EXITCODE_CRASHED = 6
};
} // namespace CAL

#endif // CONFIG_H
