CREATE TABLE taskcategories (
    recid INTEGER NOT NULL,
    categoryid VARCHAR(255) COLLATE NOCASE NOT NULL,
    UNIQUE(recid, categoryid),
    FOREIGN KEY(recid) REFERENCES tasks(recid),
    FOREIGN KEY (categoryid) REFERENCES categories(categoryid));
