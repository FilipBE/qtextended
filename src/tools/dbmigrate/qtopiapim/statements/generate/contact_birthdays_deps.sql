INSERT INTO pimdependencies (srcrecid, destrecid, deptype) 
    SELECT recid, 
        ((recid & 16777215) | (:birthdaycontext * 16777216)), 
        'birthday' 
        FROM contacts WHERE birthday IS NOT NULL;

