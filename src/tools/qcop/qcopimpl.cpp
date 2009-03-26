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

#include "qcopimpl.h"
#include <qtopiaservices.h>
#include <quuid.h>
#include <quniqueid.h>
#include <QContent>

#include <QDateTime>
#include <QApplication>

void doqcopusage()
{
    fprintf( stderr, "Usage: qcop [-l username] command [args] [-- command2 [args2] ...]\n" );
    fprintf( stderr, "\n" );
    fprintf( stderr, "send channel message [parameters]\n" );
    fprintf( stderr, "    Send a QCop \"message\" to \"channel\".\n" );
    fprintf( stderr, "watch channel [channel ...]\n" );
    fprintf( stderr, "    Watch all messages on \"channel\".\n" );
    fprintf( stderr, "wait channel message\n" );
    fprintf( stderr, "    Wait for \"message\" to appear on \"channel\" and print its parameters.\n" );
    fprintf( stderr, "query channel\n" );
    fprintf( stderr, "    Query the supported messages on \"channel\".\n" );
    fprintf( stderr, "list\n" );
    fprintf( stderr, "    List all available services and their application mappings.\n" );
    fprintf( stderr, "\n" );
    fprintf( stderr, "The following modifiers can appear before a command:\n");
    fprintf( stderr, "\n" );
    fprintf( stderr, "service\n" );
    fprintf( stderr, "    Interpret channel names as service names.\n" );
    fprintf( stderr, "hex\n" );
    fprintf( stderr, "    Interpret QByteArray values as hexadecimal (default).\n" );
    fprintf( stderr, "nohex\n" );
    fprintf( stderr, "    Interpret QByteArray values as Latin-1 strings.\n" );
    fprintf( stderr, "timeout msecs\n" );
    fprintf( stderr, "    Time out the next command after \"msecs\" milliseconds.\n" );
    fprintf( stderr, "notimeout\n" );
    fprintf( stderr, "    Don't time out the next command.\n" );
    fprintf( stderr, "enum name\n" );
    fprintf( stderr, "    Recognise \"name\" as an enumerated type.\n" );
    exit(1);
}

void doqcopsyntax( const QString &where, const QString &what )
{
    fprintf( stderr, "Syntax error in %s: %s\n", where.toLatin1().data(), what.toLatin1().data() );
    exit(1);
}

void disableqdebug( QtMsgType type, const char *msg )
{
    // Ignore messages that are sent via qDebug.
   Q_UNUSED( type );
   Q_UNUSED( msg );
}

static char const hexchars[] = "0123456789ABCDEF";

QString toHex( const QByteArray& binary )
{
    QString str = "";

    for ( int i = 0; i < binary.size(); i++ ) {
        str += (QChar)(hexchars[ (binary[i] >> 4) & 0x0F ]);
        str += (QChar)(hexchars[ binary[i] & 0x0F ]);
    }

    return str;
}

QByteArray fromHex( const QString& hex )
{
    QByteArray bytes;
    uint ch;
    int posn;
    int nibble, value, flag, size;

    flag = 0;
    value = 0;
    size = 0;
    for ( posn = 0; posn < hex.length(); ++posn ) {
        ch = (uint)( hex[posn].unicode() );
        if ( ch >= '0' && ch <= '9' ) {
            nibble = ch - '0';
        } else if ( ch >= 'A' && ch <= 'F' ) {
            nibble = ch - 'A' + 10;
        } else if ( ch >= 'a' && ch <= 'f' ) {
            nibble = ch - 'a' + 10;
        } else {
            continue;
        }
        value = (value << 4) | nibble;
        flag = !flag;
        if ( !flag ) {
            bytes.resize( size + 1 );
            bytes[size++] = (char)value;
            value = 0;
        }
    }

    return bytes;
}

