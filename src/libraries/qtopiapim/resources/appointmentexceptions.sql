CREATE TABLE appointmentexceptions (
    recid INTEGER NOT NULL,
    edate DATE NOT NULL,
    alternateid INTEGER, 
    UNIQUE(recid, edate), 
    FOREIGN KEY(recid) REFERENCES appointments(recid));
