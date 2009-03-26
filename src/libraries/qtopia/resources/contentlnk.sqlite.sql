-- Document Model sub-system SQL Schema

-- SQL file optimised for SQL Lite.  Please see the comments in the
-- file contentlnk.sql for SQL92/99 standard sql.

-- Note that SQLite will convert all column types with the substring "CHAR"
-- into the SQLite type "TEXT" which is a pointer offset string type

-- Main Content Class
CREATE TABLE content 
(
    cid INTEGER PRIMARY KEY,        -- SQLite chokes on "AUTO_INCREMENT", will make this auto inc anyway
    uiName VARCHAR(100) NOT NULL,   -- indexed
    uiNameSortOrder VARCHAR(100) NOT NULL,
    mType INTEGER,          -- indexed
    drmFlags INTEGER,              -- enum "[p]lain", "[c]ontrolled", or "[u]nrenderable"
    docStatus CHAR(1),              -- enum "[d]oc", or "[b]in", indexed
    path VARCHAR(255),              -- path + filename, unique
    location INTEGER,
    icon VARCHAR(255),              -- path + filename
    lastUpdated INTEGER
);

CREATE INDEX cNameIndex ON content ( uiName );

CREATE INDEX cNameSortOrderIndex ON content ( uiNameSortOrder );

CREATE INDEX cMimeIndex ON content ( mType );

CREATE INDEX cDocStatusIndex ON content ( docStatus );

CREATE UNIQUE INDEX cLinkPathIndex ON content ( path, location );

CREATE INDEX cLocationIndex ON content ( location );

CREATE INDEX cPath on content (path);

PRAGMA user_version=110;
