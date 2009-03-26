CREATE TABLE contactcategories (
    recid INTEGER NOT NULL,
    categoryid VARCHAR(255) COLLATE NOCASE NOT NULL,
    UNIQUE(recid, categoryid), 
    FOREIGN KEY(recid) REFERENCES contacts(recid), 
    FOREIGN KEY (categoryid) REFERENCES categories(categoryid));

-- create index on joined column.
CREATE INDEX contactcategoriesindex ON contactcategories (categoryid);
