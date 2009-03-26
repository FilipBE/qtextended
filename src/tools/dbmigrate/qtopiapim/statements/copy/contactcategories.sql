INSERT INTO contactcategories (recid, categoryid)
    SELECT convertRecId(recid), categoryid FROM contactcategories_old;
