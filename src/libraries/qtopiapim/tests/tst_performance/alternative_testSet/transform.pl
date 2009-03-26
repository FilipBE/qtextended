#!/usr/bin/perl

# Reads in a .csv file representing the new columns, returns sql code that will:
#  - Creates appropriate rows in field definition table
#  - imports contacts from 4.4.0 database

my $fieldscsv = shift;
my $actionscsv = shift;
open(FIELDS, "< $fieldscsv") or die "Could not open csv file: $fieldscsv\n";
open(ACTIONS, "< $actionscsv") or die "Could not open csv file: $actionscsv\n";

print "BEGIN TRANSACTION;\n";

print "CREATE TABLE contactFields(fieldId INT PRIMARY KEY, fieldName TEXT, fieldType TEXT, fieldIcon TEXT, inputHint TEXT, importance INT, fieldMax INT, UNIQUE(fieldName));\n";
print "CREATE TABLE contactFieldTags(fieldId INT, fieldTag TEXT, UNIQUE(fieldId, fieldTag), FOREIGN KEY(fieldId) REFERENCES contactFields(fieldId));\n";
print "CREATE TABLE contactTextFields(recordId INT, fieldId INT, fieldValue TEXT, FOREIGN KEY(fieldId) REFERENCES contactFields(fieldId));\n";
print "CREATE TABLE contactIntFields(recordId INT, fieldId INT, fieldValue INT, FOREIGN KEY(fieldId) REFERENCES contactFields(fieldId));\n";
print "CREATE TABLE contactDateFields(recordId INT, fieldId INT, fieldValue DATE, FOREIGN KEY(fieldId) REFERENCES contactFields(fieldId));\n";
print "CREATE TABLE contactAddress(recordId INT, fieldId INT, street TEXT, city TEXT, state TEXT, zip TEXT, country TEXT, FOREIGN KEY(fieldId) REFERENCES contactFields(fieldId));\n";
print "CREATE TABLE contactActions(actionId INT, actionName TEXT, actionIcon TEXT, actionService TEXT, actionRequest TEXT, actionArgs TEXT);\n";
print "CREATE TABLE contactActionTags(actionId INT, actionTag TEXT, UNIQUE(actionId, actionTag), FOREIGN KEY(actionId) REFERENCES contactActions(actionId));\n";
print "CREATE TABLE contactFieldActions(fieldId INT, actionId INT, FOREIGN KEY(fieldId) REFERENCES contactFields(fieldId), FOREIGN KEY(actionId) REFERENCES contactActions(actionId));\n";

my $actionId = 0;
while (<ACTIONS>) {
    chomp;
    my $line = $_;
    if ($actionId > 0) {
        $line =~ s/"//g;
        my @columns = split ":", $line;
        my $actionName = $columns[0];
        my $tagLine = $columns[1];
        $tagLine =~ s/\s//g;
        my @tags = split ",", $tagLine;

        print "INSERT INTO contactActions (actionId, actionName) VALUES ($actionId, '$actionName');\n";
        foreach (@tags) {
            print "INSERT INTO contactActionTags (actionId, actionTag) VALUES ($actionId, '$_');\n";
        }
    }
    $actionId++;
}

my $fieldId = 0;
while (<FIELDS>) {
    chomp;
    my $line = $_;
    if ($fieldId > 0) {

        $line =~ s/"//g;

        my @columns = split ":", $line;
        my $fieldName = $columns[0];
        $filedName =~ s/\s//g;

        my $fieldType = $columns[1];
        my $tagLine = $columns[2];
        $tagLine =~ s/\s//g;
        my @tags = split ",", $tagLine;
        my $actionLine = $columns[3];
        $actionLine =~ s/\s//g;
        my @actions = split ",", $actionLine;
        my $inputHint = $columns[4];
        my $importance = $columns[5];
        my $fieldMax = $columns[6];
        my $sqlImport = $columns[8];

        $inputHint or $inputHint = 'text';
        $fieldMax or $fieldMax = '0';

        print "INSERT INTO contactFields (fieldId, fieldName, fieldType, inputHint, importance, fieldMax) VALUES ($fieldId, '$fieldName', '$fieldType', '$inputHint', $importance, $fieldMax);\n";

        if ($sqlImport) {
            if ($fieldType eq 'text') {
                $sqlImport =~ s/ FROM/, $fieldId FROM/;
                $sqlImport =~ s/ from/, $fieldId FROM/;
                print "INSERT INTO contactTextFields (recordId, fieldValue, fieldId) $sqlImport;\n";
            }
            if ($fieldType eq 'date') {
                $sqlImport =~ s/ FROM/, $fieldId FROM/;
                $sqlImport =~ s/ from/, $fieldId FROM/;
                print "INSERT INTO contactDateFields (recordId, fieldValue, fieldId) $sqlImport;\n";
            }
            if ($fieldType eq 'int') {
                $sqlImport =~ s/ FROM/, $fieldId FROM/;
                $sqlImport =~ s/ from/, $fieldId FROM/;
                print "INSERT INTO contactIntFields (recordId, fieldValue, fieldId) $sqlImport;\n";
            }
        } elsif ($fieldName eq 'address') {
            print "INSERT INTO contactAddress (recordId, fieldId, street, city, state, zip, country) SELECT recid, $fieldId, street, city, state, zip, country from contactaddresses WHERE addresstype = 0;\n";
        } elsif ($fieldName eq 'homeaddress') {
            print "INSERT INTO contactAddress (recordId, fieldId, street, city, state, zip, country) SELECT recid, $fieldId, street, city, state, zip, country from contactaddresses WHERE addresstype = 1;\n";
        } elsif ($fieldName eq 'workaddress') {
            print "INSERT INTO contactAddress (recordId, fieldId, street, city, state, zip, country) SELECT recid, $fieldId, street, city, state, zip, country from contactaddresses WHERE addresstype = 2;\n";
        }
        # child and notes are harder.  child is a split on a text field, notes is reading from non-db source.  In both case C++ functions
        # are probably advisable for actual conversion.
        # other missing 'sqlImport' fields are ones where no previous equivalent field was stored in the database.

        foreach (@tags) {
            print "INSERT INTO contactFieldTags (fieldId, fieldTag) VALUES ($fieldId, '$_');\n";
        }
        foreach (@actions) {
            print "INSERT INTO contactFieldActions (fieldId, actionId) SELECT $fieldId, actionId FROM contactActions WHERE actionName = '$_';\n";
        }
    }
    $fieldId++;
}

close FIELDS;

print "COMMIT TRANSACTION;\n";
