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

#include "atfrontend.h"
#include "atoptions.h"
#include <qatresult.h>
#include <qtopialog.h>
#include <qbytearray.h>
#include <qtimer.h>

/*
    \class AtFrontEnd
    \inpublicgroup QtTelephonyModule
    \brief Wraps a serial port to process incoming AT commands
*/

/*
    \enum AtFrontEnd::State
    Describes the current interface state.

    \value Offline The interface is offline and in command mode.
    \value OnlineData The interface is online and in data mode.
    \value OnlineCommand The interface is online and in command mode.
*/

class AtFrontEndPrivate
{
public:
    AtFrontEndPrivate( const QString& startupOptions )
    {
        canMux = true;
        device = 0;
        dataSource = 0;
        databuf = 0;
        options = new AtOptions( startupOptions );
        reset();
    }
    ~AtFrontEndPrivate()
    {
        delete options;
        if ( databuf )
            delete databuf;
    }

    bool canMux;
    QSerialIODevice *device;
    AtFrontEnd::State state;
    bool requestExtra;
    bool extraDcdOff;
    AtOptions *options;
    QByteArray buffer;
    QStringList lastCommand;
    QIODevice* dataSource;  //used for data calls
    char *databuf;

    void reset()
    {
        state = AtFrontEnd::Offline;
        requestExtra = false;
        extraDcdOff = false;
        buffer.clear();
        lastCommand.clear();
        if ( databuf ) {
            delete databuf;
            databuf = 0;
        }
    }
};

#define DATACALL_BUFSIZ 1024

AtFrontEnd::AtFrontEnd( const QString& startupOptions, QObject *parent )
    : QObject( parent )
{
    d = new AtFrontEndPrivate( startupOptions );
}

AtFrontEnd::~AtFrontEnd()
{
    delete d;
}

QSerialIODevice *AtFrontEnd::device() const
{
    return d->device;
}

void AtFrontEnd::setDevice( QSerialIODevice *device )
{
    if ( d->device != device ) {
        if ( d->device ) {
            // Disconnect the signals on the previous device we were using.
            disconnect( d->device, SIGNAL(readyRead()),
                        this, SLOT(readyRead()) );
            disconnect( d->device, SIGNAL(dsrChanged(bool)),
                        this, SLOT(dsrChanged(bool)) );
        }
        d->device = device;
        d->reset();
        if ( d->device ) {
            // Connect up the signals on the new device.
            connect( d->device, SIGNAL(readyRead()),
                     this, SLOT(readyRead()) );
            connect( d->device, SIGNAL(dsrChanged(bool)),
                     this, SLOT(dsrChanged(bool)) );

            // Turn off DCD, because we aren't in an online data state yet.
            d->device->setCarrier( false );

            // Make sure that DTR is turned on, because the previous statement
            // may have turned it off if DCD and DTR are joined together.
            d->device->setDtr( true );
        }
    }
}

void AtFrontEnd::setDataSource( QIODevice* sourceDevice )
{
    if ( sourceDevice == d->dataSource )
        return;

    if ( d->dataSource ) {
        setState( AtFrontEnd::Offline );
        //the associated PhoneCall will delete the old dataSource
        if ( d->databuf ) {
            delete d->databuf;
            d->databuf = 0;
        }
    }
    d->dataSource = sourceDevice;
}

/*
    Get the current interface state.
*/
AtFrontEnd::State AtFrontEnd::state() const
{
    return d->state;
}

/*
    Get the options block that this interface is using.
*/
AtOptions *AtFrontEnd::options() const
{
    return d->options;
}

/*
    Determine if this front end can support GSM 07.10 multiplexing.
*/
bool AtFrontEnd::canMux() const
{
    return d->canMux;
}

/*
    Set the multiplexing support flag to \a value.
*/
void AtFrontEnd::setCanMux( bool value )
{
    d->canMux = value;
}

/*
    Set the current interface state to \a value.
*/
void AtFrontEnd::setState( AtFrontEnd::State value )
{
    // Bail out if there is no useful change of state.
    if ( !d->device || d->state == value )
        return;

    // Modify the DCD signal to reflect the new state.
    if ( value == AtFrontEnd::Offline ) {

        // Transitioning from OnlineCommand/OnlineData to Offline.  Lower DCD.
        if ( !d->device->setCarrier( false ) ) {

            // DCD was lowered but it also caused DTR to lower.
            // Wait for a short delay and then raise DTR again.
            QTimer::singleShot( 100, this, SLOT(raiseDtr()) );

        }

    } else if ( d->state == AtFrontEnd::Offline ) {
        // Transitioning from Offline to OnlineCommand/OnlineData.  Raise DCD.
        if ( d->dataSource && !d->databuf ) {
            // make sure dataSource data are written to client device
            connect( d->dataSource, SIGNAL(readyRead()),
                    this, SLOT(dataSourceReadyRead()) );
            connect( d->dataSource, SIGNAL(closed()),
                    this, SLOT(dataSourceClosed()) );
            d->databuf = new char [DATACALL_BUFSIZ];
        }
        d->device->setCarrier( true );
    }

    // Update the state.
    d->state = value;
}

