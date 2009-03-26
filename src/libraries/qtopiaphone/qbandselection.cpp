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

#include <qbandselection.h>

/*!
    \class QBandSelection
    \inpublicgroup QtTelephonyModule

    \brief The QBandSelection class provides an interface to select a GSM band for the phone to operate on.

    Bands are identified by string identifiers such as the following:

    \table
    \row \o \c{GSM 850} \o GSM at 850 MHz
    \row \o \c{GSM 900} \o GSM at 900 MHz
    \row \o \c{GSM 1800} \o DCS at 1800 MHz
    \row \o \c{GSM 1900} \o PCS at 1900 MHz
    \row \o \c{Dual 900/1800} \o Dual 900 MHz and 1800 MHz bands
    \row \o \c{Dual 850/1900} \o Dual 950 MHz and 1900 MHz bands
    \endtable

    Telephony services should report dual bands separately from the
    single constituent bands, to allow the user to select either a
    specific band, or one of a set of supported bands.

    Not all modems support band selection.  The available() method should
    be used by client applications to determine if band selection is possible.

    \sa QCommInterface
    \ingroup telephony
*/

/*!
    Construct a new band selection object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports band selection.  If there is more
    than one service that supports band selection, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QBandSelection objects for each.

    \sa QCommServiceManager::supports()
*/
QBandSelection::QBandSelection
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QBandSelection", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this band selection object.
*/
QBandSelection::~QBandSelection()
{
}

/*!
    \enum QBandSelection::BandMode
    This enum defines the mode of band selection to be used.

    \value Automatic Use an explicit band selection if possible, but fall
                     back to any available band otherwise.
    \value Manual Use only the explicitly specified band.
*/

/*!
    Request the currently selected band.  The result is reported via the
    band() signal.

    \sa band(), requestBands()
*/
void QBandSelection::requestBand()
{
    invoke( SLOT(requestBand()) );
}

/*!
    Request a list of all bands that can be selected from.  The result is
    reported via the bands() signal.

    \sa bands(), requestBand()
*/
void QBandSelection::requestBands()
{
    invoke( SLOT(requestBands()) );
}

/*!
    Sets the current band \a mode and \a value.  If \a value is null,
    and \a mode is \c Automatic, then any available band will be used.
    The result of setting the band is reported via the setBandResult() signal.

    \sa setBandResult(), requestBand()
*/
void QBandSelection::setBand
        ( QBandSelection::BandMode mode, const QString& value )
{
    invoke( SLOT(setBand(QBandSelection::BandMode,QString)),
            qVariantFromValue( mode ), value );
}

/*!
    \fn void QBandSelection::band( QBandSelection::BandMode mode, const QString& value )

    Signal that is emitted in response to requestBand(), to report the
    currently selected \a mode and band \a value.

    \sa requestBand()
*/

/*!
    \fn void QBandSelection::bands( const QStringList& list )

    Signal that is emitted to report the supported \a list of bands.

    \sa requestBands()
*/

/*!
    \fn void QBandSelection::setBandResult( QTelephony::Result result )

    Signal that is emitted to report the \a result of calling setBand().

    \sa setBand()
*/

Q_IMPLEMENT_USER_METATYPE_ENUM(QBandSelection::BandMode)
