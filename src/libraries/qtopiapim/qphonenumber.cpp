/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include <qphonenumber.h>
#include <QDebug>

/*!
    \class QPhoneNumber
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

    \ingroup pim
    \brief The QPhoneNumber class provides utility functions to manipulate telephone numbers.

    The functions provided by this class are largely intended to assist with
    the dialing (and "quick dialing") experience, and include:
    \list
    \o stripping a number of any non-telephony characters
    \o returning the "local" component (non area/country code) of a telephone number
    \o checking to see how closely two telephone numbers match
    \o testing whether a phone number starts with a supplied prefix
    \o converting an alphabetic telephone number into the equivalent numeric number
    \endlist


  \sa {Pim Library}
*/


/*
   Helper function for deciding if something is a URL (does it have a protocol, mostly)
*/
static bool isNumberURL(const QString& number)
{
    // Does this look like a URL?
    int colonIdx = number.indexOf(':');
    // If it isn't the first or last character, guess that it is a URI
    // (we don't create a QUrl here)
    if (colonIdx > 0 && colonIdx < number.length() - 1)
        return true;
    return false;
}

/*!
    Strip the given \a number down to remove non-digit and non-dialing characters.
    If \a number seems to be a URI, this function will return the original value.
*/
QString QPhoneNumber::stripNumber( const QString& number )
{
    if (isNumberURL(number))
        return number; // XXX we could strip non RFC compliant characters, I guess

    QString n = "";
    int posn;
    uint ch;
    for ( posn = 0; posn < number.length(); ++posn ) {
        ch = number[posn].unicode();
        if ( ch >= '0' && ch <= '9' ) {
            n += (QChar)ch;
        } else if ( ch == '+' || ch == '#' || ch == '*' ) {
            n += (QChar)ch;
        } else if ( ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' ) {
            // ABCD can actually be digits!
            n += (QChar)ch;
        } else if ( ch == 'a' || ch == 'b' || ch == 'c' || ch == 'd' ) {
            n += (QChar)( ch - 'a' + 'A' );
        } else if ( ch == ',' || ch == 'p' || ch == 'P' || ch == 'X' || ch == 'x' ) {
            // Comma and 'p' mean a short pause.
            // 'x' means an extension, which for now is the same as a pause.
            n += (QChar)',';
        } else if ( ch == 'w' || ch == 'W' ) {
            // 'w' means wait for dial tone.
            n += (QChar)'W';
        } else if ( ch == '!' || ch == '@' ) {
            // '!' = hook flash, '@' = wait for silence.
            n += (QChar)ch;
        }
    }
    return n;
}


// Rule information for international prefixes.
typedef struct
{
    const char *country;        // Country the rules apply to.
    int internationalCode;      // International prefix for country.
    char areaCodeLength;        // Length of local area codes.
    char areaCodeLengthAlt;     // Alternative length, or zero.
    signed char areaCodePrefix; // Local area code prefix, or -1 if none.
    char areaCodePrefixLength;  // Length of the area code prefix.

} IDDRule;

// This list was derived from a combination of the codes in the
// back of the Telstra White Pages, and the local area code prefixes
// at "http://kropla.com/dialcode.htm". As a list of facts, they are
// not copyrightable.


