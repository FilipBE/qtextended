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
!ifndef LIST_INCLUDE
!define LIST_INCLUDE

; usage:
; push item
; push list
; call ItemInList
; returns 1 or 0
!macro ItemInList UN
Function ${UN}ItemInList
  exch $0 ;list
  exch
  exch $1 ;item
  push $2 ;counter
  push $3 ;substr
  push $4 ;char
  
  strcpy $3 ""
  strcpy $2 "0"

  loop:
    strcpy $4 $0 1 $2
    strcmp "$4" "" atend
    intop $2 $2 + 1

    strcmp "$4" "|" 0 +4
      strcmp "$3" "$1" found
      strcpy $3 "" ;reset substr
      goto +2
    strcpy $3 "$3$4" ;append char to substr
    goto loop

  found:
    strcpy $0 "1"
    goto done
    
  atend:
    strcmp "$3" "$1" found
    strcpy $0 "0"

  done:
  pop $4
  pop $3
  pop $2
  pop $1
  exch $0
FunctionEnd
!macroend

!insertmacro ItemInList ""
!insertmacro ItemInList "un."

Function GetItemInList
  exch $0 ;list
  exch
  exch $1 ;index
  push $2 ;counter
  push $3 ;substr
  push $4 ;char
  push $5 ;current index

  strcpy $3 ""
  strcpy $2 "0"
  strcpy $5 "1"

  loop:
    strcpy $4 $0 1 $2
    strcmp "$4" "" atend
    intop $2 $2 + 1

    strcmp "$4" "|" 0 +5
      strcmp "$5" "$1" found
      strcpy $3 "" ;reset substr
      intop $5 $5 + 1
      goto +2
    strcpy $3 "$3$4" ;append char to substr
    goto loop

  found:
    strcpy $0 "$3"
    goto done

  atend:
    strcmp "$5" "$1" found
    strcpy $0 ""

  done:
  pop $5
  pop $4
  pop $3
  pop $2
  pop $1
  exch $0
FunctionEnd

!endif ;LIST_INCLUDE