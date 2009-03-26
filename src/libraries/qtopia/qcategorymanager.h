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

#ifndef QCATEGORYMANAGER_H
#define QCATEGORYMANAGER_H

#include <qtopiaglobal.h>
#include <QStringList>
#include <QString>
#include <QIcon>
#include <QList>
#include <QSharedData>

class QSettings;
class QCategoryFilterData;
class QTOPIA_EXPORT QCategoryFilter
{
public:
    enum FilterType {
        List,
        Unfiled,
        All
    };
    QCategoryFilter();
    QCategoryFilter(FilterType t);
    explicit QCategoryFilter(const QList<QString>&);
    explicit QCategoryFilter(const QString &);
    QCategoryFilter(const QCategoryFilter &other);
    ~QCategoryFilter();

    QCategoryFilter &operator=(const QCategoryFilter &other);

    bool operator==(const QCategoryFilter &o) const;
    bool operator!=(const QCategoryFilter &o) const;

    bool accepted(const QList<QString> &) const;
    bool acceptAll() const;
    bool acceptUnfiledOnly() const;

    // might be a bad choice.
    QList<QString> requiredCategories() const;

    QString label(const QString &scope = QString()) const;

    void writeConfig( QSettings &, const QString &key) const;
    void readConfig( const QSettings &, const QString &key);

private:
    QSharedDataPointer<QCategoryFilterData> d;
};

class QCategoryManagerData;
class QTOPIA_EXPORT QCategoryManager : public QObject
{
    Q_OBJECT
public:
    explicit QCategoryManager(QObject *parent = 0);
    explicit QCategoryManager(const QString &scope, QObject *parent = 0);
    ~QCategoryManager();


    QString label(const QString &) const;
    QList<QString> labels(const QList<QString> &) const;
    QIcon icon(const QString &) const;
    QString iconFile(const QString &) const;
    QString ringTone(const QString &) const;

    static QString unfiledLabel();
    static QString allLabel();
    static QString multiLabel();

    bool isSystem(const QString &id) const;
    bool setSystem(const QString &id);

    bool isGlobal(const QString &id) const;
    bool setGlobal(const QString &id, bool);

    QString add( const QString &trLabel, const QString &icon=QString(), bool forceGlobal=false );
    bool addCategory( const QString &id, const QString &trLabel, const QString &icon=QString(), bool forceGlobal=false, bool isSystem=false );
    bool ensureSystemCategory( const QString &id, const QString &trLabel, const QString &icon=QString(), bool forceGlobal=false );

    bool remove( const QString &id );
    bool setLabel( const QString &id, const QString &trLabel );
    bool setIcon( const QString &id, const QString &icon );
    bool setRingTone( const QString &id, const QString &fileName );

    bool contains(const QString &id) const;
    bool containsLabel(const QString &trLabel, bool forceGlobal=false) const;
    QString idForLabel( const QString &trLabel ) const;
    QList<QString> categoryIds() const;

    bool exists( const QString &id ) const;

signals:
    void categoriesChanged();

private slots:
    void reloadCategories();

private:
    QCategoryManagerData *d;
};

#endif
