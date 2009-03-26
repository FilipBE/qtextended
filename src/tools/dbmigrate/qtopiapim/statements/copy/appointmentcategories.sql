INSERT INTO appointmentcategories (recid, categoryid)
    SELECT convertRecId(recid), categoryid FROM appointmentcategories_old;
