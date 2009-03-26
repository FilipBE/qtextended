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

#ifndef QTOPIASERVERAPPLICATION_H
#define QTOPIASERVERAPPLICATION_H

#include <qtopiaapplication.h>
#include <QList>
class QWSEvent;
class QValueSpaceObject;

class QtopiaServerApplication : public QtopiaApplication
{
Q_OBJECT
Q_ENUMS(ShutdownType)
public:
    enum ShutdownType { NoShutdown, ShutdownSystem, RebootSystem,
                        RestartDesktop, TerminateDesktop };
    enum StartupType  { ImmediateStartup, IdleStartup };

    // Construction
    QtopiaServerApplication(int& argc, char **argv);
    ~QtopiaServerApplication();
    static QtopiaServerApplication *instance();

#ifdef Q_WS_QWS
    // Event filtering
    class QWSEventFilter {
    public:
        virtual ~QWSEventFilter() {}

        virtual bool qwsEventFilter( QWSEvent * ) = 0;
    };
    void installQWSEventFilter(QWSEventFilter *);
    void removeQWSEventFilter(QWSEventFilter *);
    bool notify( QObject*, QEvent* );
#endif

    // Tasks
    static bool startup(int &argc, char **argv, const QList<QByteArray> &startupGroups, QtopiaServerApplication::StartupType type = QtopiaServerApplication::ImmediateStartup);
    static QString taskConfigFile();

    static QObject *qtopiaTask(const QByteArray &taskName,
                               bool onlyRunning = false);
    static void excludeFromTaskCleanup(QObject* task, bool exclude);
    static void addAggregateObject(QObject *me, QObject *them);

    static QByteArray taskValueSpaceObject(const QByteArray &taskName);
    static bool taskValueSpaceSetAttribute(const QByteArray &taskName,
                                           const QByteArray &attribute,
                                           const QVariant &value);
    static int &argc();
    static char **argv();
    static ShutdownType shutdownType();

    // Tasks - private
    typedef QObject *(*CreateTaskFunc)(void *);
    static void addTask(const char *, bool, CreateTaskFunc, void *);
    static void addTaskProvide(const char *, const char *);
    static QObject *_qtopiaTask(const char *, bool);
    static QList<QObject *> _qtopiaTasks(const char *, bool);
    static void addTaskInterface(const char *, QObject *);

signals:
    void shutdownRequested();

public slots:
    void shutdown( QtopiaServerApplication::ShutdownType t );

protected:
    virtual void shutdown();
    virtual void restart();

#ifdef Q_WS_QWS
    bool qwsEventFilter( QWSEvent * );
#endif

private slots:
    void serverWidgetVsChanged();

private:
    static void _shutdown(ShutdownType);
#ifdef Q_WS_QWS
    QList<QWSEventFilter *> m_filters;
#endif
    static QtopiaServerApplication *m_instance;
    QByteArray mainWidgetName;
    QValueSpaceObject* serverWidget_vso;
    static void loadTaskPlugins();
};

template<class T>
inline const char *qtopiaTask_InterfaceName()
{
    return 0;
}

template<class T>
inline QList<T *> qtopiaTasks(bool onlyActive = false)
{
    QList<T *> rv;
    QList<QObject *> obj =
        QtopiaServerApplication::_qtopiaTasks( qtopiaTask_InterfaceName<T>(),
                                        onlyActive );

    for(int ii = 0; ii < obj.count(); ++ii) {
        T * t = qobject_cast<T *>(obj.at(ii));
        if(t) rv.append(t);
    }
    return rv;
}

template<class T>
inline T * qtopiaTask(bool onlyActive = false)
{
    return qobject_cast<T *>(QtopiaServerApplication::_qtopiaTask( qtopiaTask_InterfaceName<T>(), onlyActive));
}

template<class T>
inline void qtopiaProvidesInterface(QObject *me)
{
    QtopiaServerApplication::addTaskInterface(qtopiaTask_InterfaceName<T>(), me);
}

