INSERT INTO appointments (recid, description, start, end, allday, repeatrule, repeatfrequency, context) 
    SELECT ((recid & 16777215) | (:birthdaycontext * 16777216)), 
        coalesce(firstname || ' ' || lastname, firstname, lastname, company), 
        birthday || 'T00:00:00', 
        birthday || 'T23:59:00', 
        'true', 
        5, 
        1, 
        :birthdaycontext2 
        FROM contacts WHERE birthday IS NOT NULL;
