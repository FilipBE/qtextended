:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:: Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
:: Contact: Qt Software Information (qt-info@nokia.com)
::
:: This file is part of the Windows installer of the Qt Toolkit.
::
:: Commercial Usage
:: Licensees holding valid Qt Commercial licenses may use this file in
:: accordance with the Qt Commercial License Agreement provided with the
:: Software or, alternatively, in accordance with the terms contained in
:: a written agreement between you and Nokia.
::
::
:: GNU General Public License Usage
:: Alternatively, this file may be used under the terms of the GNU
:: General Public License versions 2.0 or 3.0 as published by the Free
:: Software Foundation and appearing in the file LICENSE.GPL included in
:: the packaging of this file.  Please review the following information
:: to ensure GNU General Public Licensing requirements will be met:
:: http://www.fsf.org/licensing/licenses/info/GPLv2.html and
:: http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
:: exception, Nokia gives you certain additional rights. These rights
:: are described in the Nokia Qt GPL Exception version 1.3, included in
:: the file GPL_EXCEPTION.txt in this package.
::
:: Qt for Windows(R) Licensees
:: As a special exception, Nokia, as the sole copyright holder for Qt
:: Designer, grants users of the Qt/Eclipse Integration plug-in the
:: right for the Qt/Eclipse Integration to link to functionality
:: provided by Qt Designer and its related libraries.
::
:: If you are unsure which license is appropriate for your use, please
:: contact the sales department at qt-sales@nokia.com.
::
:: This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
:: WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
call :%1 %2
goto END

:dest
  set IWMAKE_EXTRACTDEST=%IWMAKE_ROOT%\%~1
goto :eof

:extUnpack
  set IWMAKE_EXTRACTSRC=%~n1
  pushd %IWMAKE_ROOT%
  if exist "%IWMAKE_EXTRACTSRC%.zip" del /Q /F "%IWMAKE_EXTRACTSRC%.zip"
  %IWMAKE_WGET%\wget %IWMAKE_WGETUSER% %IWMAKE_WGETPASS% %IWMAKE_RELEASELOCATION%/%IWMAKE_EXTRACTSRC%.zip >> %IWMAKE_LOGFILE% 2>&1
  popd
  call :unpack "%~1"
goto :eof

:unpack
  set IWMAKE_EXTRACTSRC=%~n1
  pushd %IWMAKE_ROOT%
  if exist "%IWMAKE_EXTRACTDEST%" rd /S /Q %IWMAKE_EXTRACTDEST% >> %IWMAKE_LOGFILE% 2>&1
  if exist "%IWMAKE_EXTRACTSRC%" rd /S /Q %IWMAKE_EXTRACTSRC% >> %IWMAKE_LOGFILE% 2>&1
  %IWMAKE_UNZIPAPP% %IWMAKE_EXTRACTSRC%.zip >> %IWMAKE_LOGFILE%
  popd
  move %IWMAKE_ROOT%\%IWMAKE_EXTRACTSRC% %IWMAKE_EXTRACTDEST% >> %IWMAKE_LOGFILE%
goto :eof

:extPatch
  pushd %IWMAKE_ROOT%
  if exist "%~1" del /Q /F "%~1"
  %IWMAKE_WGET%\wget %IWMAKE_WGETUSER% %IWMAKE_WGETPASS% %IWMAKE_RELEASELOCATION%/%~1 >> %IWMAKE_LOGFILE% 2>&1
  popd
  call :patch "%~1"
goto :eof

:patch
  pushd %IWMAKE_ROOT%
  %IWMAKE_UNZIPAPP% %~1 >> %IWMAKE_LOGFILE%
  popd
  xcopy /R /I /S /Q /Y %IWMAKE_ROOT%\%IWMAKE_EXTRACTSRC%\*.* %IWMAKE_EXTRACTDEST%\ >> %IWMAKE_LOGFILE%
  rd /S /Q %IWMAKE_ROOT%\%IWMAKE_EXTRACTSRC% >> %IWMAKE_LOGFILE%
goto :eof

:END
