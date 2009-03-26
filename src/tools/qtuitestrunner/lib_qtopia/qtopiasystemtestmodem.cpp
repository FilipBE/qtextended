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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//
#include <QtopiaSystemTestModem>
#include <QSystemTest>
#include <CallManager>

#include <QPointer>
#include <PhoneSimServer>
#include <HardwareManipulator>

#include "qtopiasystemtest_p.h"

class PhoneTest: public HardwareManipulator
{
Q_OBJECT
public:
    PhoneTest(QObject *parent=0) : HardwareManipulator(parent) {}
    QtopiaSystemTestModemPrivate *d;
public slots:
    void unsolicitedCommand(QString const &data);
    void variableChanged(QString const &name, QString const &value);
    void handleFromData( const QString& );
    void handleToData( const QString& );
    void setupPersistentVariables(QVariantMap const &persistent);

};

class QtopiaSystemTestModemPrivate
{
public:
    QtopiaSystemTestModemPrivate(QtopiaSystemTestModem *parent, QtopiaSystemTestPrivate *test);
    ~QtopiaSystemTestModemPrivate();

    QtopiaSystemTestModem *p;
    QPointer<QtopiaSystemTestPrivate> t;
    PhoneSimServer *server;
    QPointer<PhoneTest> manip;
    QVariantMap persistent;

    QMap<QString, QString> expected_modem_commands;
    QMap<QString, QString> expected_modem_responses;
    QEventLoop modem_event_loop;
    QStringList received_modem_commands;
    QStringList received_modem_responses;

};

class PhoneTestFactory : public HardwareManipulatorFactory
{
public:
    PhoneTestFactory(QtopiaSystemTestModemPrivate *p) : d(p) {}
    HardwareManipulator *create(QObject *parent) {
        PhoneTest *t = new PhoneTest(parent);
        t->d=d;
        d->manip = t;
        if (!QMetaObject::invokeMethod(t, "setupPersistentVariables", Qt::QueuedConnection, Q_ARG(QVariantMap, d->persistent)))
            qWarning("Persistent variables were lost");
        return t;
    }
private:
    QtopiaSystemTestModemPrivate *d;
};

#include "qtopiasystemtestmodem.moc"

/*!
    \preliminary
    \class QtopiaSystemTestModem
    \brief The QtopiaSystemTestModem class provides functionality for simulating modem events from within a system test.

    \ingroup qtuitest_systemtest
    \inpublicgroup QtUiTestModule

    The QtopiaSystemTestModem class encapsulates a simulated AT modem.  This allows phone
    events to be simulated, such as an incoming call or network handover.

    Note that creating or destroying a QtopiaSystemTestModem causes Qt Extended to restart, as this
    is the only way to change the modem used by Qtopia.

    An instance of this class can be obtained using the \l{QtopiaSystemTest::testModem()} function.  It is recommended to call this function
    in initTestCase to prevent Qt Extended restarting during a test.

    \sa QSystemTest
*/

/*!
    \internal
*/
QtopiaSystemTestModem::QtopiaSystemTestModem(QtopiaSystemTestPrivate *parent) : QObject(parent)
{
    d = new QtopiaSystemTestModemPrivate(this, parent);
    if (!d->server) {
        delete d; d = 0;
    }
    if (d && d->server && !isValid()) {
        qWarning("A test modem was successfully created but Qtopia didn't "
                 "connect to it.  Please ensure phonesim is not running.");
    }
}

/*!
    Destroy the simulated modem.

    Note that this function causes Qt Extended to restart, to ensure that the simulated
    modem is no longer being used.
*/
QtopiaSystemTestModem::~QtopiaSystemTestModem()
{
    delete d;
}

/*!
    \internal
*/
bool QtopiaSystemTestModem::isValid() const
{
    return d && d->server && d->server->rules() && d->server->rules()->callManager();
}

