#include "fsGUI.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qInstallMessageHandler(moduleMessageOutput);
    setModuleName("FSGui");
    QApplication a(argc, argv);
    QString filename="GUIDebug.txt";
    QFile file( filename );QTextStream stream( &file );
    if ( file.open(QIODevice::WriteOnly|QIODevice::Truncate) )
    {
        stream << "Application created." << endl;
    }
    qDebug().noquote() << "FSGui/FSData Version Info:\n" << GlobalConfiguration::getVersionInfo(false); //print version info to debug log
    FSGui w(a.arguments());
    stream << "FSGUI object created." << endl;
    w.show();
    stream << "Show command executed." << endl;

    return a.exec();
}