static IDDRule const rules[] = {
    {"Afghanistan",              93,    2,  0,   0,   1},
    {"Albania",                 355,    2,  3,   0,   1},
    {"Algeria",                 213,    2,  0,   7,   1},
    {"Andorra",                 376,    0,  0,  -1,   1},
    {"Angola",                  244,    1,  0,   0,   1},
    {"Anguilla",               1264,    0,  0,   1,   1},
    {"Antarctica",              672,    2,  0,  -1,   1},
    {"Antigua and Barbuda",    1268,    0,  0,   1,   1},
    {"Argentina",                54,    3,  2,   0,   1},
    {"Armenia",                 374,    1,  0,   8,   1},
    {"Aruba",                   297,    0,  0,  -1,   1},
    {"Ascension Island",        247,    0,  0,  -1,   1},
    {"Australia",                61,    1,  0,   0,   1},
    {"Austria",                  43,    3,  1,   0,   1},
    {"Azerbaijan",              994,    2,  0,   8,   1},

    {"Bahamas",                1242,    0,  0,   1,   1},
    {"Bahrain",                 973,    0,  0,  -1,   1},
    {"Bangladesh",              880,    2,  1,   0,   1},
    {"Barbados",               1246,    0,  0,   1,   1},
    {"Belarus",                 375,    3,  2,   8,   1},
    {"Belgium",                  32,    2,  1,   0,   1},
    {"Belize",                  501,    0,  0,   0,   1},
    {"Benin",                   229,    0,  0,  -1,   1},
    {"Bermuda",                1441,    0,  0,   1,   1},
    {"Bhutan",                  975,    0,  0,  -1,   1},
    {"Bolivia",                 591,    1,  0,   0,   1},
    {"Bosnia Herzegovina",      387,    2,  0,   0,   1},
    {"Botswana",                267,    0,  0,  -1,   1},
    {"Brazil",                   55,    2,  0,   0,   1},
    {"Brunei",                  673,    0,  0,   0,   1},
    {"Bulgaria",                359,    2,  1,   0,   1},
    {"Burkina Faso",            226,    0,  0,  -1,   1},
    {"Burundi",                 257,    0,  0,  -1,   1},

    {"Cambodia",                855,    2,  0,   0,   1},
    {"Cameroon",                237,    0,  0,  -1,   1},
    {"Cape Verde Islands",      238,    0,  0,  -1,   1},
    {"Cayman Islands",         1345,    0,  0,   1,   1},
    {"Central African Republic",236,    0,  0,   0,   1},
    {"Chad",                    235,    0,  0,  -1,   1},
    {"Chile",                    56,    2,  1,   0,   1},
    {"China",                    86,    2,  0,   0,   1},
    {"Columbia",                 57,    1,  0,   0,   1},
    {"Comoros",                 269,    0,  0,  -1,   1},
    {"Congo",                   242,    0,  0,  -1,   1},
    {"Cook Islands",            682,    0,  0,   0,   2},   // prefix = 00
    {"Costa Rica",              506,    0,  0,  -1,   1},
    {"Croatia",                 385,    2,  1,   0,   1},
    {"Cuba",                     53,    2,  1,   0,   1},
    {"Cyprus",                  357,    0,  0,  -1,   1},
    {"Cyprus (Turkish)",      90392,    0,  0,  -1,   1},
    {"Czech Republic",          420,    0,  0,   0,   1},

    {"Democratic Republic of Congo",
                                243,    1,  0,  -1,   1},
    {"Denmark",                  45,    0,  0,  -1,   1},
    {"Diego Garcia",            246,    0,  0,  -1,   1},
    {"Djibouti",                253,    0,  0,  -1,   1},
    {"Dominica",               1767,    0,  0,   1,   1},
    {"Dominican Republic",     1809,    0,  0,   1,   1},

    {"East Timor",              670,    3,  0,  -1,   1},
    {"Ecuador",                 593,    1,  0,   0,   1},
    {"Egypt",                    20,    2,  1,   0,   1},
    {"El Salvador",             503,    0,  0,  -1,   1},
    {"Equatorial Guinea",       240,    1,  0,  -1,   1},
    {"Eritrea",                 291,    1,  0,   0,   1},
    {"Estonia",                 372,    2,  1,   0,   1},
    {"Ethiopia",                251,    1,  0,   0,   1},

    {"Falkland Islands",        500,    0,  0,  -1,   1},
    {"Faroe Islands",           298,    0,  0,  -1,   1},
    {"Fiji",                    679,    0,  0,   0,   1},
    {"Finland",                 358,    1,  0,   0,   1},
    {"France",                   33,    1,  0,   0,   1},
    {"French Guiana",           594,    0,  0,  -1,   1},
    {"French Polynesia",        689,    0,  0,  -1,   1},

    {"Gabon",                   241,    0,  0,  -1,   1},
    {"Gambia",                  220,    0,  0,  -1,   1},
    {"Georgia",                 995,    2,  0,   8,   1},
    {"Germany",                  49,    3,  2,   0,   1},
    {"Ghana",                   233,    2,  0,  -1,   1},
    {"Gibraltar",               350,    0,  0,  -1,   1},
    {"Greece",                   30,    4,  2,  -1,   1},
    {"Greenland",               299,    0,  0,  -1,   1},
    {"Grenada",                1473,    0,  0,   1,   1},
    {"Guadeloupe",              590,    0,  0,  -1,   1},
    {"Guam",                   1671,    0,  0,   1,   1},
    {"Guantanamo Bay",         5399,    0,  0,   0,   1},
    {"Guatemala",               502,    0,  0,   0,   1},
    {"Guinea Bissau",           245,    0,  0,  -1,   1},
    {"Guinea Republic",         224,    0,  0,   0,   1},
    {"Guyana",                  592,    0,  0,   0,   1},

    {"Haiti",                   509,    0,  0,   0,   1},
    {"Honduras",                504,    0,  0,   0,   1},
    {"Hong Kong",               852,    0,  0,  -1,   1},
    {"Hungary",                  36,    2,  1,   6,   2},   // prefix = 06

    {"Iceland",                 354,    0,  0,   0,   1},
    {"India",                    91,    3,  2,   0,   1},
    {"Indonesia",                62,    3,  2,   0,   1},
    {"Inmarsat",                 87,    1,  0,  -1,   1},
    {"Iran",                     98,    3,  2,   0,   1},
    {"Iraq",                    964,    2,  1,   0,   1},
    {"Ireland",                 353,    2,  1,   0,   1},
    {"Israel",                  972,    1,  0,   0,   1},
    {"Italy",                    39,    0,  0,  -1,   1},
    {"Ivory Coast",             225,    0,  0,   0,   1},

    {"Jamaica",                1876,    0,  0,   1,   1},
    {"Japan",                    81,    2,  1,   0,   1},
    {"Jordan",                  962,    1,  0,   0,   1},

    {"Kazakstan",                 7,    5,  4,   8,   1},
    {"Kenya",                   254,    2,  1,   0,   1},
    {"Kiribati",                686,    0,  0,   0,   1},
    {"Korea (North)",           850,    1,  0,   0,   1},
    {"Korea (South)",            82,    2,  1,   0,   1},
    {"Kuwait",                  965,    0,  0,   0,   1},
    {"Kyrgyzstan",              996,    3,  0,   0,   1},

    {"Laos",                    856,    2,  0,   0,   1},
    {"Latvia",                  371,    0,  0,   8,   1},
    {"Lebanon",                 961,    1,  0,   0,   1},
    {"Lesotho",                 266,    0,  0,   0,   1},
    {"Liberia",                 231,    0,  0,  22,   2},   // prefix = 22
    {"Libya",                   218,    2,  0,   0,   1},
    {"Liechtenstein",           423,    0,  0,  -1,   1},
    {"Lithuania",               370,    2,  1,   8,   1},
    {"Luxemborg",               352,    0,  0,  -1,   1},

    {"Macau",                   853,    0,  0,   0,   1},
    {"Macedonia (FYROM)",       389,    2,  1,   0,   1},
    {"Madagascar",            26120,    0,  0,   0,   1},
    {"Malawi",                  265,    0,  0,  -1,   1},
    {"Malaysia",                 60,    1,  0,   0,   1},
    {"Maldives",                960,    0,  0,   0,   1},
    {"Mali",                    223,    0,  0,   0,   1},
    {"Malta",                   356,    0,  0,   0,   1},
    {"Mariana Islands",        1670,    0,  0,   1,   1},
    {"Marshall Islands",        692,    0,  0,   1,   1},
    {"Martinique",              596,    0,  0,   0,   1},
    {"Mauritania",              222,    0,  0,   0,   1},
    {"Mauritius",               230,    0,  0,   0,   1},
    {"Mayotte Island",          269,    0,  0,  -1,   1},
    {"Mexico",                   52,    3,  2,   1,   2},   // prefix = 01
    {"Micronesia",              691,    0,  0,   1,   1},
    {"Moldova",                 373,    2,  1,   8,   1},
    {"Monaco",                  377,    0,  0,   0,   1},
    {"Mongolia",                976,    2,  1,   0,   1},
    {"Montserrat",             1664,    0,  0,   1,   1},
    {"Morocco",                 212,    1,  0,  -1,   1},
    {"Mozambique",              258,    2,  1,   0,   1},
    {"Myanmar",                  95,    2,  1,  -1,   1},

    {"Namibia",                 264,    2,  0,   0,   1},
    {"Nauru",                   674,    0,  0,   0,   1},
    {"Nepal",                   977,    1,  0,   0,   1},
    {"Netherlands",              31,    2,  0,   0,   1},
    {"Netherlands Antilles",    599,    1,  0,   0,   1},
    {"New Caledonia",           687,    0,  0,   0,   1},
    {"New Zealand",              64,    1,  0,   0,   1},
    {"Nicaragua",               505,    0,  0,   0,   1},
    {"Niger",                   227,    0,  0,   0,   1},
    {"Nigeria",                 234,    2,  1,   0,   1},
    {"Niue",                    683,    0,  0,   0,   1},
    {"Norfolk Island",          672,    1,  0,  -1,   1},
    {"Norway",                   47,    0,  0,  -1,   1},

    {"Oman",                    968,    0,  0,   0,   1},

    {"Pakistan",                 92,    3,  2,   0,   1},
    {"Palau",                   680,    0,  0,  -1,   1},
    {"Palestine",               970,    1,  0,   0,   1},
    {"Panama",                  507,    0,  0,   0,   1},
    {"Papua New Guinea",        675,    0,  0,  -1,   1},
    {"Paraguay",                595,    2,  0,   0,   1},
    {"Peru",                     51,    2,  1,   0,   1},
    {"Philipines",               63,    2,  1,   0,   1},
    {"Poland",                   48,    2,  0,   0,   1},
    {"Portugal",                351,    3,  2,  -1,   1},
    {"Puerto Rico (787)",      1787,    0,  0,   1,   1},
    {"Puerto Rico (939)",      1939,    0,  0,   1,   1},

    {"Qatar",                   974,    0,  0,   0,   1},

    {"Reunion Island",          262,    0,  0,   0,   1},
    {"Romania",                  40,    3,  2,   0,   1},
    {"Russia",                    7,    3,  0,   8,   1},
    {"Rwanda",                  250,    0,  0,   0,   1},

    {"Samoa (American)",        684,    0,  0,  -1,   1},
    {"Samoa",                   685,    0,  0,  -1,   1},
    {"San Marino",              378,    0,  0,   0,   1},
    {"Sao Tome and Principe",   239,    0,  0,   0,   1},
    {"Satellite Phones",        881,    1,  0,  -1,   1},
    {"Saudi Arabia",            966,    1,  0,   0,   1},
    {"Senegal",                 221,    0,  0,   0,   1},
    {"Serbia and Montenegro",   381,    2,  0,   0,   1},
    {"Seychelles Islands",      248,    0,  0,   0,   1},
    {"Sierra Leone",            232,    2,  0,   0,   1},
    {"Singapore",                65,    0,  0,  -1,   1},
    {"Slovak Republic",         421,    2,  1,   0,   1},
    {"Slovenia",                386,    1,  0,   0,   1},
    {"Solomon Islands",         677,    0,  0,  -1,   1},
    {"Somalia",                 252,    1,  0,  -1,   1},
    {"South Africa",             27,    2,  0,   0,   1},
    {"Spain",                    34,    3,  2,  -1,   1},
    {"Sri Lanka",                94,    3,  1,   0,   1},
    {"St. Helena",              290,    0,  0,  -1,   1},
    {"St. Kitts & Nevis",      1869,    0,  0,   1,   1},
    {"St. Lucia",              1758,    0,  0,   1,   1},
    {"St. Pierre & Miquelon",   508,    0,  0,   0,   1},
    {"St. Vincent & Grenadines",1784,   0,  0,   1,   1},
    {"Sudan",                   249,    2,  0,   0,   1},
    {"Suriname",                597,    0,  0,  -1,   1},
    {"Swaziland",               268,    0,  0,  -1,   1},
    {"Sweden",                   46,    2,  1,   0,   1},
    {"Switzerland",              41,    2,  1,   0,   1},
    {"Syria",                   963,    2,  0,   0,   1},

    {"Taiwan",                  886,    1,  0,  -1,   1},
    {"Tajikistan",              992,    2,  0,   8,   1},
    {"Tanzania",                255,    2,  0,   0,   1},
    {"Thailand",                 66,    2,  1,   0,   1},
    {"Thuraya",                 882,    2,  0,  -1,   1},
    {"Togo",                    228,    0,  0,  -1,   1},
    {"Tokelau",                 690,    0,  0,  -1,   1},
    {"Tonga",                   676,    0,  0,  -1,   1},
    {"Trinidad & Tobago",      1868,    0,  0,   1,   1},
    {"Tunisia",                 216,    0,  0,   0,   1},
    {"Turkey",                   90,    3,  0,   0,   1},
    {"Turkmenistan",            993,    2,  0,   8,   1},
    {"Turks & Caicos Islands", 1649,    0,  0,   1,   1},
    {"Tuvalu",                  688,    0,  0,  -1,   1},

    {"Uganda",                  256,    2,  0,   0,   1},
    {"Ukraine",                 380,    3,  2,   8,   1},
    {"United Arab Emirates",    971,    0,  0,  -1,   1},
    {"United Kingdom",           44,    3,  2,   0,   1},
    {"United States & Canada",    1,    3,  0,   1,   1},
    {"Uruguay",                 598,    2,  1,   0,   1},
    {"Uzbekistan",              998,    2,  0,   8,   1},

    {"Vanuata",                 678,    0,  0,  -1,   1},
    {"Vatican City",             39,    0,  0,  -1,   1},
    {"Venezuela",                58,    3,  0,   0,   1},
    {"Vietnam",                  84,    1,  0,   0,   1},
    {"Virgin Islands (British)",1284,   0,  0,   1,   1},
    {"Virgin Islands (U.S.)",  1340,    0,  0,   1,   1},

    {"Wake Island",             808,    0,  0,  -1,   1},
    {"Wallis & Futuna Islands", 681,    0,  0,  -1,   1},

    {"Yemen",                   967,    1,  0,   0,   1},

    {"Zambia",                  260,    1,  0,   0,   1},
    {"Zimbabwe",                263,    2,  1,   0,   1},
};
static int const numRules = sizeof(rules) / sizeof(IDDRule);

