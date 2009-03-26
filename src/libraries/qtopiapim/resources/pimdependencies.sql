CREATE TABLE pimdependencies (
    srcrecid INTEGER,
    destrecid INTEGER,
    deptype VARCHAR(20) NOT NULL,
    PRIMARY KEY(srcrecid, destrecid, deptype));

CREATE INDEX pimdepssrcid ON pimdependencies(srcrecid);
CREATE INDEX pimdepsdestid ON pimdependencies(destrecid);
