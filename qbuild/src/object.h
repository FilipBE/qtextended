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

#ifndef OBJECT_H
#define OBJECT_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QHash>

struct TraceContext {
    QString fileName;
    int lineNumber;
};

class QMakeObject;
class QMakeObjectSubscription : public QObject
{
Q_OBJECT
public:
    QMakeObjectSubscription(QObject *parent = 0);

    void subscribe(QMakeObject *, const QString &);

signals:
    void valueChanged(const QString &name, const QStringList &added,
                      const QStringList &removed);

private:
    friend class QMakeObject;

    void objectChanged(QMakeObject *, const QStringList &added,
                       const QStringList &removed);
    void removeObject(QMakeObject *);
    typedef QList<QPair<QMakeObject *, QString> > Subscriptions;
    Subscriptions m_subs;
};

class Project;
class QMakeObjectType;
class QMakeObject
{
public:
    QMakeObject(const QMakeObject *parent, const QString & = QString());
    QMakeObject(const QString & = QString());
    virtual ~QMakeObject();

    QMakeObject *parent() const;
    QMakeObject *root() const;
    QString name() const;
    void setName(const QString &);
    QString absoluteName() const;
    QString rootPath() const;

    int valueCount() const;
    QStringList value() const;
    void setValue(const QStringList &, TraceContext * = 0);
    void addValue(const QStringList &, TraceContext * = 0);
    void subtractValue(const QStringList &, TraceContext * = 0);
    void uniteValue(const QStringList &, TraceContext * = 0);

    bool isProperty(const QString &) const;
    bool isProperty(int) const;
    QStringList properties() const;
    QMakeObject *property(const QString &) const;
    QMakeObject *property(int) const;

    QList<QMakeObject *> find(const QString &, const QString &, const QString & = QString()) ;

    virtual void dump();

    static QMakeObject *object(const QString &path, Project *ctxt);

    static void addNewTypeFunction(const QString &, const QString &);
    static void addDelTypeFunction(const QString &, const QString &);
    static void removeNewTypeFunction(const QString &, const QString &);
    static void removeDelTypeFunction(const QString &, const QString &);

    bool readOnly() const;
    void setReadOnly();

    void clear(TraceContext * = 0);

    struct TraceInfo {
        QString fileName;
        int lineNo;
        QStringList value;
        const char *op;
    };
    QList<TraceInfo> traceInfo() const;
    void watch( const QString &function );

protected:
    void dump(int);

private:
    void clearInternal(TraceContext * = 0);
    QMakeObject(const QMakeObject &);
    QMakeObject &operator=(const QMakeObject &);

    // Debugging stuff
    void objectTrace(const QStringList &, TraceContext *, const char *);

    // Identity
    QString m_path;
    QMakeObject *m_parent;
    bool m_isType;

    // Subscription stuff
    friend class QMakeObjectSubscription;
    void addSubscription(QMakeObjectSubscription *);
    void remSubscription(QMakeObjectSubscription *);
    void doSubscriptions(const QStringList &added, const QStringList &removed);
    QList<QMakeObjectSubscription *> m_subscriptions;

    // Value and properties
    QStringList m_value;
    mutable QList<QMakeObject *> m_indexObjects;
    mutable QHash<QString, QMakeObject *> m_properties;
    mutable QList<QMakeObject *> m_orderedProperties;

    // Type functions
    typedef QHash<QString, QStringList> TypeFunctions;
    static TypeFunctions m_newTypeFunctions;
    static TypeFunctions m_delTypeFunctions;
    void newType(const QString &);
    void delType(const QString &);

    bool m_readOnly;

    QList<TraceInfo> m_traceInfo;
    QMakeObjectSubscription *m_watched_sub;
    QList<QString> m_watched;
};

#endif