QString quoteString( const QString& str )
{
    // Build the quoted result.
    QString result = "\"";
    int posn = 0;
    uint ch;
    while ( posn < str.length() ) {
        ch = str[posn++].unicode();
        if ( ch == '"' || ch == '\\' || ch == '\r' || ch == '\n' ) {
            result += (QChar)'\\';
            result += (QChar)(hexchars[(ch >> 4) & 0x0F]);
            result += (QChar)(hexchars[ch & 0x0F]);
        } else {
            result += (QChar)ch;
        }
    }
    result += (QChar)'"';
    return result;
}

static QStringList enumTypes;

void convertFromString( QDataStream& stream, const QString& type,
                        const QString& value, bool hex )
{
    if ( type == "int" ) {
        stream << value.toInt();
    } else if ( type == "uint" ) {
        stream << value.toUInt();
    } else if ( type == "long" ) {
        if ( sizeof(long) == sizeof(int) )
            stream << value.toInt();
        else
            stream << value.toLongLong();
    } else if ( type == "ulong" ) {
        if ( sizeof(long) == sizeof(int) )
            stream << value.toUInt();
        else
            stream << value.toULongLong();
    } else if ( type == "qlonglong" ) {
        stream << value.toLongLong();
    } else if ( type == "qulonglong" ) {
        stream << value.toULongLong();
    } else if ( type == "bool" ) {
        if ( value.toLower() == "true" )
            stream << (int)1;
        else if ( value.toLower() == "false" )
            stream << (int)0;
        else
            doqcopsyntax( "parameter value for bool should be either 'true' or 'false'", value );
    } else if ( type == "QString" ) {
        stream << value;
    } else if ( type == "QByteArray" ) {
        if ( hex )
            stream << fromHex( value );
        else
            stream << value.toLatin1();
    } else if ( type == "QDateTime" ) {
        stream << QDateTime::fromString( QString(value), Qt::ISODate );
    } else if ( type == "QStringList" ) {
        QStringList list;
        if ( ! value.isEmpty() )
            list = value.split(",");
        stream << list;
    } else if ( type == "QUuid" ) {
        QUuid uuid( value );
        stream << uuid;
    } else if ( type == "QUniqueId" ) {
        QUniqueId uuid( value );
        stream << uuid;
    } else if ( enumTypes.contains( type ) ) {
        stream << value.toInt();
    } else if ( type == "QContentIdList" ) {
        QContentIdList ids;
        if (!value.isEmpty()) {
            foreach (QString pair, value.split(",")) {
                QStringList p = pair.split(":");
                QContentId id(p.at(0).toUInt(), p.at(1).toULongLong());
                ids << id;
            }
        }
        stream << ids;
    } else {
        doqcopsyntax( "parameter type", type );
    }
}

static bool convertError;