// Default rules to use for numbers that start with '0', or other.
static IDDRule defaultZeroRule  = {0, 0, 1, 0, 0, 1};
static IDDRule defaultOtherRule = {0, 0, 0, 0, -1, 1};


// Get the length of an international dialing prefix.
static uint prefixLength( const IDDRule *rule )
{
    int code = rule->internationalCode;
    if ( code < 10 )
        return 1;
    else if ( code < 100 )
        return 2;
    else if ( code < 1000 )
        return 3;
    else if ( code < 10000 )
        return 4;
    else
        return 5;
}

// Determine if a number starts with a particular prefix.
static bool numberStartsWith( const QString& number, int prefix, int length )
{
    if ( number.length() < length )
        return false;
    while ( length > 0 ) {
        --length;
        if ( number[length].unicode() != ( '0' + (prefix % 10) ) ) {
            return false;
        }
        prefix /= 10;
    }
    return true;
}


// Split a phone number into its IDD prefix, area code, and local number.
// Returns the IDDRule used, if any.
static const IDDRule *splitNumber( const QString& _number,
                         QString& countryCode,
                         QString& areaCode,
                         QString& localNumber,
                         const IDDRule *localRule,
                         bool useAltRule )
{
    QString number = _number;
    const IDDRule *rule;
    int index, length;
    bool implicitAreaPrefix;

    // Clear the return values.
    countryCode = QString();
    areaCode = QString();
    localNumber = QString();

    // Find the country-specific rule that applies.
    implicitAreaPrefix = false;
    // 00 is the most common international access code, apparently.
    if ( number[0] == '+'  || number.startsWith("00")) {
        rule = 0;
        if (number[0] == '+')
            number = number.mid(1);
        else
            number = number.mid(2);
        for ( index = 0; index < numRules; ++index ) {
            const IDDRule *rule2 = &( rules[ index ] );
            if ( numberStartsWith( number, rule2->internationalCode,
                                   prefixLength( rule2 ) ) ) {
                if ( !rule || rule2->internationalCode >
                                    rule->internationalCode ) {
                    rule = rule2;
                }
            }
        }
        if ( rule ) {
            countryCode = number.mid( 0, prefixLength(rule) );
            number = number.mid( prefixLength(rule) );
        } else {
            rule = &defaultOtherRule;
        }
        implicitAreaPrefix = true;
    } else if ( localRule ) {
        // Use the supplied local phone number rule.
        rule = localRule;
    } else if ( number[0] == '0' ) {
        // Use a default rule for locales that use '0' to prefix area codes.
        rule = &defaultZeroRule;
    } else {
        // Don't return any kind of area code.
        rule = &defaultOtherRule;
    }

    // Extract the area code, if present.
    length = -1;
    if ( rule->areaCodePrefix != -1 ) {
        if ( implicitAreaPrefix ) {
            if ( useAltRule && rule->areaCodeLengthAlt != 0 ) {
                length = rule->areaCodeLengthAlt;
            } else {
                length = rule->areaCodeLength;
            }
        } else {
            if ( numberStartsWith( number, rule->areaCodePrefix,
                                   rule->areaCodePrefixLength ) ) {
                number = number.mid( (uint)(int)(rule->areaCodePrefixLength) );
                if ( useAltRule && rule->areaCodeLengthAlt != 0 ) {
                    length = rule->areaCodeLengthAlt;
                } else {
                    length = rule->areaCodeLength;
                }
            }
        }
    }
    if ( length > 0 && length < (int)( number.length() ) ) {
        areaCode = number.left( length );
        number = number.mid( length );
    }

    // Whatever remains must be the local number.
    localNumber = number;

    return rule;
}