/*
    Send \c{&gt;} and a space to the peer machine and request an extra
    line of data for the current AT command.  This is used for GSM commands
    such as \c{AT+CMGS} and \c{AT+CMGW} that require extra data.
*/
void AtFrontEnd::requestExtra()
{
    if ( d->device ) {
        qLog(ModemEmulator) << "> [requesting extra line]";
        if ( ! d->options->echo ) {
            // If echo wasn't on, then we haven't sent a CRLF to
            // the peer machine yet.  Do it now because the full
            // sequence that the peer is looking for is "CRLF> ".
            char buf[4];
            buf[0] = d->options->terminator;
            buf[1] = d->options->response;
            buf[2] = '>';
            buf[3] = ' ';
            d->device->write( buf, 4 );
        } else {
            d->device->write( "> ", 2 );
        }
        d->requestExtra = true;

        // Turn on the DCD signal while we wait for the extra line.
        // This is required by GSM 27.005, section 3.5.1, AT+CMGS.
        d->extraDcdOff = d->device->setCarrier( true );
    }
}

/*
    Send \a line to the peer machine, following by a line termination sequence.
*/
void AtFrontEnd::send( const QString& line )
{
    qLog(ModemEmulator) << line;
    if ( d->device ) {
        QByteArray l;
        if ( ! d->options->echo ) {
            l += d->options->terminator;
            l += d->options->response;
        }
        l += line.toLatin1();
        l += d->options->terminator;
        l += d->options->response;
        d->device->write( l.data(), l.size() );
    }
}

/*
    Send the contents of \a result to the peer machine, formatting
    the result codes according to the prevailing error reporting options.
*/
void AtFrontEnd::send( QAtResult::ResultCode result )
{
    // Bail out if result codes are suppressed.
    if ( d->options->suppressResults )
        return;

    // Determine how to format the result line.
    int code = (int)result;
    if ( code >= 0 ) {

        // Send an extended error result code.
        if ( d->options->extendedErrors == 1 ) {

            // Send the extended code in numeric form.
            if ( code >= 300 && code <= 500 )
                send( "+CMS ERROR: " + QString::number( code ) );
            else
                send( "+CME ERROR: " + QString::number( code ) );

        } else if ( d->options->extendedErrors == 2 ) {

            // Send the extended code in verbose text form.
            QAtResult verbose;
            verbose.setResultCode( result );
            send( verbose.verboseResult() );

        } else {

            // Send the extended code as just "ERROR".
            send( "ERROR" );

        }

    } else if ( ! d->options->verboseResults ) {

        // Send a normal result code in numeric form.
        send( QString::number( -code ) );

    } else {

        // Send a normal result code in text form.
        QAtResult normal;
        normal.setResultCode( result );
        send( normal.result() );

    }
}

/*
    Stop the last command from being repeated with "A/".
*/
void AtFrontEnd::stopRepeatLast()
{
    d->lastCommand.clear();
}

/*
    \fn void AtFrontEnd::commands( const QStringList& cmds )

    Signal that is emitted when a list of AT commands, \a cmds, is recognized.
    The list contains pairs of strings, for the command name and parameters.
    The command name will be normalized to its upper-case form.
*/

/*
    \fn void AtFrontEnd::extra( const QString& line, bool cancel )

    Signal that is emitted when an extra \a line of data is recognized
    in response to requestExtra().  If \a cancel is true, it indicates
    that the line ended with ESC rather than CTRL-Z.
*/

/*
    \fn void AtFrontEnd::remoteHangup()

    Signal that is emitted when the remote side lowers DTR to indicate hangup.
    The interface will be placed into the Offline state before this signal
    is emitted.
*/

/*
    \fn void AtFrontEnd::enterCommandState()

    Signal that is emitted when the interface enters the OnlineCommand
    state due to the reception of a delay, \c{+++}, and another delay.
*/

// Raise DTR after it was lowered due to a side-effect of lowering DCD.
void AtFrontEnd::raiseDtr()
{
    if ( d->device)
        d->device->setDtr( true );
}

void AtFrontEnd::writeCRLF()
{
    char buf[2];
    buf[0] = d->options->terminator;
    buf[1] = d->options->response;
    d->device->write( buf, 2 );
}

