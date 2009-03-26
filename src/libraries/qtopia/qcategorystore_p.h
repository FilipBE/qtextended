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
#ifndef QCATEGORYSTORE_P_H
#define QCATEGORYSTORE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QSharedData>
#include <QIcon>
#include <QStringList>
#include <QFlags>
#include <QMap>
#include <qtopiaipcmarshal.h>

class QCategoryDataPrivate;

class QCategoryData
{
public:
    enum Flag
    {
        IconLoaded = 0x01,
        Global     = 0x02,
        System     = 0x04
    };

    Q_DECLARE_FLAGS(Flags,Flag);

    QCategoryData();
    QCategoryData( const QString &label, const QString &iconFile, const QString &ringTone, Flags flags );
    QCategoryData( const QCategoryData &other );

    ~QCategoryData();

    QCategoryData &operator =( const QCategoryData &other );

    bool operator ==( const QCategoryData &other ) const;

    const QString &label() const;
    const QString &iconFile() const;
    const QString &ringTone() const;

    const QIcon &icon();

    bool isGlobal() const;
    bool isSystem() const;

    bool isNull() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    void loadIcon();

    QSharedDataPointer< QCategoryDataPrivate > d;
};

typedef QMap< QString, QCategoryData > QCategoryDataMap;

Q_DECLARE_OPERATORS_FOR_FLAGS(QCategoryData::Flags);

Q_DECLARE_METATYPE(QCategoryDataMap);
Q_DECLARE_USER_METATYPE(QCategoryData);
Q_DECLARE_USER_METATYPE_ENUM(QCategoryData::Flags);
Q_DECLARE_USER_METATYPE_TYPEDEF(QCategoryDataMap,QCategoryDataMap);


class QCategoryStore : public QObject
{
    Q_OBJECT
public:
    QCategoryStore( QObject *parent = 0 );
    virtual ~QCategoryStore();

    virtual bool addCategory( const QString &categoryId, const QString &scope, const QString &label, const QString &icon, bool isSystem ) = 0;
    virtual bool categoryExists( const QString &categoryId ) = 0;
    virtual QCategoryData categoryFromId( const QString &categoryId ) = 0;
    virtual QCategoryDataMap scopeCategories( const QString &scope ) = 0;
    virtual bool removeCategory( const QString &categoryId ) = 0;
    virtual bool setCategoryScope( const QString &categoryId, const QString &scope ) = 0;
    virtual bool setCategoryIcon( const QString &categoryId, const QString &icon ) = 0;
    virtual bool setCategoryRingTone( const QString &categoryId, const QString &icon ) = 0;
    virtual bool setCategoryLabel( const QString &categoryId, const QString &label ) = 0;
    virtual bool setSystemCategory( const QString &categoryId ) = 0;

    static QCategoryStore *instance();

    QString errorString() const;

signals:
    void categoriesChanged();

protected:
    void setErrorString( const QString &errorString );

private:
    QString m_errorString;
};

#endif