/*!
  Returns the local component of the phone \a number.  In the
  case of countries with variable length area codes this may
  only return the end of the local number.  In the case of
  URLs (for example 'sip:user@voipserver'), the original
  URL is returned.
*/
QString QPhoneNumber::localNumber( const QString &number)
{
    if (isNumberURL(number))
        return number;

    QString n, c, a, l1, l2;

    n = stripNumber(number);

    splitNumber( n, c, a, l1, 0, false );
    splitNumber( n, c, a, l2, 0, true );
    if (l1.length() < l2.length() && !l1.isEmpty())
        return l1;
    return l2;
}

/*!
    Compare two phone numbers, \a num1 and \a num2, to determine the degree
    to which they match.  Returns zero for no match, or increasing positive
    values for the "quality" of the match between the values.

    Both numbers should be in numeric (not alphanumeric) form, unless they
    are URLs.  URLs are just compared strictly for equality.  If one argument
    appears to be a URL, and the other does not appear to be one, then the
    non protocol part of the URL argument will be compared strictly against the
    other argument.

    \sa resolveLetters()
*/

int QPhoneNumber::matchNumbers( const QString& num1, const QString& num2 )
{
    bool is1Url = isNumberURL(num1);
    bool is2Url = isNumberURL(num2);

    if (is1Url && is2Url) {
        if (num1 == num2)
            return 100;
        else
            return 0;
    } else if (is1Url || is2Url) {
        // Strip off the protocol part
        QString u1 = is1Url ? num1.mid(num1.indexOf(':') + 1) : num1;
        QString u2 = is2Url ? num2.mid(num2.indexOf(':') + 1) : num2;

        if (u1 == u2)
            return 5;
        else
            return 0;
    }

    // Non URL case
    QString n1 = stripNumber( num1 );
    QString n2 = stripNumber( num2 );
    if (n1[0] != '+' && n2[0] == '+')
        qSwap(n1,n2); // if any has country, first does
    QString countryCode1;
    QString areaCode1;
    QString localNumber1;
    QString countryCode2;
    QString areaCode2;
    QString localNumber2;
    int quality;

    // Bail out if either string is empty.
    if ( n1.length() == 0 || n2.length() == 0 ) {
        return 0;
    }

    // If the two numbers are equal, then report a high-quality match.
    if ( n1 == n2 ) {
        return 100;
    }

    // Split the numbers into their constituent components.
    const IDDRule* rule = splitNumber( n1, countryCode1, areaCode1, localNumber1, 0, false );
    splitNumber( n2, countryCode2, areaCode2, localNumber2, rule, false );

    // Compare the two numbers.
    quality = 0;
    if ( localNumber1 == localNumber2 ) {
        quality = 1;
        if ( areaCode1 == areaCode2 ) {
            ++quality;
        } else if ( areaCode1 != QString() && areaCode2 != QString() ) {
            quality = 0;
        }
        if ( countryCode1 == countryCode2 ) {
            if ( quality > 0 )
                ++quality;
        } else if ( countryCode1 != QString() &&
                    countryCode2 != QString() ) {
            quality = 0;
        }
    }
    if ( quality > 0 ) {
        return quality;
    }

    // Split the numbers again, using the alternative rule.
    rule = splitNumber( n1, countryCode1, areaCode1, localNumber1, 0, true );
    splitNumber( n2, countryCode2, areaCode2, localNumber2, rule = 0, true );

    // Compare the two numbers.
    quality = 0;
    if ( localNumber1 == localNumber2 ) {
        quality = 1;
        if ( areaCode1 == areaCode2 ) {
            ++quality;
        } else if ( areaCode1 != QString() && areaCode2 != QString() ) {
            quality = 0;
        }
        if ( countryCode1 == countryCode2 ) {
            if ( quality > 0 )
                ++quality;
        } else if ( countryCode1 != QString() &&
                    countryCode2 != QString() ) {
            quality = 0;
        }
    }
    if ( quality > 0 ) {
        return quality;
    }

    // There is no match between the numbers.
    return 0;
}

