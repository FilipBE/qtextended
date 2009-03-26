CREATE TABLE currentsimcard (
        cardid VARCHAR(255),
        storage CHAR(2),
        firstindex INTEGER,
        lastindex INTEGER,
        labellimit INTEGER,
        numberlimit INTEGER,
        loaded BOOLEAN,
        UNIQUE(cardid, storage));
