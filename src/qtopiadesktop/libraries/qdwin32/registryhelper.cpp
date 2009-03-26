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
#include <registryhelper.h>
#include <typeconversion.h>

#include <qdebug.h>

using namespace QDWIN32;

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
#define MAX_DATA_SIZE 65535

QString QDWIN32::readRegKey( HKEY hive, const QString &path, const QString &key )
{
    HKEY hKey;
    if ( RegOpenKeyEx( hive, qstring_to_tchar(path), 0, KEY_READ, &hKey ) != ERROR_SUCCESS ) {
        qWarning() << "Could not open" << path;
        return QString();
    }
    QString ret;

    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 

    DWORD i, retCode; 

    TCHAR  achValue[MAX_VALUE_NAME]; 
    DWORD cchValue = MAX_VALUE_NAME; 
    DWORD dwType;
    BYTE  bData[MAX_DATA_SIZE];
    DWORD cbData;

    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
            hKey,                    // key handle 
            achClass,                // buffer for class name 
            &cchClassName,           // size of class string 
            NULL,                    // reserved 
            &cSubKeys,               // number of subkeys 
            &cbMaxSubKey,            // longest subkey size 
            &cchMaxClass,            // longest class string 
            &cValues,                // number of values for this key 
            &cchMaxValue,            // longest value name 
            &cbMaxValueData,         // longest value data 
            &cbSecurityDescriptor,   // security descriptor 
            &ftLastWriteTime);       // last write time 

    // Enumerate the key values. 

    if (cValues) {
        for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) { 
            cchValue = MAX_VALUE_NAME; 
            achValue[0] = '\0'; 
            cbData = MAX_DATA_SIZE;
            memset(&bData, 0, MAX_DATA_SIZE);
            retCode = RegEnumValue( hKey, i,
                    achValue,
                    &cchValue,
                    NULL,
                    &dwType,
                    (LPBYTE)&bData,
                    &cbData);
            if (retCode == ERROR_SUCCESS && dwType == REG_SZ) { 
                if ( tchar_to_qstring(achValue, cchValue) == key ) {
                    ret = tchar_to_qstring( (TCHAR*)bData, (cbData/sizeof(TCHAR))-1 );
                    break;
                }
            }
        }
    }

    RegCloseKey( hKey );

    return ret;
}

QString QDWIN32::findSubKey( HKEY hive, const QString &path, const QString &key )
{
    HKEY hKey;
    if ( RegOpenKeyEx( hive, qstring_to_tchar(path), 0, KEY_READ, &hKey ) != ERROR_SUCCESS ) {
        qWarning() << "Could not open" << path;
        return QString();
    }
    QString ret;

    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 

    DWORD i, retCode; 

    TCHAR  achValue[MAX_VALUE_NAME]; 
    DWORD cchValue = MAX_VALUE_NAME; 

    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
            hKey,                    // key handle 
            achClass,                // buffer for class name 
            &cchClassName,           // size of class string 
            NULL,                    // reserved 
            &cSubKeys,               // number of subkeys 
            &cbMaxSubKey,            // longest subkey size 
            &cchMaxClass,            // longest class string 
            &cValues,                // number of values for this key 
            &cchMaxValue,            // longest value name 
            &cbMaxValueData,         // longest value data 
            &cbSecurityDescriptor,   // security descriptor 
            &ftLastWriteTime);       // last write time 

    // Enumerate the subkeys

    if (cSubKeys) {
        for (i=0; i<cSubKeys; i++) { 
            achValue[0] = '\0'; 
            retCode = RegEnumKey(hKey, i,
                     achValue, 
                     MAX_VALUE_NAME);
            if (retCode == ERROR_SUCCESS) {
                //_tprintf(TEXT("(%d) %s\n"), i+1, achKey);
                ret = tchar_to_qstring( (TCHAR*)achValue, lstrlen((TCHAR*)achValue) );
                break;
            }
        }
    }

    RegCloseKey( hKey );

    return ret;
}

HKEY QDWIN32::openRegKey( HKEY hive, const QString &path )
{
    HKEY ret = 0;
    if ( RegOpenKeyEx( hive, qstring_to_tchar(path), 0, KEY_READ, &ret ) != ERROR_SUCCESS )
        ret = 0;
    return ret;
}

QStringList QDWIN32::readRegKeys( HKEY hKey )
{
    QStringList keys;

    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 

    DWORD i, retCode; 

    TCHAR  achValue[MAX_VALUE_NAME]; 
    DWORD cchValue = MAX_VALUE_NAME; 
    DWORD dwType;
    BYTE  bData[MAX_DATA_SIZE];
    DWORD cbData;

    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
            hKey,                    // key handle 
            achClass,                // buffer for class name 
            &cchClassName,           // size of class string 
            NULL,                    // reserved 
            &cSubKeys,               // number of subkeys 
            &cbMaxSubKey,            // longest subkey size 
            &cchMaxClass,            // longest class string 
            &cValues,                // number of values for this key 
            &cchMaxValue,            // longest value name 
            &cbMaxValueData,         // longest value data 
            &cbSecurityDescriptor,   // security descriptor 
            &ftLastWriteTime);       // last write time 

    // Enumerate the key values. 

    if (cValues) {
        for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) { 
            cchValue = MAX_VALUE_NAME; 
            achValue[0] = '\0'; 
            cbData = MAX_DATA_SIZE;
            memset(&bData, 0, MAX_DATA_SIZE);
            retCode = RegEnumValue( hKey, i,
                    achValue,
                    &cchValue,
                    NULL,
                    &dwType,
                    (LPBYTE)&bData,
                    &cbData);
            if (retCode == ERROR_SUCCESS && dwType == REG_SZ) { 
                QString key = tchar_to_qstring( (TCHAR*)bData, (cbData/sizeof(TCHAR))-1 );
                keys << key;
            }
        }
    }

    return keys;
}

