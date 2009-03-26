CREATE TABLE mailstatusflags (
    name VARCHAR(100) NOT NULL,
    context VARCHAR(100) NOT NULL,
    statusbit INTEGER NOT NULL,
    PRIMARY KEY(name, context));

