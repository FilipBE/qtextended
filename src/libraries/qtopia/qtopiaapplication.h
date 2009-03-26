/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QTOPIAAPPLICATION_H
#define QTOPIAAPPLICATION_H

#include <qtopiaglobal.h>
#include <qtopianamespace.h>
#include <qapplication.h>
#include <qdialog.h>
#include <qtimestring.h>
#include <qcontent.h>
#include <QDebug>
#include <qtopialog.h>
#include <qtopiasxe.h>

class QtopiaChannel;
class QtopiaApplicationData;
struct QWSEvent;
struct QWSKeyEvent;
class PluginLibraryManager;
class ScreenSaver;
class QtopiaStyle;
class TestSlaveInterface;

extern char _key[];

class QTOPIA_EXPORT QtopiaApplication : public QApplication
{
    Q_OBJECT
    friend class QtopiaApplicationData;
public:
    QtopiaApplication( int& argc, char **argv);
    QtopiaApplication( int& argc, char **argv, Type);
    ~QtopiaApplication();

    static QtopiaApplication *instance();

    bool willKeepRunning() const;
    void registerRunningTask(const QString &, QObject * = 0);
    void unregisterRunningTask(const QString &);
    void unregisterRunningTask(QObject *);

#if !defined(QT_NO_SXE) || defined(Q_QDOC)
    enum SxeAuthorizerRole
    {
        SxeAuthorizerClientOnly = 0x01,
        SxeAuthorizerServerOnly = 0x02,
        SxeAuthorizerServerAndClient = SxeAuthorizerClientOnly | SxeAuthorizerServerOnly
    };

     SxeAuthorizerRole sxeAuthorizerRole() const;
    void setSxeAuthorizerRole( SxeAuthorizerRole status );
#endif

    virtual void applyStyle();

    enum StylusMode {
        LeftOnly,
        RightOnHold
        // RightOnHoldLeftDelayed, etc.
    };
    static void setStylusOperation( QWidget*, StylusMode );
    static StylusMode stylusOperation( QWidget* );

    enum InputMethodHint {
        Normal,
        AlwaysOff,
        AlwaysOn,
        Number,
        PhoneNumber,
        Words,
        Text,
        Named,
        ProperNouns
    };

    enum PowerConstraint {
        Disable = 0,  //disable all timeout actions
        DisableLightOff = 50, //do not turn backlight off
        DisableSuspend = 900, //do not suspend the device
        Enable = 1000 //enable all timeout actions
    };

    static void setInputMethodHint( QWidget *, InputMethodHint, const QString& param=QString() );
    static void setInputMethodHint( QWidget *, const QString& named );
    static InputMethodHint inputMethodHint( QWidget * );
    static QString inputMethodHintParam( QWidget * );
    static void hideInputMethod();
    static void showInputMethod();

    QWidget* mainWidget() const;
    void setMainWidget(QWidget *, bool nomax=false);
    void showMainWidget();
    void showMainDocumentWidget();

    void showMainWidget( QWidget*, bool nomax=false );
    void showMainDocumentWidget( QWidget*, bool nomax=false );
    static void showDialog( QDialog*, bool nomax=false );
    static int execDialog( QDialog*, bool nomax=false );
    static void setMenuLike( QDialog *, bool );
    static bool isMenuLike( const QDialog* );
    static void setPowerConstraint(PowerConstraint);    // libqtopia

    int exec();

    static void loadTranslations(const QString&);
    static void loadTranslations(const QStringList&);
    void initApp( int argc, char **argv );

    TestSlaveInterface* testSlave();

#ifdef Q_WS_X11
    class X11EventFilter {
    public:
        virtual ~X11EventFilter() {}

        virtual bool x11EventFilter(XEvent *) = 0;
    };
    void installX11EventFilter(X11EventFilter *);
    void removeX11EventFilter(X11EventFilter *);
#endif

signals:
    void clientMoused();
    void timeChanged();
    void categoriesChanged();
    void clockChanged( bool pm );
    void volumeChanged( bool muted );
    void appMessage( const QString& msg, const QByteArray& data);
    void weekChanged( bool startOnMonday );
    void dateFormatChanged();
    void flush();
    void reload();
    void contentChanged(const QContentIdList &,QContent::ChangeType);
    void resetContent();

private slots:
    void systemMessage( const QString &msg, const QByteArray &data );
    void pidMessage( const QString &msg, const QByteArray &data );
    void dotpidMessage( const QString &msg, const QByteArray &data );
    void removeSenderFromStylusDict();
    void removeSenderFromIMDict();
    void hideOrQuit();
    void lineEditTextChange(const QString &);
    void multiLineEditTextChange();
    void buttonChange(bool);
    void textBrowserHighlightChange(const QString &);
#ifndef QT_NO_WIZARD
    void wizardPageCompleteChanged();
#endif

