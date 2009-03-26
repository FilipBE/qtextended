CREATE TABLE changelog (
    recid INTEGER,
    context INTEGER NOT NULL,
    created TIMESTAMP,
    modified TIMESTAMP,
    removed TIMESTAMP,
    PRIMARY KEY(recid));
