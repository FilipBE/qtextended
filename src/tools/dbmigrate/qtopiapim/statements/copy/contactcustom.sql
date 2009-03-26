INSERT INTO contactcustom (recid, fieldname, fieldvalue)
    SELECT convertRecId(recid), fieldname, fieldvalue FROM contactcustom_old;
