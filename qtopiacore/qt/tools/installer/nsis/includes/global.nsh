;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
;; Contact: Qt Software Information (qt-info@nokia.com)
;;
;; This file is part of the Windows installer of the Qt Toolkit.
;;
;; Commercial Usage
;; Licensees holding valid Qt Commercial licenses may use this file in
;; accordance with the Qt Commercial License Agreement provided with the
;; Software or, alternatively, in accordance with the terms contained in
;; a written agreement between you and Nokia.
;;
;;
;; GNU General Public License Usage
;; Alternatively, this file may be used under the terms of the GNU
;; General Public License versions 2.0 or 3.0 as published by the Free
;; Software Foundation and appearing in the file LICENSE.GPL included in
;; the packaging of this file.  Please review the following information
;; to ensure GNU General Public Licensing requirements will be met:
;; http://www.fsf.org/licensing/licenses/info/GPLv2.html and
;; http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
;; exception, Nokia gives you certain additional rights. These rights
;; are described in the Nokia Qt GPL Exception version 1.3, included in
;; the file GPL_EXCEPTION.txt in this package.
;;
;; Qt for Windows(R) Licensees
;; As a special exception, Nokia, as the sole copyright holder for Qt
;; Designer, grants users of the Qt/Eclipse Integration plug-in the
;; right for the Qt/Eclipse Integration to link to functionality
;; provided by Qt Designer and its related libraries.
;;
;; If you are unsure which license is appropriate for your use, please
;; contact the sales department at qt-sales@nokia.com.
;;
;; This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
;; WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
!include "StrFunc.nsh"
!include "includes\list.nsh"

${StrCase}
${StrTrimNewLines}
${StrStr}
${StrRep}
${UnStrRep}

var STARTMENU_STRING
var PRODUCT_UNIQUE_KEY
var RUNNING_AS_ADMIN

!ifndef MODULE_MINGW
  !ifdef MODULE_MSVC_VC60
    !define INSTALL_COMPILER "vc60"
  !else
    !ifdef MODULE_MSVC_VS2002
      !define INSTALL_COMPILER "vs2002"
    !else
      !ifdef MODULE_MSVC_VS2005
        !define INSTALL_COMPILER "vs2005"
      !else
        !define INSTALL_COMPILER "vs2003"
      !endif
    !endif
  !endif
!else
  !define INSTALL_COMPILER "mingw"
!endif

; ADDIN\INTEGRATION
var VS_VERSION
var VS_VERSION_SHORT
var ADDIN_INSTDIR
var VSIP_INSTDIR
var HELP_INSTDIR
var ECLIPSE_INSTDIR
var QTJAMBIECLIPSE_INSTDIR

; LICENSECHECK
var LICENSE_KEY
var LICENSEE
var LICENSE_PRODUCT
var LICENSE_FILE

; MSVC
!ifdef MODULE_MSVC
  !define MSVC_ValidateDirectory
  var MSVC_INSTDIR
!endif

; MINGW
!ifdef MODULE_MINGW
  !define MINGW_ValidateDirectory
  var MINGW_INSTDIR
!endif

; QSA
var QSA_INSTDIR

; QTDIR PAGE
var QTDIR_SELECTED
var COMPILER_SELECTED

; used by addin7x and vsip
!ifndef MODULE_VSIP_ROOT
  !define MODULE_VSIP_ROOT "${INSTALL_ROOT}\vsip"
!endif

; add to confirm path
var UninstallerConfirmProduct

Function un.ConfirmOnDelete
  exch $0
  push $1

  push "$0"
  push "$UninstallerConfirmProduct"
  call un.ItemInList
  pop $1
  IntCmp $1 1 ConfirmOnDeleteDone
  
  strcmp "$UninstallerConfirmProduct" "" 0 +3
    strcpy $UninstallerConfirmProduct "$0"
  goto +2
    strcpy $UninstallerConfirmProduct "$UninstallerConfirmProduct$\r$\n$0"
    
  ConfirmOnDeleteDone:
  pop $1
  pop $0
FunctionEnd

!macro ConfirmOnRemove REG_KEY PRODUCT_NAME
  push $0
  ClearErrors
  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" "${REG_KEY}"
  intcmp $0 1 0 +3
    push "${PRODUCT_NAME}"
    call un.ConfirmOnDelete
  ClearErrors
  pop $0
!macroend