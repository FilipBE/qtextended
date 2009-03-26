CREATE TABLE sqlsources (
    condensedid INTEGER PRIMARY KEY,
    contextid VARCHAR(255),
    subsource VARCHAR(255),
    last_sync TIMESTAMP,
    UNIQUE(contextid, subsource));

-- Defaults for sql sources tables to allow import.  Sources from 4.1.6
-- contacts
INSERT INTO sqlsources VALUES(1, '{a7a2832c-cdb3-40b6-9d95-6cd31e05647d}', 'default', NULL);
-- appointments
INSERT INTO sqlsources VALUES(2, '{5ecdd517-9aed-4e4b-b248-1970c56eb49a}', 'default', NULL);
-- tasks
INSERT INTO sqlsources VALUES(3, '{10b16464-b37b-4e0b-9f15-7a5d895b70c6}', 'default', NULL);