void AtFrontEnd::writeBackspace()
{
    char buf[3];
    buf[0] = d->options->backspace;
    buf[1] = ' ';
    buf[2] = d->options->backspace;
    d->device->write( buf, 3 );
}

// Process incoming data.
void AtFrontEnd::readyRead()
{
    char ch;
    QString line;

    while ( d->device ) {

        // Are we now in the online data state?
        if ( d->state == AtFrontEnd::OnlineData ) {
            if ( d->dataSource ) {
                // pass the data through to the raw channel
                // TODO check for +++ within the data stream.
                qint64 len = 0;
                while ( (len = d->device->read(d->databuf, DATACALL_BUFSIZ)) > 0 ) {
                    d->dataSource->write( d->databuf, len );
                }
                if ( !d->device->isOpen() ) {
                    d->device->deleteLater();
                    d->device = 0;
                }
                break;
            } else {
                // Dummy: just eat the characters
                d->device->getChar( &ch );
                continue;
            }
        }

        // Get the next character and process it.
        int result = d->device->read( &ch, 1 );
        if ( result < 0 )
            break;
        else if ( !result ) {
            if ( !d->device->isOpen() ) {
                d->device->deleteLater();
                d->device = 0;
            }
            break;
        }
        if ( !ch )
            break;      // Ignore NUL bytes in the input stream.
        if ( ( ch == d->options->terminator || ch == d->options->response ) && !d->requestExtra ) {

            // Terminate the current line and pass it up the stack.
            line = QString::fromLatin1( d->buffer.data(), d->buffer.size() );
            d->buffer.clear();
            if ( d->options->echo )
                writeCRLF();
            parseCommandLine( line );

        } else if ( d->requestExtra && ch == 0x1A || ch == 0x1B ) {

            // Terminate the current line and pass it up as extra data.
            line = QString::fromLatin1( d->buffer.data(), d->buffer.size() );
            d->buffer.clear();
            if ( d->options->echo )
                writeCRLF();
            d->requestExtra = false;
            if ( d->extraDcdOff && d->state == AtFrontEnd::Offline )
                d->device->setCarrier( false );
            emit extra( line, ( ch == 0x1B ) );

        } else if ( ch == d->options->backspace || ch == 0x7F ) {

            // Backspace over the last character.
            if ( d->buffer.size() > 0 ) {
                if ( d->options->echo )
                    writeBackspace();
                d->buffer.resize( d->buffer.size() - 1 );
            }

        } else if ( ch == '/' && d->buffer.size() == 1 &&
                    ( d->buffer[0] == 'a' || d->buffer[0] == 'A' ) &&
                    !( d->requestExtra ) ) {

            // We have encountered the "A/" repeat sequence.
            d->device->putChar( ch );
            d->buffer.clear();
            if ( d->options->echo )
                writeCRLF();
            if ( d->lastCommand.size() > 0 ) {
                emit commands( d->lastCommand );
            }

        } else if ( ch != d->options->response && ch != 0x00 ) {

            // Add this character to the current buffer and echo it.
            d->buffer += ch;
            if ( d->options->echo ) {
                d->device->putChar( ch );
                if ( ch == d->options->terminator )
                    d->device->putChar( d->options->response );
            }

        }

    }
}

void AtFrontEnd::dsrChanged( bool value )
{
    if ( !value && d->state != AtFrontEnd::Offline ) {
        // The remote side has hung up.  Exit the online state and report.
        setState( AtFrontEnd::Offline );
        emit remoteHangup();

    }
}

// Convert to upper case, but only for ASCII letters.  Needed to
// prevent the locale from interfering with V.250 compatibility.
static unsigned int convertToUpperAscii( unsigned int ch )
{
    if ( ch >= 'a' && ch <= 'z' )
        return ch - 'a' + 'A';
    else
        return ch;
}

