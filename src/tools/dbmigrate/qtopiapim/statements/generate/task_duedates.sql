INSERT INTO appointments (recid, description, start, end, allday, context) 
    SELECT ((recid & 16777215) | (:taskcontext * 16777216)), 
        description, 
        due || 'T00:00:00', 
        due || 'T23:59:00', 
        'true', 
        :taskcontext2 
        FROM tasks WHERE due IS NOT NULL;