QString convertToString
        ( QDataStream& stream, const QString& type, bool hex, bool quote,
          bool reportErrors = false )
{
    if ( type == "int" ) {
        int v;
        stream >> v;
        return QString::number( v );
    } else if ( type == "uint" ) {
        uint v;
        stream >> v;
        return QString::number( v );
    } else if ( type == "long" ) {
        if ( sizeof(long) == sizeof(int) ) {
            int v;
            stream >> v;
            return QString::number( v );
        } else {
            qlonglong v;
            stream >> v;
            return QString::number( v );
        }
    } else if ( type == "ulong" ) {
        if ( sizeof(long) == sizeof(int) ) {
            uint v;
            stream >> v;
            return QString::number( v );
        } else {
            qulonglong v;
            stream >> v;
            return QString::number( v );
        }
    } else if ( type == "qlonglong" ) {
        qlonglong v;
        stream >> v;
        return QString::number( v );
    } else if ( type == "qulonglong" ) {
        qulonglong v;
        stream >> v;
        return QString::number( v );
    } else if ( type == "bool" ) {
        int v;
        stream >> v;
        if ( v )
            return "true";
        else
            return "false";
    } else if ( type == "QString" ) {
        QString v;
        stream >> v;
        if ( quote )
            return quoteString( v );
        else
            return v;
    } else if ( type == "QByteArray" ) {
        QByteArray v;
        stream >> v;
        QString result;
        if ( hex )
            result = toHex( v );
        else
            result = QString( v );
        if ( quote )
            return quoteString( result );
        else
            return result;
    } else if ( type == "QDateTime" ) {
        QDateTime v;
        stream >> v;
        QString result = v.toUTC().toString(Qt::ISODate);
        if ( quote )
            return quoteString( result );
        else
            return result;
    } else if ( type == "QStringList" ) {
        QStringList list;
        stream >> list;
        if ( quote ) {
            QStringList newlist;
            QStringList::Iterator it;
            for ( it = list.begin(); it != list.end(); ++it ) {
                newlist += quoteString( *it );
            }
            return "[" + newlist.join(",") + "]";
        } else {
            return list.join(",");
        }
    } else if ( type == "QUuid" ) {
        QUuid uuid;
        stream >> uuid;
        return uuid.toString();
    } else if ( type == "QUniqueId" ) {
        QUniqueId uuid;
        stream >> uuid;
        return uuid.toString();
    } else if ( type.startsWith( "QList<" ) && type.endsWith( ">" ) ) {
        // Read the contents of a list and convert each element into a string.
        QString innerType = type.mid( 6, type.length() - 7 );
        QStringList list;
        quint32 size;
        stream >> size;
        while ( size > 0 ) {
            list += convertToString( stream, innerType, hex, quote );
            --size;
        }
        return "[" + list.join(",") + "]";
    } else if ( enumTypes.contains( type ) ) {
        int v;
        stream >> v;
        return QString::number( v );
    } else if ( type == "QContentIdList" ) {
        QContentIdList ids;
        stream >> ids;
        QString s;
        foreach (QContentId id, ids) {
            s += QString::number(id.first) + ':' + QString::number(id.second) + ',';
        }
        s.chop(1);

        return s;
    } else {
        if ( reportErrors )
            doqcopsyntax( "parameter type", type );
        else
            convertError = true;
    }
    return QString();
}

struct ParamInfo
{
    QString name;
    QStringList parameters;
    QString message;
};

ParamInfo parseParameters( const QString& cmd )
{
    QByteArray norm = QMetaObject::normalizedSignature
        ( cmd.trimmed().toLatin1().constData() );
    QString command = QString( norm );
    ParamInfo info;

    int paren = command.indexOf( "(" );
    if ( paren <= 0 )
        doqcopsyntax( "command", command );

    QString params = command.mid( paren + 1 );
    if ( params[(int)params.length()-1] != ')' )
        doqcopsyntax( "command", command );
    params = params.left( params.length() - 1 );

    info.name = command.left( paren );
    if ( ! params.isEmpty() )
        info.parameters = params.split( "," );
    info.message = command;

    return info;
}

// Process the old (pre Qtopia 4) syntax for qcop.
void parseOldCommands( QApplication& app, int argc, char **argv )
{
    QString channel = argv[1];
    QString command = argv[2];

    ParamInfo info = parseParameters( command );

#ifdef HAVE_QCOP
    int argIdx = 3;
    {
        QCopEnvelope env( channel, info.message );
        if ( !info.parameters.isEmpty() ) {
            QStringList::Iterator it;
            for ( it = info.parameters.begin();
                  it != info.parameters.end(); ++it ) {
                QString arg = argv[argIdx];
                convertFromString( env, *it, arg, true );
                argIdx++;
            }
        }
        // send env
    }

    // Check for a "-w" option, which indicates that we should
    // wait for a QCop command before exiting.
    if ( argIdx < argc && QString(argv[argIdx]) == "-w" ) {
        if ( ( argIdx + 3 ) > argc ) {
            doqcopusage();
        }

        channel = argv[argIdx + 1];
        command = argv[argIdx + 2];
        info = parseParameters( command );

        int timeout = -1;
        if ( ( argIdx + 3 ) < argc ) {
            timeout = QString(argv[argIdx + 3]).toInt();
        }

        QCopWaiter *waiter = new QCopWaiter( channel, info.message, timeout );
        waiter->setExitOnTimeout();
        QObject::connect
            ( waiter, SIGNAL(done(bool)), &app, SLOT(quit()) );
        waiter->start( true );

        return;
    }

#endif

    QTimer::singleShot( 0, &app, SLOT(quit()) );
}

