INSERT INTO contactaddresses (recid, addresstype, street, city, state, zip, country)
    SELECT convertRecId(recid), addresstype, street, city, state, zip, country FROM contactaddresses_old;
