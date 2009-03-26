CREATE TABLE simcardidmap (
    sqlid INTEGER,
    cardid VARCHAR(255),
    storage CHAR(2),
    cardindex INTEGER,
    UNIQUE(cardid, cardindex, storage));
