-- Document Model sub-system SQL Schema

CREATE TABLE contentProps
(
    cid INTEGER NOT NULL,
    grp VARCHAR(255),
    name VARCHAR(255) NOT NULL,
    value BLOB,
    PRIMARY KEY (cid, grp, name)
);
