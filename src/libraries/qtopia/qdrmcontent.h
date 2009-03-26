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

#ifndef QDRMCONTENT_H
#define QDRMCONTENT_H

#include <QObject>
#include <QDateTime>
#include <qtimestring.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>
#include <qcontent.h>

class QDrmContentPrivate;

class QTOPIA_EXPORT QDrmContent : public QObject
{
    Q_OBJECT
public:

    enum RenderState{ Started, Stopped, Paused };

    enum LicenseOption
    {
        NoLicenseOptions = 0x00,
        Activate         = 0x01,
        Reactivate       = 0x02,
        Handover         = 0x04,
        Default          = Activate | Reactivate,
    };

    Q_DECLARE_FLAGS( LicenseOptions, LicenseOption );

    explicit QDrmContent( QDrmRights::Permission permission = QDrmRights::Unrestricted,
                          LicenseOptions options = Default, QObject *parent = 0 );

    virtual ~QDrmContent();

    RenderState renderState() const;

    QDrmRights::Permission permission() const;

    void setPermission( QDrmRights::Permission permission );

    LicenseOptions licenseOptions() const;

    void setLicenseOptions( LicenseOptions options );
    void enableLicenseOptions( LicenseOptions options );
    void disableLicenseOptions( LicenseOptions options );

    QWidget *focusWidget() const;
    void setFocusWidget( QWidget *focus );

    QContent content() const;

    static bool activate( const QContent &content, QWidget *focus = 0 );
    static bool canActivate( const QContent &content );

    static QStringList supportedTypes();
    static QList< QPair< QString, QString > > httpHeaders();

public slots:
    bool requestLicense( const QContent &content );
    void releaseLicense();

    void renderStarted();
    void renderStopped();
    void renderPaused();

signals:
    void licenseGranted( const QContent &content );
    void licenseDenied( const QContent &content );
    void renderStateChanged( const QDrmContent &content );
    void rightsExpired( const QDrmContent &content );

private:
    void init( QDrmRights::Permission permission, LicenseOptions options );
    bool getLicense( const QContent &content, QDrmRights::Permission );

    QDrmContentPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QDrmContent::LicenseOptions );

#endif
