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

#ifndef QCONTENTENGINE_P_H
#define QCONTENTENGINE_P_H

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

#include <QStringList>
#include <QAbstractFileEngine>
#include <QIcon>
#include <QDrmContent>
#include <QSharedData>
#include <QMimeType>

class QDrmContentLicense;
class QAbstractFileEngine;
class QContentEnginePrivate;
class QContentStore;

class QContentEngineData
{
public:
    QContentEngineData()
        : id( QContent::InvalidId )
        , role( QContent::UnknownUsage )
        , drmState( QContent::Unprotected )
    {
    }

    virtual ~QContentEngineData(){}

    QContentId id;
    QString engineType;
    QString name;
    QString fileName;
    QMimeType mimeType;
    QString media;
    QString iconName;
    QContent::Role role;
    QContent::DrmState drmState;
    QDateTime lastUpdated;
};

struct QMimeEngineData
{
    QString type;
    QString icon;
    QString application;
    QDrmRights::Permission permission;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);
};

Q_DECLARE_USER_METATYPE(QMimeEngineData);
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QMimeEngineData>);

QTOPIA_EXPORT QDataStream &operator <<( QDataStream &stream, const QMimeEngineData &engine );
QTOPIA_EXPORT QDataStream &operator >>( QDataStream &stream, QMimeEngineData &engine );

class QContentEngine : public QSharedData
{
public:
    virtual ~QContentEngine();

    QContentId id() const{ return d_ptr->id; }

    const QString &engineType() const{ return d_ptr->engineType; }

    const QStringList &categories() const;
    const QString &fileName() const{ return d_ptr->fileName; }
    const QIcon &icon() const;
    const QString &iconName() const{ return d_ptr->iconName; }
    const QString &media() const;
    const QString &name() const{ return d_ptr->name; }
    QContent::Role role() const{ return d_ptr->role; }
    const QMimeType &mimeType() const{ return d_ptr->mimeType; }
    const QDateTime &lastUpdated() const{ return d_ptr->lastUpdated; }

    const QString &translatedName() const;

    qint64 size( bool force ) const;

    void setCategories( const QStringList &categories );
    void setFileName( const QString &fileName );
    void setIconName( const QString &iconName );
    void setMedia( const QString &media );
    void setName( const QString &name );
    void setRole( QContent::Role role );
    void setMimeType( const QMimeType &mimeType );
    void setMimeTypes( const QStringList &mimeTypes, const QStringList& mimeTypeIcons, const QList< QDrmRights::Permission > permissions);

    QString property( const QString &group, const QString &key ) const;
    void setProperty( const QString &group, const QString &key, const QString &value );

    QStringList propertyGroups() const;
    QStringList propertyKeys( const QString &group ) const;

    bool isValid( bool force ) const;

    QContent::DrmState drmState() const{ return d_ptr->drmState; }
    QDrmRights::Permissions permissions( bool force ) const;

    virtual QDrmRights rights( QDrmRights::Permission ) const;

    const QList< QMimeEngineData > &executableMimeTypes() const;

    virtual QContentEngine *copyTo( const QString &newPath ) = 0;
    virtual bool moveTo( const QString &newPath ) = 0;
    virtual bool rename(const QString &name) = 0;

    virtual bool execute( const QStringList &arguments ) const = 0;

    virtual bool canActivate() const;

    virtual bool activate( QDrmRights::Permission permission, QWidget *parent );

    virtual bool reactivate( QDrmRights::Permission permission, QWidget *parent );

    virtual QDrmContentLicense *requestLicense( QDrmRights::Permission permission, QDrmContent::LicenseOptions options );

    virtual bool remove();

    virtual QIODevice *open( QIODevice::OpenMode mode ) = 0;

    virtual bool isOutOfDate() const = 0;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    const QString &errorString() const;

    enum Attribute
    {
        Categories          = 0x001,
        FileName            = 0x002,
        Icon                = 0x004,
        Media               = 0x008,
        Name                = 0x010,
        Role                = 0x020,
        MimeType            = 0x040,
        Properties          = 0x080,
        Permissions         = 0x100,
        Size                = 0x200,
        ExecutableMimeTypes = 0x400,
        TranslatedName      = 0x800,
        Validity            = 0x1000
    };

    virtual QContentEngine *createCopy() const = 0;

    void copy( const QContentEngine &other );

    static QContentEngine *createEngine( const QString &fileName );
    static QContentEngine *createEngine( const QContent &content );

    Q_DECLARE_FLAGS(Attributes,Attribute);
protected:

    QContentEngine( const QString &engineType );
    QContentEngine( QContentEnginePrivate &dd, const QString &engineType );

    void setId( QContentId id );
    void setDrmState( QContent::DrmState state );
    void setLastUpdated( const QDateTime &date );

    void loadProperties();
    void loadCategories();
    void loadIcon();
    void loadExecutableMimeTypes();
    void loadTranslatedName();

    Attributes dirtyAttributes() const;

    void clearDirtyAttributes();

    QContentEngineData *d_ptr;

    virtual QDrmRights::Permissions queryPermissions();
    virtual qint64 querySize() = 0;
    virtual bool queryValidity() = 0;

    void setError( const QString &errorString );

private:
    Q_DECLARE_PRIVATE(QContentEngine)

    friend class QContentStore;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QContentEngine::Attributes);

Q_DECLARE_USER_METATYPE_ENUM(QContentEngine::Attributes);

QTOPIA_EXPORT QDebug operator <<( QDebug debug, const QContentEngine &engine );

QTOPIA_EXPORT QDataStream &operator <<( QDataStream &stream, const QContentEngine &engine );
QTOPIA_EXPORT QDataStream &operator >>( QDataStream &stream, QContentEngine &engine );

#if defined(QTOPIA_DBUS_IPC) && !defined(QTOPIA_HOST)
QTOPIA_EXPORT QDBusArgument &operator <<( QDBusArgument &stream, const QContentEngine &engine );
QTOPIA_EXPORT const QDBusArgument &operator >>( const QDBusArgument &stream, QContentEngine &engine );
#endif

class QContentEngineHandler
{
    public:
        QContentEngineHandler();
        virtual ~QContentEngineHandler();

        virtual QContentEngine *create( const QString &fileName ) const = 0;
        virtual QContentEngine *create( const QContent &content ) const = 0;
};

typedef QMap< QString, QString > QContentEngineGroupCache;
typedef QMap< QString, QContentEngineGroupCache > QContentEnginePropertyCache;

Q_DECLARE_METATYPE(QContentEngineGroupCache);
Q_DECLARE_METATYPE(QContentEnginePropertyCache);
Q_DECLARE_USER_METATYPE_TYPEDEF(QContentEnginePropertyCache,QContentEnginePropertyCache);

class QContentEnginePrivate : public QContentEngineData
{
public:
    QContentEnginePrivate()
        : loadedAttributes( 0 )
        , size( 0 )
        , error( false )
        , isValid( false )
    {
    }

    virtual ~QContentEnginePrivate()
    {
    }

    QContentEngine::Attributes loadedAttributes;
    QContentEngine::Attributes dirtyAttributes;

    qint64 size;

    QString translatedName;
    QIcon icon;
    QStringList categories;
    QContentEnginePropertyCache propertyCache;
    bool error;
    QString errorString;
    bool isValid;

    QDrmRights::Permissions permissions;

    QList<QMimeEngineData> mimeEngineData;
};

#endif
