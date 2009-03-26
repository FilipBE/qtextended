CREATE TABLE categories (
        categoryid VARCHAR(255) NOT NULL,
        categorytext NVARCHAR(255),
        categoryscope VARCHAR(255),
        categoryicon VARCHAR(255),
        flags integer,
	PRIMARY KEY(categoryid));

INSERT INTO categories (categoryid, categorytext, flags) VALUES('Business', 'Business', 1);
INSERT INTO categories (categoryid, categorytext, flags) VALUES('Personal', 'Personal', 1);
INSERT INTO categories (categoryid, categorytext, categoryscope, flags) VALUES('SystemRingtones', 'SystemRingtones', 'System', 1);
