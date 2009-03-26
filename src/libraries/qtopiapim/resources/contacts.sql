CREATE TABLE contacts (
	recid INTEGER NOT NULL,
	title NVARCHAR(20) COLLATE NOCASE,
	firstname NVARCHAR(100) COLLATE NOCASE,
	middlename NVARCHAR(100) COLLATE NOCASE,
	lastname NVARCHAR(100) COLLATE NOCASE,
	suffix NVARCHAR(20) COLLATE NOCASE,
	
	default_email NVARCHAR(128) COLLATE NOCASE,
	default_phone VARCHAR(100) COLLATE NOCASE,

        b_webpage NVARCHAR(100) COLLATE NOCASE,

	jobtitle NVARCHAR(100) COLLATE NOCASE,
	department NVARCHAR(100) COLLATE NOCASE,
	company NVARCHAR(100) COLLATE NOCASE,
	office NVARCHAR(100) COLLATE NOCASE,
	profession NVARCHAR(100) COLLATE NOCASE,
	assistant NVARCHAR(100) COLLATE NOCASE,
	manager NVARCHAR(100) COLLATE NOCASE,


        h_webpage NVARCHAR(255) COLLATE NOCASE,

	spouse NVARCHAR(100) COLLATE NOCASE,
	gender NVARCHAR(100) COLLATE NOCASE,
	birthday DATE,
	anniversary DATE,
	nickname NVARCHAR(100) COLLATE NOCASE,
	children NVARCHAR(100) COLLATE NOCASE, -- should be table of relationships?
	
	portrait VARCHAR(100) COLLATE NOCASE,
	
	lastname_pronunciation NVARCHAR(100) COLLATE NOCASE,
	firstname_pronunciation NVARCHAR(100) COLLATE NOCASE,
	company_pronunciation NVARCHAR(100) COLLATE NOCASE,

        context INTEGER NOT NULL,

        label NVARCHAR(255) COLLATE NOCASE,

	PRIMARY KEY(recid)
);

-- Most important index (this gets dropped and recreated when change label format, too)
-- (if this changes, have to change ContactSqlIO::updateLabels
CREATE INDEX contactslabelindex ON contacts(label);

-- two most common sort orderings.  If likely to have other common sort orderings
-- highly recommended to create and maintain indexes.
CREATE INDEX contactsflcindex ON contacts (firstname, lastname, company, recid);
CREATE INDEX contactslfcindex ON contacts (lastname, firstname, company, recid);

CREATE INDEX contactsfindex ON contacts (firstname);
CREATE INDEX contactslindex ON contacts (lastname);
CREATE INDEX contactscindex ON contacts (company);
CREATE INDEX contactscdindex ON contacts (company, department);
