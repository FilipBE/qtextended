CREATE TABLE servicehistory (
    id INTEGER primary key autoincrement,
    label VARCHAR(100),
    icon VARCHAR(100),
    service VARCHAR(100) NOT NULL,
    message VARCHAR(255),
    arguments BLOB
);

CREATE INDEX servicehist ON servicehistory (service,message,arguments);

-- Remove oldest record from the servicehistory table
-- after each insert into servicehistory
-- Keep on one line.
CREATE TRIGGER servicehistory_limit AFTER INSERT ON servicehistory BEGIN delete from servicehistory where id <= (select max(id) from servicehistory) - 100; END;
