-- Document Model sub-system SQL Schema

-- mid auto incrment
-- SQLite will autoincrement as its INTEGER PRIMARY KEY,
--  INTEGER PRIMARY KEY AUTOINCREMENT can be used for a slightly different
--  increment algorithm.
-- MySQL uses AUTO_INCREMENT
-- Mimer documentation does not appear to list an auto increment feature,
--  however may be possible to do using Sequences.

CREATE TABLE mapCategoryToContent
(
    mid INTEGER PRIMARY KEY,
    categoryid VARCHAR(255) NOT NULL,
    cid INTEGER NOT NULL,
    FOREIGN KEY(categoryid) REFERENCES category( categoryid ),
    FOREIGN KEY(cid) REFERENCES content( cid ) 
);

CREATE INDEX mCatIndex ON mapCategoryToContent ( categoryid );

CREATE INDEX mCidIndex ON mapCategoryToContent ( cid );

CREATE UNIQUE INDEX mCidCatIndex ON mapCategoryToContent ( cid, categoryid );
