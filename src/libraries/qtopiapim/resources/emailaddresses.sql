-- relationships, addr, phone could all be split out into other tables,
-- keeping like this as matches current UI.

CREATE TABLE emailaddresses (
    recid INTEGER,
    addr NVARCHAR(128) COLLATE NOCASE NOT NULL,
    label NVARCHAR(128) COLLATE NOCASE,
    UNIQUE(recid, addr),
    FOREIGN KEY(recid) REFERENCES contacts(recid)
);
