CREATE TABLE simlabelidmap (
    sqlid INTEGER,
    cardid VARCHAR(255),
    storage CHAR(2),
    label INTEGER,
    UNIQUE(cardid, label, storage), UNIQUE(sqlid));
