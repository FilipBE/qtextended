CREATE TABLE taskcustom (
        recid INTEGER NOT NULL,
        fieldname VARCHAR(255) COLLATE NOCASE NOT NULL,
        fieldvalue VARCHAR(255) COLLATE NOCASE, 
	UNIQUE(recid, fieldname), 
	FOREIGN KEY(recid) REFERENCES tasks(recid));