QtopiaSystemTestModemPrivate::QtopiaSystemTestModemPrivate(QtopiaSystemTestModem *parent, QtopiaSystemTestPrivate *test) : p(parent), t(test)
{
    server = 0;

    QString filename = QFileInfo(QString::fromAscii(__FILE__)).absolutePath() + "/qtopiasystemtestmodem.xml";
    if (!QFileInfo(filename).exists()) filename = ":/qtopiasystemtestmodem.xml";
    server = new PhoneSimServer(filename, 0, p);
    server->setHardwareManipulator(new PhoneTestFactory(this));

    QString port = QString::number(server->serverPort());

    // FIXME this code needs to be made device-independent.
    // ensureEnvironment should be made to work on any device.
    // See bug 210317.
    // This covers the greenphone...
    t->p->setSetting("$HOME/Settings/Trolltech/StartupFlags.conf", "PhoneDevice", "State", "sim:hostpc:" + port);
    if (t->p->queryFailed()) return;

    // And this covers the desktop...
    if (!t->p->runsOnDevice()) {
        t->ensureEnvironment("QTOPIA_PHONE_DEVICE", "sim:localhost:" + port);
        if (t->p->queryFailed()) return;
    }


    qWarning( "Restarting Qtopia to activate the Phone Simulator" );
    t->p->restartQtopia();
    if ( t->p->queryFailed() || !t->p->waitForValueSpace( "/System/Tasks/WindowManagement/State", "Active", 60000 ) || t->p->queryFailed() ) {
        delete server;
        server = 0;
    }
}

QtopiaSystemTestModemPrivate::~QtopiaSystemTestModemPrivate()
{
    if (!server) return;

    // Make it not use test modem on desktop
    if (!t->p->runsOnDevice())
        t->ensureEnvironment("QTOPIA_PHONE_DEVICE", QString());

    delete server;
    delete manip;
}

#define CM d->server->rules()->callManager()
#define CHK(ret) do {   \
    Q_ASSERT(this);     \
    if (!d->server || !d->server->rules() || !d->server->rules()->callManager()) {  \
        d->t->p->setQueryError("Qtopia not connected to QtopiaSystemTestModem");    \
        ret;            \
    }                   \
} while(0)

/*!
    Make the simulated modem emit arbitrary unsolicited data.
    \a string is the string emitted by the modem.

    Example:
    \code
        // Change operator name from within a system test
        testModem().setVariable("OP", "Qt Extended Comms");
        testModem().send("+CREG: 5");
    \endcode
*/
void QtopiaSystemTestModem::send(QString const &string)
{
    CHK(return);
    if (!d->manip) return;
    d->manip->unsolicitedCommand(string);
}

/*!
    Change the value of a specified phone simulator variable.
    \a name is the name of the variable to change, \a value is the new value of
    the variable.
    If \a persistent is true, the value of the variable will be retained when
    Qt Extended is restarted.  Otherwise the variable will be reset to its default value.

    Example:
    \code
        // Change operator name from within a system test
        testModem().setVariable("OP", "Qt Extended Comms");
        testModem().send("+CREG: 5");
    \endcode

    \sa send(), getVariable()
*/
void QtopiaSystemTestModem::setVariable(QString const &name, QString const &value, bool persistent)
{
    CHK(return);
    if (!d->manip) return;
    if (persistent)
        d->persistent[name] = value;
    else
        d->persistent.remove(name);
    d->manip->variableChanged(name, value);
}

/*!
    Returns the value of a specified phone simulator variable.
    \a name is the name of the variable to obtain the value from.

    Example:
    \code
        // Change operator name from within a system test, verify the result
        testModem().setVariable("OP", "Qt Extended Comms");
        var op = testModem().getVariable("OP");   // Returns "Qt Extended Comms"
    \endcode

    \sa send(), setVariable()
*/
QString QtopiaSystemTestModem::getVariable(QString const &name) const
{
    CHK(return QString());
    return d->server->rules()->variable(name);
}

