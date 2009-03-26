INSERT INTO taskcustom (recid, fieldname, fieldvalue)
    SELECT convertRecId(recid), fieldname, fieldvalue FROM taskcustom_old;
