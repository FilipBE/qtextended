INSERT INTO appointments (recid, description, start, end, allday, repeatrule, repeatfrequency, context) 
    SELECT ((recid & 16777215) | (:anniversarycontext * 16777216)), 
        coalesce(firstname || ' ' || lastname, firstname, lastname, company), 
        anniversary || 'T00:00:00', 
        anniversary || 'T23:59:00', 
        'true', 
        5, 
        1, 
        :anniversarycontext2 
        FROM contacts WHERE anniversary IS NOT NULL;

