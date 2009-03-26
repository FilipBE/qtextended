CREATE TABLE callhistory (
        recid INTEGER PRIMARY KEY,
        servicetype VARCHAR(25),
        calltype INTEGER NOT NULL,
        phonenumber VARCHAR(100),
        contactid INTEGER,
        time TIMESTAMP,
        endtime TIMESTAMP,
        timezoneid INTEGER,
        simrecord INTEGER
);