bool isNumber( const char *value )
{
    return ( value[0] >= '0' && value[0] <= '9' );
}

void addHandler( QCopHandler *& first, QCopHandler *& last,
                 QCopHandler *handler )
{
    if ( last ) {
        QObject::connect( last, SIGNAL(done(bool)),
                          handler, SLOT(start(bool)) );
    } else {
        first = handler;
    }
    last = handler;
}

// Parse a channel name from the command-line.
QString parseChannel( int& argc, char **& argv, bool serviceFlag,
                      bool /*watch*/, QString *serviceName = 0 )
{
    if ( argc <= 1 || QString( argv[1] ) == "--" )
        doqcopusage();
    QString channel = argv[1];
    --argc;
    ++argv;
    if ( serviceFlag ) {
        if ( serviceName )
            *serviceName = channel;
        channel = QtopiaService::channel( channel );
    }
    return channel;
}

// Parse a string argument.
QString parseString( int& argc, char **& argv )
{
    if ( argc <= 1 || QString( argv[1] ) == "--" )
        doqcopusage();
    QString value = argv[1];
    --argc;
    ++argv;
    return value;
}

// Check that we are positioned at the end of a command.
void checkEnd( int argc, char **argv )
{
    if ( argc > 1 && QString( argv[1] ) != "--" )
        doqcopusage();
}

// Determine if we are positioned at the end of a command.
bool atEnd( int argc, char **argv )
{
    if ( argc > 1 && QString( argv[1] ) != "--" )
        return false;
    else
        return true;
}

