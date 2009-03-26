CREATE TABLE mimeTypeMapping (
        application STRING NOT NULL,
        mimeType STRING NOT NULL,
        mimeSubType STRING NOT NULL,
        icon STRING NOT NULL,
        drmFlags INTEGER,
        PRIMARY KEY(application, mimeType, mimeSubType)
);

CREATE INDEX mimeTypeMappingmimeTypeIndex on mimeTypeMapping(mimeType);
CREATE INDEX mimeTypeMappingmimeTypeSubIndex on mimeTypeMapping(mimeSubType);

CREATE TABLE defaultMimeApplication (
        mimeType STRING NOT NULL,
        mimeSubType STRING NOT NULL,
        application STRING NOT NULL,
        PRIMARY KEY(mimeType, mimeSubType)
);