void AtFrontEnd::parseCommandLine( const QString& line )
{
    unsigned int ch;
    unsigned int ch2;
    int posn, start;
    bool amp, withinString;

    // Search for the first occurrence of "AT" on the line.
    // Everything before that is considered line noise and ignored.
    posn = 0;
    while ( ( posn + 1 ) < line.length() ) {
        if ( ( line[posn] == QChar('a') ||
               line[posn] == QChar('A') ) &&
             ( line[posn + 1] == QChar('t') ||
               line[posn + 1] == QChar('T') ) ) {
            break;
        }
        ++posn;
    }
    if ( ( posn + 1 ) >= line.length() )
        return;     // No "AT prefix on the line, so ignore it.
    posn += 2;

    // Squash spaces out of the line: they aren't important except in strings.
    // Also remove the "AT" prefix as we don't need it any more.
    QString nline;
    if ( line.indexOf( QChar(' '), posn ) != -1 ||
         line.indexOf( QChar('\t'), posn ) != -1 ) {
        withinString = false;
        while ( posn < line.length() ) {
            ch = line[posn++].unicode();
            if ( !withinString && ( ch == ' ' || ch == '\t' ) )
                continue;
            if ( ch == QChar('"') )
                withinString = !withinString;
            nline += QChar(ch);
        }
    } else {
        nline = line.mid( posn );
    }

    // Split the line up into individual commands.  The resulting string list
    // will contain pairs of (command, parameters).
    QStringList cmds;
    posn = 0;
    amp = false;
    while ( posn < nline.length() ) {
        ch = nline[posn].unicode();
        if ( ch == ';' ) {
            // Skip separators between extension commands.
            ++posn;
        } else if ( ch == 's' || ch == 'S' ) {
            // Special V.250 setting command.
            ++posn;
            start = posn;
            int sep = -1;
            while ( posn < nline.length() ) {
                ch2 = nline[posn].unicode();
                if ( ( ch2 >= '0' && ch2 <= '9' ) ) {
                    ++posn;
                } else if ( ch2 == '=' || ch2 == '?' ) {
                    if ( sep == -1 )
                        sep = posn;
                    else {
                        // Tried to use more than one '=' or '?'.
                        send( "ERROR" );
                        return;
                    }
                    ++posn;
                } else {
                    break;
                }
            }
            if ( sep == -1 ) {
                // Something like "ATSN".
                cmds += "S" + nline.mid( start, posn - start );
                cmds += QString();
            } else {
                // Something like "ATSN=M" or "ATSN?".
                cmds += "S" + nline.mid( start, sep - start );
                cmds += nline.mid( sep, posn - sep );
            }
        } else if ( ( ch == 'd' || ch == 'D' ) && !amp ) {
            // Special V.250 dial command: rest of line is the dial string.
            cmds += QString("D");
            cmds += nline.mid( posn + 1 );
            break;
        } else if ( ( ch >= 'A' && ch <= 'Z' ) || ( ch >= 'a' && ch <= 'z' ) ) {
            // Regular V.250 command: may be followed by a number.
            ++posn;
            start = posn;
            while ( posn < nline.length() ) {
                ch2 = nline[posn].unicode();
                if ( ch2 < '0' || ch2 > '9' )
                    break;
                ++posn;
            }
            ch = convertToUpperAscii( ch );
            if ( amp )
                cmds += QString( QChar('&') ) + QString( QChar(ch) );
            else
                cmds += QString( QChar(ch) );
            cmds += nline.mid( start, posn - start );
            amp = false;
        } else if ( ch == '&' ) {
            // V.250 prefixed command such as AT&F or AT&W.
            // Set a flag and then come back in via the case above.
            amp = true;
            ++posn;
        } else if ( ch == '+' || ch == '*' || ch == '%' ) {
            // Extension command from V.250.  Technically, only commands
            // that start with '+' are V.250 extension commands, but there
            // are common modems in the field that also use '*' and '%'.
            static char const nameChars[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789!%-./:_";
            QString name = QString( QChar(ch) );
            ++posn;
            while ( posn < nline.length() ) {
                ch = nline[posn].unicode();
                if ( strchr( nameChars, (int)ch ) == 0 )
                    break;
                name += QChar( convertToUpperAscii( ch ) );
                ++posn;
            }
            withinString = false;
            start = posn;
            while ( posn < nline.length() ) {
                ch = nline[posn].unicode();
                if ( ch == ';' && !withinString )
                    break;
                if ( ch == '"' )
                    withinString = !withinString;
                ++posn;
            }
            cmds += name;
            cmds += nline.mid( start, posn - start );
        } else {
            // Syntax error at this point in the command.
            send( "ERROR" );
            return;
        }
    }

    // If there are no commands (i.e. just "AT"), then respond OK and bail out.
    if ( cmds.size() == 0 ) {
        send( "OK" );
        return;
    }

    // Save as the last command just in case we receive "A/" next.
    d->lastCommand = cmds;

    // Dispatch the commands that we recognized.
    emit commands( cmds );
}

void AtFrontEnd::dataSourceReadyRead()
{
    qint64 len = 0;
    while ( (len = d->dataSource->read(d->databuf, DATACALL_BUFSIZ)) > 0 ) {
        d->device->write( d->databuf, len );
    }
}

void AtFrontEnd::dataSourceClosed()
{
    setState( AtFrontEnd::Offline );
    //the associated PhoneCall will delete the dataSource
    if ( d->databuf ) {
        delete d->databuf;
        d->databuf = 0;
    }
    delete d->device;
    d->device = 0;
}