void parseCommands( QApplication& app, int argc, char **argv )
{
    if ( argc == 1 ) {
        doqcopusage();
    }
    bool hexFlag = true;
    bool serviceFlag = false;
    int timeout = -1;   // Use default timeout interval.
    QCopHandler *first = 0;
    QCopHandler *last = 0;
    QCopHandler *handler;
    QString channel, msg;
    QString service;
    while ( argc > 1 ) {
        QString cmd = argv[1];
        ++argv;
        --argc;
        if ( cmd == "help" ) {

            // Print the help message and exit.
            doqcopusage();

        } else if ( cmd == "--" ) {

            // End of one command and the start of another.  Reset all flags.
            hexFlag = true;
            serviceFlag = false;
            timeout = -1;

        } else if ( cmd == "send" ) {

            // Send a message to a specific channel.
            channel = parseChannel( argc, argv, serviceFlag, false, &service );
            msg = parseString( argc, argv );
            if ( serviceFlag ) {
                // Modify the message to include the service name prefix.
                if ( msg.startsWith( "::" ) )
                    msg = msg.mid(2);
                else if ( !msg.contains( "::" ) )
                    msg = service + "::" + msg;
            }
            ParamInfo info = parseParameters( msg );
            QCopEnvelope *env = new QCopEnvelope( channel, info.message );
            if ( !info.parameters.isEmpty() ) {
                QStringList::Iterator it;
                for ( it = info.parameters.begin();
                      it != info.parameters.end(); ++it ) {
                    QString arg = parseString( argc, argv );
                    convertFromString( *env, *it, arg, true );
                }
            }
            handler = new QCopSender( env, &app );
            addHandler( first, last, handler );
            checkEnd( argc, argv );

        } else if ( cmd == "wait" ) {

            // Wait for a specific message to arrive.
            channel = parseChannel( argc, argv, serviceFlag, true );
            msg = parseString( argc, argv );
            handler = new QCopWaiter( channel, msg, timeout, hexFlag, &app );
            addHandler( first, last, handler );
            checkEnd( argc, argv );

        } else if ( cmd == "watch" ) {

            // Put a watch on a specific channel.
            channel = parseChannel( argc, argv, serviceFlag, true );
            QCopWatcher *watcher =
                new QCopWatcher( channel, timeout, hexFlag, &app );
            addHandler( first, last, watcher );
            while ( ! atEnd( argc, argv ) ) {
                channel = parseChannel( argc, argv, serviceFlag, true );
                watcher->addExtraWatch( channel );
            }
            checkEnd( argc, argv );

        } else if ( cmd == "service" ) {

            // Treat the next channel name as a service name.
            serviceFlag = true;

        } else if ( cmd == "query" ) {

            // Query all messages that are supported by a channel.
            channel = parseChannel( argc, argv, serviceFlag, false );
            if ( serviceFlag ) {
                // Qualify the query with the service name, as there may
                // be more than one service implemented by the application.
                handler = new QCopQuery
                    ( channel, QString(argv[0]) + "::", timeout, &app );
            } else {
                handler = new QCopQuery( channel, "", timeout, &app );
            }
            addHandler( first, last, handler );
            checkEnd( argc, argv );

        } else if ( cmd == "list" ) {

            // List all available services and their application mappings.
            QStringList services = QtopiaService::list();
            QStringList::Iterator it;
            for ( it = services.begin(); it != services.end(); ++it ) {
                channel = QtopiaService::channel( *it );
                if ( channel.isEmpty() ) {
                    printf( "%s\tno mapping\n",
                            (*it).toLatin1().constData() );
                } else {
                    printf( "%s\t%s\n",
                            (*it).toLatin1().constData(),
                            channel.toLatin1().constData() );
                }
            }
            checkEnd( argc, argv );

        } else if ( cmd == "timeout" ) {

            // Set the timeout interval on the next command.
            if ( argc > 1 && isNumber(argv[1]) )
                timeout = QString(argv[1]).toInt();
            else
                doqcopusage();
            ++argv;
            --argc;

        } else if ( cmd == "notimeout" ) {

            // Force no timeout interval on the next command.
            timeout = -2;

        } else if ( cmd == "hex" ) {

            // Turn on hex dumping of QByteArray parameters.
            hexFlag = true;

        } else if ( cmd == "nohex" ) {

            // Turn off hex dumping of QByteArray parameters.
            hexFlag = false;

        } else if ( cmd == "enum" ) {

            // Mark certain type names as enumerated.
            enumTypes += parseString( argc, argv );
            checkEnd( argc, argv );

        } else if ( cmd.indexOf('/') != -1 ) {
            // Old-style usage: qcop [-l username] channel command
            //                  [parameters] [-w channel command [timeout]]
            parseOldCommands( app, argc + 1, argv - 1 );
            return;
        } else {
            doqcopusage();
        }
    }
    addHandler( first, last, new QCopEndHandler( &app ) );
    first->start( true );
}

int doqcopimpl (int argc, char *argv[])
{
    qInstallMsgHandler( disableqdebug );
    if ( argc > 1 ) {
        QString opt = argv[1];
        if ( opt == "-l" ) {
            if ( argc < 5 ) {
                doqcopusage();
                return 1;
            }
#ifndef Q_OS_WIN32
            const char *username = argv[2];
            struct passwd *pwd = getpwnam( username );
            if ( !pwd ) {
                fprintf( stderr, "Unknown user %s\n", username );
                exit(1);
            }
            int uid =  pwd->pw_uid;
            int gid =  pwd->pw_gid;
            if ( initgroups( username, gid ) != 0 ) {
                fprintf( stderr, "Could not chg group for user:%s\n", username );
                exit(1);
            }

            if ( setuid( uid ) != 0 ) {
                fprintf( stderr, "Could not run as user %s\n", username );
                exit(1);
            }
            setenv( "LOGNAME", username, 1 );
#else
            setenv("LOGNAME", argv[2], 1);
#endif

            argc -= 2;
            for ( int i = 1; i < argc; i++ ) {
                argv[i] = argv[i+2];
            }
        }

    }

    QApplication app( argc, argv, true );
    parseCommands( app, argc, argv );
    return app.exec();
}

