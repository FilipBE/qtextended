INSERT INTO tasks (recid, description, priority, status, percentcompleted, due, started,
        completed, context)
SELECT convertRecId(recid), description, priority, status, percentcompleted, due, started,
       completed, 3 FROM tasks_old;