/*!
    Forget all persistent values set through setVariable().
    The next time Qt Extended is restarted, all variables which were previously persistent
    will be set to their default values.
*/
void QtopiaSystemTestModem::resetPersistentVariables()
{
    CHK(return);
    d->persistent = QVariantMap();
}

void PhoneTest::unsolicitedCommand(QString const &data)
{
    HardwareManipulator::unsolicitedCommand(data);
}

/*!
    \internal
*/
void PhoneTest::handleToData(QString const &command)
{
    if(d->expected_modem_commands.count() > 0){
        d->received_modem_commands.append(command);
        return;
    }
    d->modem_event_loop.exit(1);
}

/*!
    \internal
*/
void PhoneTest::handleFromData(QString const &response)
{
    if(d->expected_modem_responses.count() > 0){
        d->received_modem_responses.append(response);
        return;
    }
    d->modem_event_loop.exit(1);
}

void PhoneTest::variableChanged(QString const &name, QString const &value)
{
    HardwareManipulator::variableChanged(name, value);
}

void PhoneTest::setupPersistentVariables(QVariantMap const &persistent)
{
    foreach(QString key, persistent.keys()) {
        variableChanged(key, persistent[key].toString());
    }
}

/*!
    Returns the hold request failure setting from the phone simulator.

    Example:
    \code
        // Retrieve the HOLD failure status from the simulator
        testModem().setHoldWillFail(true);
        QVERIFY( testModem().holdWillFail() );
    \endcode

    \sa setHoldWillFail()
*/
bool QtopiaSystemTestModem::holdWillFail() const
{
    CHK(return false);
    return CM->holdWillFail();
}

/*!
    Enables a simulated failure response on HOLD requests.
    \a value is the desired setting.

    Example:
    \code
        // Simulate a failed HOLD request within a system test
        testModem().setHoldWillFail(true);     // Hold failures enabled
        testModem().startIncomingCall(12345);  // Initiate an incoming call
        keyClick( Qt.Key_Call );
        selectContext("Hold");                  // Modem will respond with "ERROR"
    \endcode

    \sa holdWillFail()
*/
void QtopiaSystemTestModem::setHoldWillFail( bool value )
{
    CHK(return);
    CM->setHoldWillFail(value);
}

/*!
    Returns the activate request failure setting from the phone simulator.

    Example:
    \code
        // Retrieve the ACTIVATE failure status from the simulator
        testModem().setActivateWillFail(true);
        QVERIFY( testModem().activateWillFail() );         // returns 'true'
    \endcode

    \sa setActivateWillFail()
*/
bool QtopiaSystemTestModem::activateWillFail() const 
{
    CHK(return false);
    return CM->activateWillFail();
}

/*!
    Enables a simulated failure response on ACTIVATE requests eg. alternate between an active and held call.
    \a value is the desired setting.

    Example:
    \code
        // Simulate a failed ACTIVATE request within a system test
        testModem().setActivateWillFail(true); // Activate failures enabled
        testModem().startIncomingCall(12345);  // Initiate an incoming call
        keyClick( Qt.Key_Call );
        testModem().startIncomingCall(67890);  // Initiate an second incoming call
        keyClick( Qt.Key_Call ) );
        selectContext( "Swap" );       // Request ACTIVATE on held call, modem will respond with "ERROR"
    \endcode

    \sa activateWillFail()
*/
void QtopiaSystemTestModem::setActivateWillFail( bool value )
{
    CHK(return);
    CM->setActivateWillFail( value );
}

/*!
    Returns the join request failure setting from the phone simulator.

    Example:
    \code
        // Retrieve the JOIN failure status from the simulator
        testModem().setJoinWillFail(true);
        QVERIFY( testModem().joinWillFail() );             // returns 'true'
    \endcode

    \sa setJoinWillFail()
*/
bool QtopiaSystemTestModem::joinWillFail() const
{
    CHK(return false);
    return CM->joinWillFail();
}

