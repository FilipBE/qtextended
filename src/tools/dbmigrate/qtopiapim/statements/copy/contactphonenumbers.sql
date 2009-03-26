INSERT INTO contactphonenumbers (recid, phone_number, phone_type)
    SELECT convertRecId(recid), phone_number, phone_type FROM contactphonenumbers_old;
