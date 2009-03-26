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

#ifndef QDSDATA_P_H
#define QDSDATA_P_H

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

// Qtopia includes
#include <QUniqueId>

// Qt includes
#include <QFile>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QByteArray;
class QDataStream;
class QMimeType;
class QBuffer;

// ============================================================================
//
//  QDSDataThrehold
//
// ============================================================================

class QDSDataThreshold
{
public:
    QDSDataThreshold();

    int mValue;
};

// ============================================================================
//
//  QDSLockedFile
//
// ============================================================================

class QTOPIA_AUTOTEST_EXPORT QDSLockedFile : public QFile
{
public:
    QDSLockedFile();
    explicit QDSLockedFile( const QString& name );
    ~QDSLockedFile();

    enum LockMode { ReadLock, WriteLock, NoLock };

    bool openLocked ( QIODevice::OpenMode mode, bool acquireLock = true );
    void close();

    bool isLocked() const;
    bool lock( LockMode mode, bool block = true );
    LockMode lockMode() const;
    bool unlock();

    bool remove();

    static bool remove( const QString& name );

private:
    LockMode mLock;
};

// ============================================================================
//
//  QDSDataPrivate
//
// ============================================================================

class QDSDataPrivate
{
public:
    QDSDataPrivate();
    explicit QDSDataPrivate( const QUniqueId& dataId );
    QDSDataPrivate( const QByteArray& data, const QMimeType& type );
    QDSDataPrivate( QFile& data, const QMimeType& type );
    QDSDataPrivate( const QUniqueId& dataId,
                    const QByteArray& data,
                    const QMimeType& type );
    ~QDSDataPrivate();

    void shiftToStore();

    bool            mUsingLocalStore;
    QUniqueId  mId;
    QByteArray*     mLocalStore;
    QMimeType*      mType;

    //static QDSDataThreshold   mThreshold;
    //static QUniqueIdGenerator mIdGen;
};

// ============================================================================
//
//  QDSDataStore
//
// ============================================================================

class QDSDataStore
{
public:
    static bool add( const QUniqueId& id,
                     const QByteArray& data,
                     const QMimeType& type );

    static bool add( const QUniqueId& id,
                     QFile& data,
                     const QMimeType& type );

    static bool add( const QUniqueId& id );

    static bool set( const QUniqueId& id,
                     const QByteArray& data );

    static bool set( const QUniqueId& id,
                     QFile& data );

    static bool set( const QUniqueId& id,
                     const QByteArray& data,
                     const QMimeType& type );

    static bool set( const QUniqueId& id,
                     QFile& data,
                     const QMimeType& type );

    static bool remove( const QUniqueId& id );
    static bool exists( const QUniqueId& id );

    static bool transmit( const QUniqueId& id );
    static bool received( const QUniqueId& id );

    static QMimeType type( const QUniqueId& id );
    static QByteArray data( const QUniqueId& id );

    static QString infoFileName( const QUniqueId& id );
    static QString dataFileName( const QUniqueId& id );

    static int referenceCount( const QUniqueId& id );
    static bool removeReference( const QUniqueId& id );

    static int transmitCount( const QUniqueId& id );
    static bool removeTransmit( const QUniqueId& id );
};

#endif