QCopHandler::QCopHandler( QObject *parent )
    : QObject( parent )
{
}

QCopHandler::~QCopHandler()
{
}

void QCopHandler::start( bool ok )
{
    // Chain the failure through to the next handler.
    if ( !ok )
        emit done( ok );
}

QCopEndHandler::QCopEndHandler( QObject *parent )
    : QCopHandler( parent )
{
}

QCopEndHandler::~QCopEndHandler()
{
}

void QCopEndHandler::start( bool ok )
{
    // Must re-enter event loop one more time before stopping.
    if ( ok )
        QTimer::singleShot( 0, this, SLOT(succeed()) );
    else
        QTimer::singleShot( 0, this, SLOT(fail()) );
}

void QCopEndHandler::succeed()
{
    QApplication::exit( 0 );
}

void QCopEndHandler::fail()
{
    QApplication::exit( 1 );
}

QCopSender::QCopSender( QCopEnvelope *env, QObject *parent )
    : QCopHandler( parent )
{
    this->env = env;
}

QCopSender::~QCopSender()
{
}

void QCopSender::start( bool ok )
{
    QCopHandler::start( ok );
    if ( ok ) {
        // To send the message, all we have to do is delete the envelope.
        delete env;
        QTimer::singleShot( 0, this, SLOT(finish()) );
    }
}

void QCopSender::finish()
{
    emit done( true );
}

QCopWatcher::QCopWatcher( const QString& channel, int timeout, bool hexflag,
                          QObject *parent )
    : QCopHandler( parent )
{
    this->channels += channel;
    this->timeout = timeout;
    this->hexflag = hexflag;
}

QCopWatcher::~QCopWatcher()
{
}

void QCopWatcher::start( bool ok )
{
    QCopHandler::start( ok );
    if ( ok ) {
        QStringList::ConstIterator iter;
        for ( iter = channels.begin(); iter != channels.end(); ++iter ) {
            QCopChannel *chan = new QCopChannel( *iter, this );
            QObject::connect
                ( chan, SIGNAL(received(QString,QByteArray)),
                  this, SLOT(received(QString,QByteArray)) );
        }
        if ( timeout >= 0 ) {
            QTimer::singleShot( timeout, this, SLOT(gotTimeout()) );
        }
    }
}

void QCopWatcher::addExtraWatch( const QString& channel )
{
    channels += channel;
}

void QCopWatcher::received( const QString& msg, const QByteArray& data )
{
    // Handle messages on regular channels.
    if ( msg != "forwardedMessage(QString,QString,QByteArray)" ) {
        if ( channels.size() > 0 ) {
            // If we are watching more than one channel, display the names.
            QCopChannel *channel = (QCopChannel *)sender();
            printf( "%s: ", channel->channel().toLatin1().constData() );
        }
        printMessage( msg, data );
        return;
    }

    // Handle forwarded messages on wildcard channels.
    QDataStream stream( data );
    QString channel, message;
    QByteArray newData;
    stream >> channel;
    stream >> message;
    stream >> newData;
    printf( "%s: ", channel.toLatin1().constData() );
    printMessage( message, newData );
}