/*!
    Enables a simulated failure response on JOIN requests ie. creating a multiparty call.
    \a value is the desired setting.

    Example:
    \code
        // Simulate a failed JOIN request within a system test
        testModem().setJoinWillFail(true);     // Join failures enabled
        testModem().startIncomingCall(12345);  // Initiate an incoming call
        keyClick( Qt::Key_Call );
        testModem().startIncomingCall(67890);  // Initiate an second incoming call
        keyClick( Qt::Key_Call );
        selectContext( "Join" );       // Request JOIN on calls, modem will respond with "ERROR"
    \endcode

    \sa joinWillFail()
*/
void QtopiaSystemTestModem::setJoinWillFail( bool value )
{
    CHK(return);
    CM->setJoinWillFail( value );
}

/*!
    \internal
*/
bool QtopiaSystemTestModem::deflectWillFail() const
{
    CHK(return false);
    return CM->deflectWillFail();
}

/*!
    \internal
*/
void QtopiaSystemTestModem::setDeflectWillFail( bool value )
{
    CHK(return);
    CM->setDeflectWillFail( value );
}

/*!
    Prompts the modem to 'remotely' end all calls, simulating a hang up event from all callers.

    Example:
    \code
        // Simulate a universal remote hangup event
        testModem().startIncomingCall(12345);  // Initiate an incoming call
        keyClick( Qt.Key_Call );
        testModem().startIncomingCall(67890);  // Initiate an second incoming call
        keyClick( Qt.Key_Call );
        testModem().hangupAll();               // All calls are ended
    \endcode

    \sa hangupConnected(), hangupHeld(), hangupConnectedAndHeld(), hangupCall()
*/
void QtopiaSystemTestModem::hangupAll()
{
    CHK(return);
    CM->hangupAll();
}

/*!
    Prompts the modem to 'remotely' end all active calls, simulating a hang up event from the active callers.

    Example:
    \code
        // Simulate an active calls remote hangup event
        testModem().startIncomingCall(12345);  // Initiate an incoming call (call 1)
        keyClick( Qt.Key_Call );
        testModem().startIncomingCall(67890);  // Initiate an second incoming call (call 2)
        testModem().hangupConnected();         // Only call 1 is ended
    \endcode

    \sa hangupAll(), hangupHeld(), hangupConnectedAndHeld(), hangupCall()
*/
void QtopiaSystemTestModem::hangupConnected()
{
    CHK(return);
    CM->hangupConnected();
}

/*!
    Prompts the modem to 'remotely' end all held calls, simulating a hang up event from all callers on hold.

    Example:
    \code
        // Simulate a held calls remote hangup event
        testModem().startIncomingCall(12345);  // Initiate an incoming call (call 1)
        keyClick( Qt.Key_Call );
        testModem().startIncomingCall(67890);  // Initiate an second incoming call (call 2)
        keyClick( Qt.Key_Call );                // Call 2 is active, Call 1 is placed on hold
        testModem().hangupHeld();              // Call 1 is ended
    \endcode

    \sa hangupAll(), hangupConnected(), hangupConnectedAndHeld(), hangupCall()
*/
void QtopiaSystemTestModem::hangupHeld()
{
    CHK(return);
    CM->hangupHeld();
}

/*!
    Prompts the modem to 'remotely' end all active and held calls, simulating a hang up event from all callers active and on hold.

    Example:
    \code
        // Simulate an active and held calls remote hangup event
        testModem().startIncomingCall(12345);  // Initiate an incoming call (call 1)
        keyClick( Qt.Key_Call );
        testModem().startIncomingCall(67890);  // Initiate an second incoming call (call 2)
        keyClick( Qt.Key_Call );                // Call 2 is active, Call 1 is placed on hold
        testModem().startIncomingCall(54321);  // Initiate an incoming call (call 3)
        testModem().hangupConnectedAndHeld();  // Calls 1 and 2 are ended, Call 3 is still waiting
    \endcode

    \sa hangupAll(), hangupConnected(), hangupHeld(), hangupCall()
*/
void QtopiaSystemTestModem::hangupConnectedAndHeld()
{
    CHK(return);
    CM->hangupConnectedAndHeld();
}

