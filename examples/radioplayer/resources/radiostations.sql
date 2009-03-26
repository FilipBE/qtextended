
CREATE TABLE radiostations (frequency BIGINT,
                            band VARCHAR(16),
                            frequencydescription VARCHAR(255),
                            name VARCHAR(255),
                            genre VARCHAR(255),
                            region VARCHAR(255),
                            PRIMARY KEY (frequency, region));

CREATE INDEX radiofreqindex ON radiostations (frequency);
CREATE INDEX radioregionindex ON radiostations (region);
