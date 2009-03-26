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

#include <qprinterinterface.h>

/*!
    \class QtopiaPrinterInterface
    \inpublicgroup QtBluetoothModule

    \brief The QtopiaPrinterInterface class provides the minimum interface to various printers.
    \ingroup multimedia

    The embedded systems usually have restricted printing capability.
    The QtopiaPrinterInterface class declares the minimum set of functions
    that has to be implemented by printer plug-ins to provide printing
    capabilities within Qtopia.

    \sa QtopiaPrinterPlugin
 */


/*!
    Destroys the printer object.
 */
QtopiaPrinterInterface::~QtopiaPrinterInterface()
{
}

/*!
    \fn void QtopiaPrinterInterface::print( QMap<QString, QVariant> options )

    Sends printer \a options data to a printer that can be used to configure the printer.
    Values are consist of a QString form of QPrintEngine::PrintEnginePropertyKey and its value.
*/

/*!
    \fn void QtopiaPrinterInterface::printFile( const QString &fileName, const QString &mimeType = QString() )
    Prints \a fileName. \a mimeType is optional.
*/

/*!
    \fn void QtopiaPrinterInterface::printHtml( const QString &html )
    Prints \a html.
*/

/*!
    \fn bool QtopiaPrinterInterface::abort()
    Attempts to stop the printing. Returns true if printing is successfully interrupted; otherwise returns false.
*/

/*!
    \fn QString QtopiaPrinterInterface::name()
    Returns a name of the printing mechanism.
*/

/*!
    \fn bool QtopiaPrinterInterface::isAvailable()
    Returns true if the printing is currently supported.
*/


/*!
    \class QtopiaPrinterPlugin
    \inpublicgroup QtBluetoothModule

    \brief The QtopiaPrinterPlugin class provides an abstract base for QtopiaPrinterInterface plug-ins.

    \section1 Creating a New Plug-in

    To create a new printer plug-in, create a new project directory, for example, \c newprinter,
    under \c $QPEDIR/src/plugins/qtopiapringing/.

    Create a new printing mechanism by sub-classing QtopiaPrinterPlugin
    and reimplementing the pure virtual functions. The new plug-in must be exported.
    For example:

    \code
        class QTOPIA_PLUGIN_EXPORT NewPrinterPlugin : public QtopiaPrinterPlugin
        {
            Q_OBJECT
        public:
            NewPrinterPlugin( QObject *parent = 0 );
            ~NewPrinterPlugin();

            virtual void print( QMap<QString, QVariant> options );
            ...
        };
    \endcode

    \section1 Loading the Plug-in

    Use QPluginManager to load plug-ins from an application.
    For example:

    \code
        QPluginManager *manager = new QPluginManager( "qtopiaprinting" );

        QObject *instance;
        QtopiaPrinterInterface *iface;

        instance = manager->instance( "newprinter" );
        iface = qobject_cast<QtopiaPrinterPlugin *>(instance);

        if ( iface ) {
            if ( iface->isAvailable() )
                iface->printFile( fileName );
        }
    \endcode

    Alternatively, PrintService can be used.

    \code
        QtopiaServiceRequest service( "Print", "print(QString)" );
        service << fileName;
        service.send();
    \endcode

    There is a Qt Extended Print Server that handles this request.
    The Print Server will present the user with a list of available plug-ins to choose from.
    It also loads the selected plug-in and dispatches the print job.

    \ingroup multimedia
    \ingroup plugins

    \sa QtopiaPrinterInterface, QPluginManager, PrintService

*/

/*!
    Constructs a printer plug-in with the given \a parent.
 */
QtopiaPrinterPlugin::QtopiaPrinterPlugin( QObject *parent )
    :QObject( parent )
{
}

/*!
    Destroys the printer plug-in.
 */
QtopiaPrinterPlugin::~QtopiaPrinterPlugin()
{
}

