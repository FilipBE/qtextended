INSERT INTO taskcategories (recid, categoryid)
    SELECT convertRecId(recid), categoryid FROM taskcategories_old;