#define QTOPIA_TASK_QINTERFACE(IFace) \
    template <> inline const char *qtopiaTask_InterfaceName<IFace>() \
    { return # IFace; }

#define QTOPIA_TASK_INTERFACE(IFace) \
    template <> inline IFace *qobject_cast<IFace *>(QObject *object) \
    { return reinterpret_cast<IFace *>((object ? object->qt_metacast(# IFace) : 0)); } \
    template <> inline IFace *qobject_cast<IFace *>(const QObject *object) \
    { return reinterpret_cast<IFace *>((object ? const_cast<QObject *>(object)->qt_metacast(# IFace) : 0)); } \
    QTOPIA_TASK_QINTERFACE(IFace)

#define QTOPIA_TASK_PROVIDES(name, interface) \
    struct _task_install_provides_ ## name ## _ ## interface { \
        _task_install_provides_ ## name ## _ ## interface() { \
            QtopiaServerApplication::addTaskProvide(# name, qtopiaTask_InterfaceName<interface>()); \
        } \
    }; \
    static _task_install_provides_ ## name ## _ ## interface _task_install_provides_instance_ ## name ## _ ## interface;

#define QTOPIA_STATIC_TASK(name, function) \
    static QObject *_task_install_create_ ## name(void *) { \
        function;\
        return 0; \
    } \
    struct _task_install_ ## name { \
        _task_install_ ## name() { \
            QtopiaServerApplication::addTask(# name, false, \
                                             _task_install_create_ ## name, 0); \
        } \
    }; \
    static _task_install_ ## name _task_install_instance_ ## name;

#define QTOPIA_TASK(name, function) \
    static QObject *_task_install_create_ ## name(void *) { \
        return new function; \
    } \
    struct _task_install_ ## name { \
        _task_install_ ## name() { \
            QtopiaServerApplication::addTask(# name, false, \
                                             _task_install_create_ ## name, 0); \
        } \
    }; \
    static _task_install_ ## name _task_install_instance_ ## name;

#define QTOPIA_DEMAND_TASK(name, function) \
    static QObject *_task_install_create_ ## name(void *) { \
        return new function; \
    } \
    struct _task_install_ ## name { \
        _task_install_ ## name() { \
            QtopiaServerApplication::addTask(# name, true, \
                                             _task_install_create_ ## name, 0); \
        } \
    }; \
    static _task_install_ ## name _task_install_instance_ ## name;


class _ReplacementInstaller
{
public:
    typedef QWidget *(*CreateFunc)(QWidget *parent, Qt::WFlags f);

    _ReplacementInstaller(const QMetaObject *you,
                          const QMetaObject *them,
                          CreateFunc create);
    _ReplacementInstaller(const QMetaObject *you,
                          const QMetaObject *them,
                          const char *feature,
                          CreateFunc create);
    static QWidget *widget(const QMetaObject *them,
                           QWidget *parent,
                           Qt::WFlags flags);
};

#define QTOPIA_REPLACE_WIDGET(OldClass, NewClass) \
    QWidget *_replacement_create_ ## NewClass(QWidget *parent, Qt::WFlags f) {\
        return new NewClass(parent, f); \
    } \
    _ReplacementInstaller _replacement_install_ ## NewClass (&NewClass::staticMetaObject, &OldClass::staticMetaObject, _replacement_create_ ## NewClass);

#define QTOPIA_REPLACE_WIDGET_OVERRIDE(OldClass, NewClass) \
    template<> OldClass *qtopiaWidget<OldClass>(QWidget *parent, \
                                                Qt::WFlags flags) \
    { \
        return new NewClass(parent, flags);\
    } \
    template<> OldClass *qtopiaWidget<OldClass>(QWidget *parent) \
    { \
        return new NewClass(parent, 0);\
    } \
    template<> OldClass *qtopiaWidget<OldClass>() \
    { \
        return new NewClass(0, 0);\
    }

#define QTOPIA_REPLACE_WIDGET_WHEN(OldClass, NewClass, Feature) \
    QWidget *_replacement_create_ ## NewClass(QWidget *parent, Qt::WFlags f) {\
        return new NewClass(parent, f); \
    } \
    _ReplacementInstaller _replacement_install_ ## NewClass (&NewClass::staticMetaObject, &OldClass::staticMetaObject, # Feature, _replacement_create_ ## NewClass);

template<class T>
T *qtopiaWidget(QWidget *parent, Qt::WFlags flags)
{
    return static_cast<T *>(_ReplacementInstaller::widget(&T::staticMetaObject,
                                                          parent, flags));
}
template<class T>
T *qtopiaWidget(QWidget *parent)
{
    return static_cast<T *>(_ReplacementInstaller::widget(&T::staticMetaObject,
                                                          parent, 0));
}
template<class T>
T *qtopiaWidget()
{
    return static_cast<T *>(_ReplacementInstaller::widget(&T::staticMetaObject,
                                                          0, 0));
}

class SystemShutdownHandler : public QObject
{
Q_OBJECT
public:
    virtual bool systemRestart();
    virtual bool systemShutdown();

    static inline int timeout()
    { return 5000; }

signals:
    void proceed();
};
QTOPIA_TASK_INTERFACE(SystemShutdownHandler);

#endif
