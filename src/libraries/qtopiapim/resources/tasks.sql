CREATE TABLE tasks (
        recid INTEGER NOT NULL,
        description NVARCHAR(255) COLLATE NOCASE,
        priority INTEGER, 
	status INTEGER,
        percentcompleted INTEGER,
        due DATE,
        started DATE,
        completed DATE, 
	context INTEGER NOT NULL,
        PRIMARY KEY(recid));

CREATE INDEX tasks_descrption ON tasks (description, priority, percentcompleted, recid);
CREATE INDEX tasks_completed ON tasks (completed, priority, description, recid);
