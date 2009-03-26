CREATE TABLE deletedmessages( 
    id INTEGER PRIMARY KEY NOT NULL,
    parentaccountid INTEGER NOT NULL,
    serveruid VARCHAR,
    frommailbox VARCHAR,
    FOREIGN KEY (parentaccountid) REFERENCES mailaccounts(id));