void QCopWatcher::printMessage( const QString& msg, const QByteArray& data )
{
    QDataStream stream( data );
    ParamInfo info = parseParameters( msg );

    if ( !info.parameters.isEmpty() && !msg.endsWith( "_fragment_" ) ) {
        printf( "%s( ", info.name.toLatin1().constData() );
        QStringList::Iterator it;
        bool comma = false;
        convertError = false;
        for ( it = info.parameters.begin();
              it != info.parameters.end(); ++it ) {
            if ( comma )
                printf( ", " );
            if ( convertError ) {
                printf( "%s", (*it).toLatin1().constData() );
            } else {
                QString value = convertToString
                    ( stream, *it, hexflag, true, false );
                if ( convertError ) {
                    printf( "? %s", (*it).toLatin1().constData() );
                } else {
                    printf( "%s", value.toLatin1().constData() );
                }
            }
            comma = true;
        }
        printf( " )\n" );
    } else {
        printf( "%s\n", msg.toLatin1().constData() );
    }
}

void QCopWatcher::gotTimeout()
{
    emit done( true );
    deleteLater();
}

QCopWaiter::QCopWaiter( const QString& channel, const QString& msg,
                        int timeout, bool hexflag, QObject *parent )
    : QCopHandler( parent )
{
    this->channel = channel;
    this->msg = msg;
    this->timeout = timeout;
    this->hexflag = hexflag;
    this->finished = false;
}

QCopWaiter::~QCopWaiter()
{
}

void QCopWaiter::start( bool ok )
{
    QCopHandler::start( ok );
    if ( ok ) {
        QCopChannel *chan = new QCopChannel( channel, this );
        QObject::connect
            ( chan, SIGNAL(received(QString,QByteArray)),
              this, SLOT(received(QString,QByteArray)) );
        if ( timeout >= 0 ) {
            QTimer::singleShot( timeout, this, SLOT(gotTimeout()) );
        }
    }
}

void QCopWaiter::received( const QString& msg, const QByteArray& data )
{
    if ( msg == this->msg && !finished ) {
        ParamInfo info = parseParameters( msg );

        QDataStream stream( data );
        if ( !info.parameters.isEmpty() ) {
            QStringList::Iterator it;
            for ( it = info.parameters.begin();
                  it != info.parameters.end(); ++it ) {
                QString value = convertToString( stream, *it, hexflag, false );
                puts( value.toLatin1().constData() );
            }
        }

        finished = true;
        emit done( true );
    }
}

void QCopWaiter::gotTimeout()
{
    if ( !finished ) {
        if ( exitOnTimeout )
            QApplication::exit(1);
        finished = true;
        emit done( false );
    }
}

QCopQuery::QCopQuery( const QString& channel, const QString& prefix,
                      int timeout, QObject *parent )
    : QCopHandler( parent )
{
    this->channel = channel;
    this->prefix = prefix;
    this->timeout = ( timeout == -1 ? 1000 : timeout );// Default 1 sec timeout.
    this->finished = false;
}

QCopQuery::~QCopQuery()
{
}

void QCopQuery::start( bool ok )
{
    QCopHandler::start( ok );
    if ( ok ) {
        // Hook onto the response channel.
        QString responseChannel = "QPE/Query/" + QUuid::createUuid().toString();
        QCopChannel *chan = new QCopChannel( responseChannel, this );
        QObject::connect
            ( chan, SIGNAL(received(QString,QByteArray)),
              this, SLOT(received(QString,QByteArray)) );
        if ( timeout >= 0 ) {
            QTimer::singleShot( timeout, this, SLOT(gotTimeout()) );
        }

        // Send the request to the channel we are querying.
        QCopEnvelope env( channel, "__query_qcop_messages(QString,QString)" );
        env << prefix;
        env << responseChannel;
    }
}

void QCopQuery::received( const QString& msg, const QByteArray& data )
{
    if ( msg == "queryResponse(QString)" && !finished ) {
        QDataStream stream( data );
        QString value;
        stream >> value;
        if ( ! responses.contains( value ) ) {
            responses += value;
            printf( "%s\n", value.toLatin1().constData() );
        }
    }
}

void QCopQuery::gotTimeout()
{
    if ( !finished ) {
        finished = true;
        emit done( true );
    }
}
