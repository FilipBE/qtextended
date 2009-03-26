-- Document Model sub-system SQL Schema

CREATE TABLE locationLookup
(
    pKey INTEGER PRIMARY KEY,
    location VARCHAR(255) NOT NULL
);

CREATE UNIQUE INDEX cLocationLookupLocation ON locationLookup ( location );

CREATE TABLE mimeTypeLookup
(
    pKey INTEGER PRIMARY KEY,
    mimeType VARCHAR(100) NOT NULL
);

CREATE UNIQUE INDEX cMimeTypeLookupMimeType ON mimeTypeLookup ( mimeType );
