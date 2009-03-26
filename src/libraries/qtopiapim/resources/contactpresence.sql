CREATE TABLE contactpresence(uri NVARCHAR(256) COLLATE NOCASE, recid INTEGER NOT NULL, status INTEGER, statusstring NVARCHAR(255), message NVARCHAR(255), displayname NVARCHAR(255), updatetime TIMESTAMP, capabilities NVARCHAR(255), avatar NVARCHAR(255), PRIMARY KEY(uri,recid));

-- This should just be the primary key, actually
CREATE INDEX contactpresenceuri ON contactpresence(uri,recid);
CREATE INDEX contactpresencerecid ON contactpresence(recid,uri);
