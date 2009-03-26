INSERT INTO appointmentexceptions (recid, edate, alternateid)
    SELECT convertRecId(recid), edate, alternateid FROM appointmentexceptions_old;
