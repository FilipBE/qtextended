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

void cleanOutCalendar1()
{
    QVERIFY(cleanOutTable());
    QVERIFY(checkForEmptyTable());
}

void addClientRecord1()
{
    QByteArray newRecord =
"<Appointment>\n"
"    <Identifier localIdentifier=\"false\">Client Identifier</Identifier>\n"
"    <Description>Description</Description>\n"
"    <Location>Location</Location>\n"
"    <TimeZone>TimeZone</TimeZone>\n"
"    <When>\n"
"        <Start>2001-01-01T09:00:00Z</Start>\n"
"        <End>2001-01-01T09:35:00Z</End>\n"
"    </When>\n"
"    <Alarm>\n"
"        <Type>Audible</Type>\n"
"        <Delay>5</Delay>\n"
"    </Alarm>\n"
"    <Notes>Notes blah blah blah\n"
"is this how a newline is represented?</Notes>\n"
"    <Categories>\n"
"        <Category>Foo</Category>\n"
"    </Categories>\n"
"</Appointment>\n";
    int year = QDate::currentDate().year();
    year++;
    /* move this to next year so the alarm isn't removed from it */
    newRecord.replace( QString("2001").toLocal8Bit(), QString("%1").arg(year).toLocal8Bit() );
    QVERIFY(addClientRecord( newRecord ));

    QByteArray expected =
"<Appointment>\n"
"<Identifier>Client Identifier</Identifier>\n"
"<Description>Description</Description>\n"
"<Location>Location</Location>\n"
"<TimeZone>TimeZone</TimeZone>\n"
"<When>\n"
"<Start>2001-01-01T09:00:00Z</Start>\n"
"<End>2001-01-01T09:35:00Z</End>\n"
"</When>\n"
"<Alarm>\n"
"<Type>Audible</Type>\n"
"<Delay>5</Delay>\n"
"</Alarm>\n"
"<Repeat>\n"
"</Repeat>\n"
"<Notes>Notes blah blah blah\n"
"is this how a newline is represented?</Notes>\n"
"<Categories>\n"
"<Category>Foo</Category>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Appointment>\n";
    /* this was moved to to next year */
    expected.replace( QString("2001").toLocal8Bit(), QString("%1").arg(year).toLocal8Bit() );
    QVERIFY(checkForAddedItem( expected ));
}

void cleanOutCalendar2()
{
    QVERIFY(cleanOutTable());
    QVERIFY(checkForEmptyTable());
}

void addClientRecord2()
{
    QByteArray newRecord =
"<Appointment>\n"
"    <Identifier localIdentifier=\"false\">Client Identifier 2</Identifier>\n"
"    <Description>Description</Description>\n"
"    <Location>Location</Location>\n"
"    <TimeZone></TimeZone>\n"
"    <When>\n"
"        <Start>2001-01-01T09:00:00</Start>\n"
"        <End>2001-01-01T09:35:00</End>\n"
"    </When>\n"
"    <Alarm>\n"
"        <Type>Audible</Type>\n"
"        <Delay>5</Delay>\n"
"    </Alarm>\n"
"    <Notes>Notes blah blah blah\n"
"is this how a newline is represented?</Notes>\n"
"    <Categories>\n"
"        <Category>Foo</Category>\n"
"    </Categories>\n"
"</Appointment>\n";
    QVERIFY(addClientRecord( newRecord ));

    QByteArray expected =
"<Appointment>\n"
"<Identifier>Client Identifier 2</Identifier>\n"
"<Description>Description</Description>\n"
"<Location>Location</Location>\n"
"<TimeZone></TimeZone>\n"
"<When>\n"
"<Start>2001-01-01T09:00:00</Start>\n"
"<End>2001-01-01T09:35:00</End>\n"
"</When>\n"
"<Alarm>\n"
#ifdef Q_WS_QWS // Qtopia doesn't remove alarm data from alarms in the past
"<Type>Audible</Type>\n"
"<Delay>5</Delay>\n"
#endif
"</Alarm>\n"
"<Repeat>\n"
"</Repeat>\n"
"<Notes>Notes blah blah blah\n"
"is this how a newline is represented?</Notes>\n"
"<Categories>\n"
"<Category>Foo</Category>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Appointment>\n";
    QVERIFY(checkForAddedItem( expected ));
}

void cleanOutCalendar3()
{
    QVERIFY(cleanOutTable());
    QVERIFY(checkForEmptyTable());
}

