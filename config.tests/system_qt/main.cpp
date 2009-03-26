#include <QtCore>
#include <QtSql>
#include <QtSvg>
#include <stdio.h>

int main( int argc, char **argv )
{
    QCoreApplication a( argc, argv );
    Q_UNUSED(a);

    if ( !QSqlDatabase::isDriverAvailable("QSQLITE") ) {
        printf("QSQLITE driver is not available\n");
        return 1;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    return 0;
}