/*!
    Prompts the modem to 'remotely' end a specified call, simulating a hang up event from the specified caller.
    \a id is the 'call id' of the target call item, where activated calls in a call manager begin at '1' and are incremented on each
    activated call.
    Example:
    \code
        // Simulate a remote hangup event from a specified caller
        testModem().startIncomingCall(12345);  // Initiate an incoming call (Call 1)
        keyClick( Qt.Key_Call );
        testModem().startIncomingCall(67890);  // Initiate an second incoming call (Call 2)
        keyClick( Qt.Key_Call );
        testModem().hangupCall(1);             // Call 1 is ended
    \endcode

    \sa hangupAll(), hangupConnected(), hangupHeld(), hangupConnectedAndHeld()
*/
void QtopiaSystemTestModem::hangupCall( int id )
{
    CHK(return);
    CM->hangupCall( id );
}

/*!
    Returns the maximum number of concurrent parties currently allowed in the test modem.

    Example:
    \code
        // Retrieve the maximum number of allowed participants in a multiparty call
        testModem().setMultipartyLimit(5);
        var lim = testModem().multipartyLimit();         // returns 5
    \endcode

    \sa setMultipartyLimit()
*/
int QtopiaSystemTestModem::multipartyLimit() const
{
    CHK(return 0);
    return CM->multipartyLimit();
}

/*!
    Sets the maximum number of concurrent parties in the test modem.
    \a value is the target maximum number of parties.

    Example:
    \code
        // Sets the maximum number of participants in a multiparty call
        testModem().setMultipartyLimit(2);     // Set maximum participants to '2'
        testModem().startIncomingCall(12345);  // Initiate an incoming call (Call 1)
        keyClick( Qt.Key_Call );
        testModem().startIncomingCall(67890);  // Initiate an second incoming call (Call 2)
        keyClick( Qt.Key_Call );
        selectContext( "Join" );
        testModem().startIncomingCall(54321);  // Initiate an incoming call (Call 3)
        keyClick( Qt.Key_Call );
        selectContext( "Join" );                  // Maximum participants reached, join will fail
    \endcode

    \sa multipartyLimit()
*/
void QtopiaSystemTestModem::setMultipartyLimit( int value )
{
    CHK(return);
    CM->setMultipartyLimit( value );
}

/*!
    \internal
*/
void QtopiaSystemTestModem::startIncomingCall( const QString& number, bool dialBack )
{
    CHK(return);
    CM->startIncomingCall( number, dialBack );
}

/*!
    Initiates an incoming call.
    \a number is the phone number of the calling party (can be empty).

    Example:
    \code
        // Initiate an incoming call
        testModem().startIncomingCall(12345);  // Initiate an incoming call (Call 1)
        keyClick( Qt.Key_Call );   // Call 1 now active
    \endcode
*/
void QtopiaSystemTestModem::startIncomingCall( const QString& number )
{
    CHK(return);
    CM->startIncomingCall( number );
}

/*!
    Initiates an incoming short message.
    \a type indicates the message class (0 - 3)
    \a sender is the phone number of the sending party
    \a serviceCenter is the SMS forwarding service
    \a text is the short message text.

    Example:
    \code
        // Initiate an incoming SMS
        expectMessageBox( "New Message", "Do you wish to view", "Yes", 5000 ){
            testModem().incomingSMS( "12345", "67090", "Hello" );
        }
    \endcode
*/
void QtopiaSystemTestModem::incomingSMS( const int type, const QString &sender, const QString &serviceCenter, const QString &text )
{
    CHK(return);
    d->server->rules()->getMachine()->constructSMSMessage( type, sender, serviceCenter, text );
}

