#ifdef QTOPIA_APP_INTERFACE
#undef QTOPIA_APP_INTERFACE
#endif

#ifdef REAL_FILE
#include REAL_FILE
#else

#ifdef LINK_QTOPIA

#include <QWidget>
#include <qtopiaapplication.h>

class LTMain : public QWidget
{
public:
    LTMain( QWidget *parent = 0, Qt::WFlags flags = 0 )
        : QWidget( parent, flags )
    {
    }
};
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, LTMain)
QTOPIA_MAIN

#elif defined(LINK_QT)

#include <QWidget>
#include <QApplication>

class LTMain : public QWidget
{
public:
    LTMain( QWidget *parent = 0, Qt::WFlags flags = 0 )
        : QWidget( parent, flags )
    {
    }
};

int main( int argc, char **argv )
{
    QApplication app(argc, argv);
    LTMain w;
    w.show();
    return app.exec();
}

#else

int main( int argc, char **argv )
{
    return 0;
}

#endif

#endif


