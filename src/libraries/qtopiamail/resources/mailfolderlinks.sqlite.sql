CREATE TABLE mailfolderlinks (
    id INTEGER,
    descendantid INTEGER,
    FOREIGN KEY (id) REFERENCES mailfolders(id),
    FOREIGN KEY (descendantid) REFERENCES mailfolders(id));
