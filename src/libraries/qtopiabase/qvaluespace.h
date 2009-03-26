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

#ifndef QVALUESPACE_H
#define QVALUESPACE_H

#include <QVariant>
#include <QByteArray>
#include <QList>
#include <QObject>
#include <QUuid>

#include "qtopiailglobal.h"

class QTOPIAIL_EXPORT IValueSpaceLayer : public QObject
{
Q_OBJECT
public:
#ifdef QT_ARCH_X86_64
    typedef unsigned long HANDLE;
    static const HANDLE InvalidHandle = 0xFFFFFFFFFFFFFFFF;
#else
    typedef unsigned int HANDLE;
    static const HANDLE InvalidHandle = 0xFFFFFFFF;
#endif

    /* Returns the layer name */
    virtual QString name() = 0;

    enum Type { Server, Client };
    /* Called by the value space to initialize the layer */
    virtual bool startup(Type) = 0;
    /* Called by the value space to re-initialize the layer */
    virtual bool restart() = 0;
    /* Called by the value space server to shutdown the layer */
    virtual void shutdown() = 0;

    virtual QUuid id() = 0;
    virtual unsigned int order() = 0;

    /* Returns the current value of \a handle */
    virtual bool value(HANDLE, QVariant *) = 0;
    /* Returns the current value of a subpath of handle. subPath must start
       with '/' */
    virtual bool value(HANDLE, const QByteArray &, QVariant *) = 0;

    /* Removes all keys under handle */
    virtual bool remove(HANDLE) = 0;
    /* Removes all keys under a subpath or handle */
    virtual bool remove(HANDLE, const QByteArray &) = 0;
    /* Sets the current value of \a handle, if possible */
    virtual bool setValue(HANDLE, const QVariant &) = 0;
    /* Sets the current value of a subpath of \a handle, if possible.  subPath
       must start with '/' */
    virtual bool setValue(HANDLE, const QByteArray &, const QVariant &) = 0;
    /* Commit any changes (if needed) made through setValue() now */
    virtual bool syncChanges() = 0;

    /* Returns the list of immediate children, or an empty set if no children */
    virtual QSet<QByteArray> children(HANDLE) = 0;
    /* Returns an item handle.  Use of an invalid parent is allowed.  Returning
       an InvalidHandle means that you will never, ever expose that key or any
       sub key. Path must start with '/'*/
    virtual HANDLE item(HANDLE parent, const QByteArray &) = 0;

    enum Properties { Publish = 0x00000001 };
    /* Set a property on a handle. */
    virtual void setProperty(HANDLE handle, Properties) = 0;

    /* Removes a previously allocated handle. */
    virtual void remHandle(HANDLE) = 0;

signals:
    void handleChanged(unsigned int);
};

// syncqtopia header QValueSpace
namespace QValueSpace {
    QTOPIAIL_EXPORT void initValuespaceManager();
    void initValuespace();
    QTOPIAIL_EXPORT void reinitValuespace();

    typedef IValueSpaceLayer *(*LayerCreateFunc)();
    void installLayer(LayerCreateFunc func);
    void installLayer(IValueSpaceLayer * layer);

    struct AutoInstall {
        AutoInstall(LayerCreateFunc func) { installLayer(func); }
    };
};

#define QVALUESPACE_AUTO_INSTALL_LAYER(name) \
    IValueSpaceLayer * _qvaluespaceauto_layercreate_ ## name() \
    { \
        return name::instance(); \
    } \
    static QValueSpace::AutoInstall _qvaluespaceauto_ ## name(_qvaluespaceauto_layercreate_ ## name);

class QValueSpaceItemPrivate;
class QValueSpaceSubItemIterator;
class QTOPIAIL_EXPORT QValueSpaceItem : public QObject
{
Q_OBJECT
public:
    QValueSpaceItem(const QValueSpaceItem &base, const QByteArray &path, QObject* parent = 0);
    QValueSpaceItem(const QValueSpaceItem &base, const QString &path, QObject* parent = 0);
    QValueSpaceItem(const QValueSpaceItem &base, const char * path, QObject* parent = 0);
    QValueSpaceItem(const QValueSpaceItem &other, QObject* parent = 0 );
    explicit QValueSpaceItem(const QByteArray &path, QObject* parent = 0);
    explicit QValueSpaceItem(const QString &path, QObject* parent = 0);
    explicit QValueSpaceItem(const char *path, QObject* parent = 0);

    explicit QValueSpaceItem(QObject* parent = 0);
    virtual ~QValueSpaceItem();

    QString itemName() const;

    QList<QString> subPaths() const;

    void remove();
    void remove(const QByteArray &subPath);
    void remove(const char *subPath);
    void remove(const QString &subPath);
    void setValue(const QVariant &value);
    void setValue(const QByteArray & subPath,
                  const QVariant &value);
    void setValue(const char * subPath,
                  const QVariant &value);
    void setValue(const QString & subPath,
                  const QVariant &value);
    bool sync();

    QVariant value(const QByteArray & subPath = QByteArray(),
                   const QVariant &def = QVariant()) const;
    QVariant value(const char * subPath,
                   const QVariant &def = QVariant()) const;
    QVariant value(const QString & subPath,
                   const QVariant &def = QVariant()) const;

    QValueSpaceItem &operator=(const QValueSpaceItem&);
signals:
    void contentsChanged();

protected:
    virtual void connectNotify(const char *);
    virtual void disconnectNotify(const char *);

private:
    QValueSpaceItemPrivate * d;
    friend class QValueSpaceSubItemIterator;
    friend class QValueSpaceSubItemIteratorPrivate;
};

class QValueSpaceObjectPrivate;
class QTOPIAIL_EXPORT QValueSpaceObject : public QObject
{
Q_OBJECT
public:
    explicit QValueSpaceObject(const char *objectPath, QObject *parent = 0);
    explicit QValueSpaceObject(const QString &objectPath, QObject *parent = 0);
    explicit QValueSpaceObject(const QByteArray &objectPath, QObject *parent = 0);
    ~QValueSpaceObject();

    QString objectPath() const;
    static void sync();

signals:
    void itemRemove(const QByteArray &attribute);
    void itemSetValue(const QByteArray &attribute, const QVariant &value);

public slots:
    void setAttribute(const char *attribute, const QVariant &data);
    void removeAttribute(const char *attribute);
    void setAttribute(const QString &attribute, const QVariant &data);
    void removeAttribute(const QString &attribute);
    void setAttribute(const QByteArray &attribute, const QVariant &data);
    void removeAttribute(const QByteArray &attribute);

protected:
    virtual void connectNotify(const char *);

private:
    friend class ApplicationLayer;
    Q_DISABLE_COPY(QValueSpaceObject)
    QValueSpaceObjectPrivate * d;
};

#endif // _QVALUESPACE_H_
