INSERT INTO emailaddresses (recid,  addr, label)
    SELECT convertRecId(recid), addr, label FROM emailaddresses_old;
