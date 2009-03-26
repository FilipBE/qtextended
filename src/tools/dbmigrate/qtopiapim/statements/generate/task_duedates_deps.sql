INSERT INTO pimdependencies (srcrecid, destrecid, deptype) 
    SELECT recid, 
        ((recid & 16777215) | (:taskcontext * 16777216)), 
        'duedate' 
        FROM tasks WHERE due IS NOT NULL;

