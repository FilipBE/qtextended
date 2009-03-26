CREATE TABLE appointments (
    recid INTEGER NOT NULL,
    description NVARCHAR(255) COLLATE NOCASE,
    location NVARCHAR(255) COLLATE NOCASE,
    "start" TIMESTAMP,
    "end" TIMESTAMP,
    allday SMALLINT,
    starttimezone VARCHAR(32) COLLATE NOCASE,
    endtimezone VARCHAR(32) COLLATE NOCASE,
    alarm INTEGER,
    alarmdelay INTEGER,
    repeatrule INTEGER,
    repeatfrequency INTEGER,
    repeatenddate DATE,
    repeatweekflags INTEGER,
    context INTEGER NOT NULL,
    PRIMARY KEY(recid));


CREATE INDEX appointments_end ON appointments ("end");
CREATE INDEX appointments_repeatenddate ON appointments ("repeatenddate");
CREATE INDEX appointments_repeats ON appointments ("repeatrule", "repeatenddate");
