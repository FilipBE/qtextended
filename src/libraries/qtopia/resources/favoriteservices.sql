CREATE TABLE favoriteservices (
    id INTEGER primary key autoincrement,
    sortIndex INTEGER,
    speedDial INTEGER,
    label VARCHAR(100),
    icon VARCHAR(100),
    service VARCHAR(100) NOT NULL,
    message VARCHAR(255),
    arguments BLOB,
    optionalMap BLOB
);
