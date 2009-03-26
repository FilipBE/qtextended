INSERT INTO contacts (recid, title, firstname, middlename, lastname, suffix,
        default_email, default_phone, b_webpage, jobtitle, department, company, office,
        profession, assistant, manager, h_webpage, spouse, gender, birthday, anniversary,
        nickname, children, portrait, lastname_pronunciation, firstname_pronunciation,
        company_pronunciation, context)
SELECT convertRecId(recid), title, firstname, middlename, lastname, suffix,
       default_email, default_phone, b_webpage, jobtitle, department, company, office,
       profession, assistant, manager, h_webpage, spouse, gender, birthday, anniversary,
       nickname, children, portrait, lastname_pronunciation, firstname_pronunciation,
       company_pronunciation, 1 FROM contacts_old;