/*!
    Returns whether the given number \a num appears to start with the given \a prefix.

    For a URL argument \a num, if the \a prefix argument is also a URL, a strict prefix
    match will be performed.  If only one of the arguments is a URL, the non protocol part
    of the URL will be strictly against the other argument.

    For non URL arguments, the \a prefix will be tested against \a num with and without
    area code or international prefixes.  Both numbers should be in numeric (not alphanumeric) form.

    \sa resolveLetters()
*/

bool QPhoneNumber::matchPrefix( const QString& num, const QString& prefix )
{
    bool is1Url = isNumberURL(num);
    bool is2Url = isNumberURL(prefix);

    if (is1Url && is2Url) {
        return num.startsWith(prefix);
    } else if (is1Url || is2Url) {
        // Strip off the protocol part
        QString u1 = is1Url ? num.mid(num.indexOf(':') + 1) : num;
        QString u2 = is2Url ? prefix.mid(prefix.indexOf(':') + 1) : prefix;

        return u1.startsWith(u2);
    }

    // Non URL case
    QString n = stripNumber( num );
    QString p = stripNumber( prefix );
    QString countryCode1;
    QString areaCode1;
    QString localNumber1;
    QString countryCode2;
    QString areaCode2;
    QString localNumber2;

    // Bail out early if we have a direct prefix match.
    if ( n.length() == 0 || p.length() == 0 )
        return false;
    if ( n.startsWith( p ) )
        return true;

    // Pull the number apart into its components.
    splitNumber( n, countryCode1, areaCode1, localNumber1, 0, false );
    splitNumber( n, countryCode2, areaCode2, localNumber2, 0, true );

    // See if the local part of the number starts with the prefix.
    if ( localNumber1.startsWith( p ) )
        return true;
    if ( localNumber2.startsWith( p ) )
        return true;

    // Join the area code and local number together and recheck.
    localNumber1 = areaCode1 + localNumber1;
    localNumber2 = areaCode2 + localNumber2;
    if ( localNumber1.startsWith( p ) )
        return true;
    if ( localNumber2.startsWith( p ) )
        return true;

    // Strip a leading 0 or 1 area code specifier from the prefix and retry.
    if ( p[0] == '0' || p[0] == '1' ) {
        p = p.mid(1);
        if ( localNumber1.startsWith( p ) )
            return true;
        if ( localNumber2.startsWith( p ) )
            return true;
    }

    // No prefix match if we get here.
    return false;
}

