INSERT INTO pimdependencies (srcrecid, destrecid, deptype) 
    SELECT recid, 
        ((recid & 16777215) | (:anniversarycontext * 16777216)), 
        'anniversary' 
        FROM contacts WHERE anniversary IS NOT NULL;