void addClientRecord3()
{
    QByteArray newRecord =
"<Appointment>\n"
"    <Identifier localIdentifier=\"false\">Client Identifier 3</Identifier>\n"
"    <Description>Description</Description>\n"
"    <Location>Location</Location>\n"
"    <When>\n"
"        <StartDate>2001-01-01</StartDate>\n"
"        <EndDate>2001-01-01</EndDate>\n"
"    </When>\n"
"    <Notes></Notes>\n"
"</Appointment>\n";
    QVERIFY(addClientRecord( newRecord ));

    QByteArray expected =
"<Appointment>\n"
"<Identifier>Client Identifier 3</Identifier>\n"
"<Description>Description</Description>\n"
"<Location>Location</Location>\n"
"<TimeZone></TimeZone>\n"
"<When>\n"
"<StartDate>2001-01-01</StartDate>\n"
"<EndDate>2001-01-01</EndDate>\n"
"</When>\n"
"<Alarm>\n"
"</Alarm>\n"
"<Repeat>\n"
"</Repeat>\n"
"<Notes></Notes>\n"
"<Categories>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Appointment>\n";
    QVERIFY(checkForAddedItem( expected ));
}

void editClientRecord3()
{
    QByteArray newRecord =
"<Appointment>\n"
"    <Identifier>Client Identifier 3</Identifier>\n"
"    <Description>Description</Description>\n"
"    <Location>Location</Location>\n"
"    <When>\n"
"        <StartDate>2001-01-02</StartDate>\n"
"        <EndDate>2001-01-03</EndDate>\n"
"    </When>\n"
"    <Notes></Notes>\n"
"</Appointment>\n";
    QString serverId;
    foreach ( QString id, idMap.keys() ) {
        if ( idMap[id] == "Client Identifier 3" ) {
            serverId = id;
            break;
        }
    }
    QVERIFY(!serverId.isEmpty());
    newRecord.replace("Client Identifier 3", serverId.toLocal8Bit());
    QVERIFY(editClientRecord(newRecord));

    QByteArray expected =
"<Appointment>\n"
"<Identifier>Client Identifier 3</Identifier>\n"
"<Description>Description</Description>\n"
"<Location>Location</Location>\n"
"<TimeZone></TimeZone>\n"
"<When>\n"
"<StartDate>2001-01-02</StartDate>\n"
"<EndDate>2001-01-03</EndDate>\n"
"</When>\n"
"<Alarm>\n"
"</Alarm>\n"
"<Repeat>\n"
"</Repeat>\n"
"<Notes></Notes>\n"
"<Categories>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Appointment>\n";
    QVERIFY(checkForAddedItem( expected ));
}

void repeatDailyClientRecord3()
{
    QByteArray newRecord =
"<Appointment>\n"
"    <Identifier>Client Identifier 3</Identifier>\n"
"    <Description>Description</Description>\n"
"    <Location>Location</Location>\n"
"    <When>\n"
"        <StartDate>2001-01-01</StartDate>\n"
"        <EndDate>2001-01-01</EndDate>\n"
"    </When>\n"
"    <Repeat>\n"
"<Type>Daily</Type>"
"<Frequency>2</Frequency>"
"<Until></Until>"
"    </Repeat>\n"
"    <Notes></Notes>\n"
"</Appointment>\n";
    QString serverId;
    foreach ( QString id, idMap.keys() ) {
        if ( idMap[id] == "Client Identifier 3" ) {
            serverId = id;
            break;
        }
    }
    QVERIFY(!serverId.isEmpty());
    newRecord.replace("Client Identifier 3", serverId.toLocal8Bit());
    QVERIFY(editClientRecord(newRecord));

    QByteArray expected =
"<Appointment>\n"
"<Identifier>Client Identifier 3</Identifier>\n"
"<Description>Description</Description>\n"
"<Location>Location</Location>\n"
"<TimeZone></TimeZone>\n"
"<When>\n"
"<StartDate>2001-01-01</StartDate>\n"
"<EndDate>2001-01-01</EndDate>\n"
"</When>\n"
"<Alarm>\n"
"</Alarm>\n"
"<Repeat>\n"
"<Type>Daily</Type>\n"
"<Frequency>2</Frequency>\n"
"<Until></Until>\n"
"</Repeat>\n"
"<Notes></Notes>\n"
"<Categories>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Appointment>\n";
    QVERIFY(checkForAddedItem( expected ));
}

void cleanOutCalendar4()
{
    QVERIFY(cleanOutTable());
    QVERIFY(checkForEmptyTable());
}