    void removeFromWidgetFlags();
    void updateDialogGeometry();

protected:
    bool notify(QObject*,QEvent*);
#ifdef Q_WS_QWS
    bool qwsEventFilter( QWSEvent * );
#endif
#ifdef Q_WS_X11
    bool x11EventFilter( XEvent * );
#endif
    QtopiaStyle *internalSetStyle( const QString &style );
    virtual void restart();
    virtual void shutdown();
    bool eventFilter( QObject *, QEvent * );
    void timerEvent( QTimerEvent * );
    bool raiseAppropriateWindow();
    virtual void tryQuit();
    virtual void connectNotify(const char *signal);
    virtual void disconnectNotify(const char *signal);

private:
    void init(int argc, char **argv, Type t);

#ifdef Q_WS_QWS
    void mapToDefaultAction( QWSKeyEvent *ke, int defKey );
#endif
    void processQCopFile();
    void hideMessageBoxButtons( QMessageBox * );
    void commonInit( int &argc, char **argv );

    static void sendInputHintFor(QWidget*,QEvent::Type);
    static void inputMethodStatusChanged(QWidget* w);

    QtopiaApplicationData *d;
};

/*
    Macros for simplifying the support of multiple launch modes in dynamic and singleexec builds.

    The options are:
      - singleexec/quicklaunch
      - singleexec/manual (user-supplied main function)
      - dynamic/quicklaunch
      - dynamic/normal

    There's no need to support dynamic/manual because any old main function will do. It's required
    for singleexec because of the extra registration required to associate the main function with
    the application's name. singleexec/normal is not supported because there's no point (size/load
    penalties of quicklaunch don't exist).

    First up is the helpers. The primary macros (QTOPIA_ADD_APPLICATION, QTOPIA_MAIN) are below.
*/
#include <qmap.h>
#include <qapplicationplugin.h>
#include <qmetaobject.h>

// helper types/functions
typedef QWidget* (*qpeAppCreateFunc)(QWidget*,Qt::WFlags);
typedef QMap<QString,qpeAppCreateFunc> QPEAppMap;
extern void qtopia_registerApp(const char *name, qpeAppCreateFunc createFunc);

typedef int (*qpeMainFunc)(int,char**);
typedef QMap<QString,qpeMainFunc> QPEMainMap;
extern void qtopia_registerMain(const char *name, qpeMainFunc mainFunc);


// The SXE stuff (depends on quicklaunch or normal mode)
#ifdef QTOPIA_APP_INTERFACE
#define QTOPIA_APP_KEY QSXE_QL_APP_KEY
#define QTOPIA_SET_KEY QSXE_SET_QL_KEY
#else
#define QTOPIA_APP_KEY QSXE_APP_KEY
#define QTOPIA_SET_KEY QSXE_SET_APP_KEY
#endif

// Document system connection stuff
#ifdef QTOPIA_DIRECT_DOCUMENT_SYSTEM_CONNECTION
#define QTOPIA_DSCT QContent::DocumentSystemDirect
#else
#define QTOPIA_DSCT QContent::DocumentSystemClient
#endif
#define QTOPIA_SET_DOCUMENT_SYSTEM_CONNECTION()\
    do {\
        bool ok = QContent::setDocumentSystemConnection( QTOPIA_DSCT );\
        if ( !ok ) {\
            qWarning() << "WARNING: Cannot change document system connection type in file" << __FILE__ << "line" << __LINE__;\
        }\
    } while ( 0 )

