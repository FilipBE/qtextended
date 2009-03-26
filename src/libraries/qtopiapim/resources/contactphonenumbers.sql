CREATE TABLE contactphonenumbers (
    phone_number VARCHAR(100) NOT NULL,
    recid INTEGER,
    phone_type INTEGER,
    FOREIGN KEY(recid) REFERENCES contacts(recid)
);

CREATE INDEX contactphonenumbersbytype ON contactphonenumbers (phone_type, phone_number);
CREATE INDEX contactphonenumbersindex ON contactphonenumbers (recid);
CREATE INDEX contactphonenumbersnumbers ON contactphonenumbers (phone_number, recid);
CREATE INDEX contactphnenumberscontacts ON contactphonenumbers (recid, phone_number);