void repeatWeeklyClientRecord4()
{
    QByteArray newRecord =
"<Appointment>\n"
"    <Identifier localIdentifier=\"false\">Client Identifier 4</Identifier>\n"
"    <Description>Description</Description>\n"
"    <Location>Location</Location>\n"
"    <When>\n"
"        <Start>2001-01-01T10:00:00</Start>\n"
"        <End>2001-01-01T11:00:00</End>\n"
"    </When>\n"
"    <Repeat>\n"
"<Type>Weekly</Type>"
"<Frequency>1</Frequency>"
"<WeekMask>Monday Tuesday Wednesday Thursday Friday</WeekMask>"
"    </Repeat>\n"
"    <Notes></Notes>\n"
"</Appointment>\n";
    QVERIFY(addClientRecord( newRecord ));

    QByteArray expected =
"<Appointment>\n"
"<Identifier>Client Identifier 4</Identifier>\n"
"<Description>Description</Description>\n"
"<Location>Location</Location>\n"
"<TimeZone></TimeZone>\n"
"<When>\n"
"<Start>2001-01-01T10:00:00</Start>\n"
"<End>2001-01-01T11:00:00</End>\n"
"</When>\n"
"<Alarm>\n"
"</Alarm>\n"
"<Repeat>\n"
"<Type>Weekly</Type>\n"
"<Frequency>1</Frequency>\n"
"<Until></Until>\n"
"<WeekMask>Monday Tuesday Wednesday Thursday Friday</WeekMask>\n"
"</Repeat>\n"
"<Notes></Notes>\n"
"<Categories>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Appointment>\n";
    QVERIFY(checkForAddedItem( expected ));
}

void cleanOutCalendar5()
{
    QVERIFY(cleanOutTable());
    QVERIFY(checkForEmptyTable());
}

void repeatMonthlyDayClientRecord5()
{
    QByteArray newRecord =
"<Appointment>\n"
"    <Identifier localIdentifier=\"false\">Client Identifier 5</Identifier>\n"
"    <Description>Description</Description>\n"
"    <Location>Location</Location>\n"
"    <When>\n"
"        <Start>2001-01-01T10:00:00</Start>\n"
"        <End>2001-01-01T11:00:00</End>\n"
"    </When>\n"
"    <Repeat>\n"
"<Type>MonthlyDay</Type>"
"<Frequency>4</Frequency>"
"<Until>2002-06-01</Until>"
"<WeekMask>Monday</WeekMask>"
"    </Repeat>\n"
"    <Notes></Notes>\n"
"</Appointment>\n";
    QVERIFY(addClientRecord( newRecord ));

    QByteArray expected =
"<Appointment>\n"
"<Identifier>Client Identifier 5</Identifier>\n"
"<Description>Description</Description>\n"
"<Location>Location</Location>\n"
"<TimeZone></TimeZone>\n"
"<When>\n"
"<Start>2001-01-01T10:00:00</Start>\n"
"<End>2001-01-01T11:00:00</End>\n"
"</When>\n"
"<Alarm>\n"
"</Alarm>\n"
"<Repeat>\n"
"<Type>MonthlyDay</Type>\n"
"<Frequency>4</Frequency>\n"
#ifdef Q_WS_QWS // Qtopia preserves inaccurate values (truncates on display)
"<Until>2002-06-01</Until>\n"
#else // Outlook truncates inaccurate values
"<Until>2002-05-06</Until>\n"
#endif
"<WeekMask>Monday</WeekMask>\n"
"</Repeat>\n"
"<Notes></Notes>\n"
"<Categories>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Appointment>\n";
    QVERIFY(checkForAddedItem( expected ));
}

// FIXME This test doesn't work because the data is invalid and Outlook does something a bit unpredictable with it.
#if 0
void cleanOutCalendar6()
{
    QVERIFY(cleanOutTable());
    QVERIFY(checkForEmptyTable());
}

void repeatMonthlyEndDayClientRecord6()
{
    QByteArray newRecord =
"<Appointment>\n"
"    <Identifier localIdentifier=\"false\">Client Identifier 6</Identifier>\n"
"    <Description>Description</Description>\n"
"    <Location>Location</Location>\n"
"    <When>\n"
"        <Start>2001-01-27T10:00:00</Start>\n"
"        <End>2001-01-27T11:00:00</End>\n"
"    </When>\n"
"    <Repeat>\n"
"<Type>MonthlyEndDay</Type>"
"<Frequency>12</Frequency>"
"<WeekMask>Saturday</WeekMask>"
"    </Repeat>\n"
"    <Notes></Notes>\n"
"</Appointment>\n";
    QVERIFY(addClientRecord( newRecord ));

    QByteArray expected =
"<Appointment>\n"
"<Identifier>Client Identifier 6</Identifier>\n"
"<Description>Description</Description>\n"
"<Location>Location</Location>\n"
"<TimeZone></TimeZone>\n"
"<When>\n"
"<Start>2001-01-27T10:00:00</Start>\n"
"<End>2001-01-27T11:00:00</End>\n"
"</When>\n"
"<Alarm>\n"
"</Alarm>\n"
"<Repeat>\n"
"<Type>MonthlyEndDay</Type>\n"
"<Frequency>12</Frequency>\n"
"<Until></Until>\n"
"<WeekMask>Saturday</WeekMask>\n"
"</Repeat>\n"
"<Notes></Notes>\n"
"<Categories>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Appointment>\n";
    QVERIFY(checkForAddedItem( expected ));
}
#endif

