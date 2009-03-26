INSERT INTO appointments (recid, description, location, 'start', 'end', allday,
        starttimezone, endtimezone, alarm, alarmdelay, repeatrule, repeatfrequency,
        repeatenddate, repeatweekflags, context)
    SELECT convertRecId(recid), description, location, 'start', 'end', allday, starttimezone,
       endtimezone, alarm, alarmdelay, repeatrule, repeatfrequency,
       repeatenddate, repeatweekflags, 2
       FROM appointments_old;