/*!
    Adds an expected AT command to the list. Set before the action prompting the command takes place.
    \a key is a reference to the command item, can be an arbitrary string. \a command is the expected command text.

    Example:
    \code
        // Test enabling Voice Call Waiting feature sends the correct AT command
        testModem().expectCommand("enable_voice", "AT+CCWA=1,1,1");
        // Enable Voice CF checkbox
        keyClick( Qt.Key_Select );
        QVERIFY( testModem().waitCommand("enable_voice", 5000) );
    \endcode
    \sa waitCommand(), expectResponse(), waitResponse()
*/
void QtopiaSystemTestModem::expectCommand( QString const& key, QString const& command  )
{
    CHK(return);
    d->expected_modem_commands[key] = command;
}


/*!
    Adds an expected AT response to the list. Set before the action prompting the response takes place.
    \a key is a reference to the response, can be an arbitrary string. \a response is the expected response text.

    Example:
    \code
        // Test enabling Voice Call Waiting receives the expected modem response
        testModem().expectResponse("enable_voice", "+CCWA=1,1");
        // Enable Voice CF checkbox
        keyClick( Qt.Key_Select );
        QVERIFY( testModem().waitResponse("enable_voice", 5000) );
    \endcode
    \sa waitResponse(), expectCommand(), waitCommand()
*/
void QtopiaSystemTestModem::expectResponse( QString const& key, QString const& response )
{
    CHK(return);
    d->expected_modem_responses[key] = response;
}


/*!
    Verifies an expected command has been sent.  Will fail if not set with expectedCommand().
    \a key is a reference to the command item as set in expectCommand(). \a timeout indicates time to verification failure.

    Returns true if the specified command occurs within the given timeout.

    Example:
    \code
        // Test enabling Voice Call Waiting feature sends the correct AT command
        testModem().expectCommand("enable_voice", "AT+CCWA=1,1,1");
        // Enable Voice CF checkbox
        keyClick( Qt.Key_Select );
        QVERIFY( testModem().waitCommand("enable_voice", 5000) );
    \endcode
    \sa expectCommand(), expectResponse(), waitResponse()
*/
bool QtopiaSystemTestModem::waitCommand( QString const& key, int timeout  )
{
    CHK(return false);
    QString expectedCommand = d->expected_modem_commands.take(key);
    if(expectedCommand == QString())
        return false;

    QTime t;
    t.start();

    do{
        if( d->received_modem_commands.contains(expectedCommand) )
            return true;
        QTest::qWait(10);
    }while( t.elapsed() < timeout );

    return false;
}


/*!
    Verifies an expected response has been received.  Will fail if not set with expectedResponse().
    \a key is a reference to the response item as set in expectResponse(). \a timeout indicates time to verification failure.

    Returns true if the specified response is received within the given timeout.

    Example:
    \code
        // Test enabling Voice Call Waiting feature prompts the correct AT response
        testModem().expectResponse("enable_voice", "+CCWA=1,1");
        // Enable Voice CF checkbox
        keyClick( Qt.Key_Select );
        QVERIFY( testModem().waitResponse("enable_voice", 5000) );
    \endcode
    \sa expectResponse(), expectCommand(), waitCommand()
*/
bool QtopiaSystemTestModem::waitResponse( QString const& key, int timeout )
{
    CHK(return false);
    QString expectedResponse = d->expected_modem_responses.take(key);
    if(expectedResponse == QString())
        return false;

    QTime t;
    t.start();

    do{
        if( d->received_modem_responses.contains(expectedResponse) )
            return true;
        QTest::qWait(10);
    }while( t.elapsed() < timeout );
    return false;
}

/*!
    Installs \a app as the SIM toolkit application in the simulated modem.
    If \a app is null, return to the default SIM toolkit application.

    The previous SIM toolkit application will be deleted if it is not the default.
*/
void QtopiaSystemTestModem::installToolkitApp( SimApplication *app )
{
    d->server->rules()->setSimApplication( app );
}

/*!
    Deletes the test modem and restarts Qtopia.
*/
void QtopiaSystemTestModem::destroy()
{
    QtopiaSystemTest* sys = 0;
    if (d->t) sys = d->t->p;

    delete this;

    if (sys) sys->restartQtopia();
}