/*!
    Convert an alphanumeric phone \a number into a regular phone number.
    e.g. \c{1-800-HEY-PHIL} becomes \c{1-800-439-7445}.

    If a URL is passed to this function (e.g. 'sip:user@sipserver'), the
    original value is returned.

    Certain letters are not converted into numbers, because they
    can be used to control the way a number is dialed.  These letters
    are lower case \c p, \c w and \c x, which insert pauses
    or waits when dialing a number.  Upper case letters are always
    converted into the corresponding numbers.

    Note: this is the only function in the API that can take an
    alphanumeric phone number as an argument.  All other functions
    assume that the number has already been resolved.
*/

QString QPhoneNumber::resolveLetters( const QString& number )
{
    if (isNumberURL(number))
        return number;
    static char const letterDigits[] = "22233344455566677778889999";
    QString n = "";
    int posn;
    uint ch;
    for ( posn = 0; posn < number.length(); ++posn ) {
        ch = number[posn].unicode();
        if ( ch >= 'A' && ch <= 'Z' ) {
            n += (QChar)( letterDigits[ch - 'A'] );
        } else if ( ch >= 'a' && ch <= 'z' && ch != 'p' && ch != 'w' && ch != 'x' ) {
            // Note: lower case 'p', 'w', and 'x' are not converted because they
            // are probably "pause" and "wait for dial tone".  Upper case
            // 'P' and 'W' are assumed to be letterized digits.
            n += (QChar)( letterDigits[ch - 'a'] );
        } else {
            n += (QChar)ch;
        }
    }
    return n;
}
