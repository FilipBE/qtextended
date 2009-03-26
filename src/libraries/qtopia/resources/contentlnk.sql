-- Document Model sub-system SQL Schema

CREATE TABLE content 
(
    cid INTEGER PRIMARY KEY,
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