// Register a main function (singleexec/manual)
#define QTOPIA_REGISTER_SINGLE_EXEC_MAIN(NAME,IMPLEMENTATION) \
    int main_##IMPLEMENTATION( int argc, char **argv );\
    static qpeMainFunc append_##IMPLEMENTATION() \
        { qtopia_registerMain(NAME, main_##IMPLEMENTATION); \
            return main_##IMPLEMENTATION; } \
    static qpeMainFunc dummy_##IMPLEMENTATION = \
        append_##IMPLEMENTATION();

// Register a widget (*/quicklaunch, dynamic/normal)
#define QTOPIA_REGISTER_WIDGET(NAME,IMPLEMENTATION) \
    static QWidget *create_##IMPLEMENTATION( QWidget *p, Qt::WFlags f ) \
        { return new IMPLEMENTATION(p, f); } \
    static qpeAppCreateFunc append_##IMPLEMENTATION() \
        { qtopia_registerApp(NAME, create_##IMPLEMENTATION); \
            return create_##IMPLEMENTATION; } \
    static qpeAppCreateFunc dummy_##IMPLEMENTATION = \
        append_##IMPLEMENTATION();

// Setup the app map (dynamic/*)
#define QTOPIA_SETUP_APP_MAP \
    QTOPIA_APP_KEY \
    QPEAppMap *qpeAppMap() { \
        static QPEAppMap *am = 0; \
        if ( !am ) am = new QPEAppMap(); \
        return am; \
    } \
    void qtopia_registerApp(const char *name, qpeAppCreateFunc createFunc) { \
        QPEAppMap *am = qpeAppMap(); \
        am->insert(name, createFunc); \
    }

// Setup the quicklaunch plugin (dynamic/quicklaunch)
#define QTOPIA_QUICKLAUNCH_IMPL \
    struct ApplicationImpl : public QApplicationPlugin { \
        ApplicationImpl() {} \
        virtual void setProcessKey( const QString &appName ) { \
            Q_UNUSED(appName); \
            QTOPIA_SET_KEY(qPrintable(appName)) \
        } \
        virtual QWidget *createMainWindow( const QString &appName, QWidget *parent, Qt::WFlags f ) { \
            QWidget* widget = 0; \
            if ( qpeAppMap()->contains(appName) ) { \
                qLog(Quicklauncher) << "creating main window for quicklaunched" << appName.toLocal8Bit().constData(); \
                QTOPIA_SET_DOCUMENT_SYSTEM_CONNECTION(); \
                widget = (*qpeAppMap())[appName](parent, f); \
                qLog(Quicklauncher) << "created main window for quicklaunched" << appName.toLocal8Bit().constData(); \
            } \
            return widget; \
        } \
        virtual QStringList keys() const { \
            QStringList list; \
            for ( QPEAppMap::Iterator it=qpeAppMap()->begin(); it!=qpeAppMap()->end(); ++it ) \
                list += it.key(); \
            return list; \
        } \
    }; \
    QTOPIA_EXPORT_PLUGIN(ApplicationImpl)

// The main function (dynamic/normal)
#define QTOPIA_MAIN_IMPL \
    int main( int argc, char **argv ) { \
        qLog(ApplicationLauncher) << "Starting main()"; \
        QTOPIA_SET_KEY(argv[0]) \
        QString executableName(argv[0]); \
        executableName = executableName.right(executableName.length() - executableName.lastIndexOf('/') - 1); \
        QtopiaApplication a( argc, argv ); \
        QTOPIA_SET_DOCUMENT_SYSTEM_CONNECTION(); \
        QWidget *mw = 0; \
        if ( qpeAppMap()->contains(executableName) ) \
            mw = (*qpeAppMap())[executableName](0,0); \
        else if ( qpeAppMap()->count() ) \
            mw = qpeAppMap()->begin().value()(0,0); \
        if ( mw ) { \
            int rv = 0; \
            a.setMainWidget(mw); \
            if ( mw->metaObject()->indexOfSlot("setDocument(QString)") != -1 ) { \
                a.showMainDocumentWidget(); \
            } else { \
                a.showMainWidget(); \
            } \
            qLog(ApplicationLauncher) << "Entering event loop"; \
            rv = a.exec(); \
            qLog(ApplicationLauncher) << "Exited event loop"; \
            delete mw; \
            qLog(ApplicationLauncher) << "Exiting main"; \
            return rv; \
        } \
        return -1; \
    }

/*
    Definitions of the primary macros.
*/
#if defined(SINGLE_EXEC)

// The QTOPIA_MAIN macro is useless in single-exec mode
#define QTOPIA_MAIN

#if defined(SINGLE_EXEC_USE_MAIN)

// singleexec/manual
#define QTOPIA_ADD_APPLICATION(NAME,IMPLEMENTATION) \
    QTOPIA_REGISTER_SINGLE_EXEC_MAIN(NAME,IMPLEMENTATION)

#endif

#endif // SINGLE_EXEC

// */quicklaunch, dynamic/normal
#ifndef QTOPIA_ADD_APPLICATION
#define QTOPIA_ADD_APPLICATION(NAME,IMPLEMENTATION) \
    QTOPIA_REGISTER_WIDGET(NAME,IMPLEMENTATION)
#endif

#ifndef QTOPIA_MAIN

#ifdef QTOPIA_APP_INTERFACE

// dynamic/quicklaunch
#define QTOPIA_MAIN \
    QTOPIA_SETUP_APP_MAP \
    QTOPIA_QUICKLAUNCH_IMPL

#else // QTOPIA_APP_INTERFACE

// dynamic/normal
#define QTOPIA_MAIN \
    QTOPIA_SETUP_APP_MAP \
    QTOPIA_MAIN_IMPL

#endif // QTOPIA_APP_INTERFACE

#endif // QTOPIA_MAIN

#endif
