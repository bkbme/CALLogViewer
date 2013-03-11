#include <QtGui/QApplication>
#include <mainwindow.h>
#include <config.h>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setOrganizationName(ORGANIZATION_NAME);
	app.setApplicationName(APPLICATION_NAME);

	MainWindow w;
	w.show();

	return app.exec();
}
