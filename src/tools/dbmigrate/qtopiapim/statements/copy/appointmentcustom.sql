INSERT INTO appointmentcustom (recid, fieldname, fieldvalue)
    SELECT convertRecId(recid), fieldname, fieldvalue FROM appointmentcustom_old;
