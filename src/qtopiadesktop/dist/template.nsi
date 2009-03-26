; Constants
!define PRODUCT_NAME "Qtopia Sync Agent"
!define PRODUCT_VERSION "$${VERSION}"
!define PRODUCT_FOLDERNAME "${PRODUCT_NAME}"
!define PRODUCT_PUBLISHER "Trolltech"
!define PRODUCT_WEB_SITE "http://www.trolltech.com/"
!define PRODUCT_DIR_REGKEY "Software\Trolltech\Qtopia Sync Agent\installdir"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"
!define PRODUCT_INSTALL_ROOT "$PROGRAMFILES\Trolltech\${PRODUCT_FOLDERNAME}"
!define PALMTOPCENTER "Software\Trolltech\Qtopia Sync Agent"

; General Setup
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "qtopiasyncagent-$${IVERSION}.exe"
InstallDir "${PRODUCT_INSTALL_ROOT}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails hide
ShowUnInstDetails hide
XPStyle on

$${DEBUG}SetCompress off
;$${NDEBUG}SetCompressor lzma


; Global Variables
var NESTED
var ICONS_GROUP

; NSIS "Modern" UI
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\win-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\win-uninstall.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "welcome.bmp"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "header.bmp"
!define MUI_HEADERIMAGE_RIGHT
!define MUI_INSTFILESPAGE_PROGRESSBAR ""
!define MUI_COMPONENTSPAGE_CHECKBITMAP "${NSISDIR}\Contrib\Graphics\Checks\classic.bmp"
!define MUI_COMPONENTSPAGE_NODESC

; MUI Pages
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${PRODUCT_NAME} ${PRODUCT_VERSION}.\r\n\r\nClick Next to continue."
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "$${DIMAGE}\LICENSE"
$${UpgradePages}
; The next line turns on the components page (if there are extra components)
$${USE_COMPONENTS}!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
; Start menu page
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${PRODUCT_FOLDERNAME}"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
;!define MUI_FINISHPAGE_RUN "$INSTDIR\qtopiasyncagent.exe"
;!define MUI_FINISHPAGE_RUN_TEXT "Run ${PRODUCT_NAME}"
$${NDEBUG}!define MUI_FINISHPAGE_SHOWREADME
$${NDEBUG}!define MUI_FINISHPAGE_SHOWREADME_TEXT "Run Qtopia Sync Agent at login"
$${NDEBUG}!define MUI_FINISHPAGE_SHOWREADME_FUNCTION SetupStartupItem
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!define MUI_PAGE_CUSTOMFUNCTION_PRE un.AllPre
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE un.CheckRunning
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the uninstallation of ${PRODUCT_NAME} ${PRODUCT_VERSION}.\r\n\r\nPlease close ${PRODUCT_NAME} ${PRODUCT_VERSION} before continuing or it will be automatically terminated.\r\n\r\nClick Uninstall to start the uninstallation."
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_INSTFILES
!define MUI_PAGE_CUSTOMFUNCTION_PRE un.MaybeSilent
!insertmacro MUI_UNPAGE_FINISH

; Language Settings
!insertmacro MUI_LANGUAGE "English"

; Reserve files
ReserveFile "ioPrevInst.ini"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; Installer sections

Section "-Qtopia Sync Agent"
    SetOverwrite on

    $${FILES}

    CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
    CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Qtopia Sync Agent.lnk" "$INSTDIR\qtopiasyncagent.exe"
    $${NDEBUG}WriteRegDWORD HKCU "${PALMTOPCENTER}\settings" "debugmode" 0
    $${DEBUG}WriteRegDWORD HKCU "${PALMTOPCENTER}\settings" "debugmode" 1
SectionEnd

Section -Removal
    CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk" "$INSTDIR\uninst.exe"
    WriteUninstaller "$INSTDIR\uninst.exe"
    WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\qtopiasyncagent.exe"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "${PRODUCT_STARTMENU_REGVAL}" "$ICONS_GROUP"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
    WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
    WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1
SectionEnd

; This is where customer-specific installer components go
$${EXTRA_COMPONENTS}

Section Uninstall
    ReadRegStr $ICONS_GROUP ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "${PRODUCT_STARTMENU_REGVAL}"
    $${UN_FILES}
    ; Remove any i18n content
    RMDir /r "$INSTDIR\doc"
    RMDir /r "$INSTDIR\i18n"

    ; If we're nested and files can't be deleted, prompt the user to reboot
    ; If the user aborts, an error occurs and the installer will stop
    StrCmp $NESTED 1 0 +3
	IfRebootFlag 0 +2
	    StrCpy $NESTED 0

    StrCmp $NESTED 1 +3 0
	Delete /rebootok "$INSTDIR\uninst.exe"
	RMDir /rebootok "$INSTDIR"

    Delete "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk"
    Delete "$SMPROGRAMS\$ICONS_GROUP\Qtopia Sync Agent.lnk"
    RMDir "$SMPROGRAMS\$ICONS_GROUP"

    Delete "$SMPROGRAMS\Startup\Qtopia Sync Agent.lnk"

    DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
    DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
SectionEnd

; Initialisation Functions

Function .onInit
    !insertmacro MUI_INSTALLOPTIONS_EXTRACT "ioPrevInst.ini"
    Call CheckUnique
    ; Check for Windows 2000 or above unless /FORCE98 was passed
    call GetParameters
    Pop $R0
    StrCmp $R0 "/FORCE98" +3 +1
    StrCmp $R0 "/FORCE98 "+2 +1
        Call Check2000
FunctionEnd

Function un.onInit
    !insertmacro MUI_INSTALLOPTIONS_EXTRACT "ioPrevInst.ini"
    call un.GetParameters
    ; check for a "nested" uninstall (uninstall from the installer)
    Pop $R0
    StrCpy $NESTED 0
    StrCmp $R0 "/NESTED" +2 +1
    StrCmp $R0 "/NESTED " 0 +2
	StrCpy $NESTED 1
FunctionEnd

; Macros (used to create functions below)

!macro USER_GET_PARAMETERS
    Push $R0
    Push $R1
    Push $R2
    Push $R3

    StrCpy $R2 1
    StrLen $R3 $CMDLINE

    ;Check for quote or space
    StrCpy $R0 $CMDLINE $R2
    StrCmp $R0 '"' 0 +3
	StrCpy $R1 '"'
	Goto loop
    StrCpy $R1 " "

    loop:
	IntOp $R2 $R2 + 1
	StrCpy $R0 $CMDLINE 1 $R2
	StrCmp $R0 $R1 get
	StrCmp $R2 $R3 get
	Goto loop

    get:
	IntOp $R2 $R2 + 1
	StrCpy $R0 $CMDLINE 1 $R2
	StrCmp $R0 " " get
	StrCpy $R0 $CMDLINE "" $R2

    Pop $R3
    Pop $R2
    Pop $R1
    Exch $R0
!macroend

!macro IsUserAdmin RESULT
    !define Index "Line${__LINE__}"
    StrCpy ${RESULT} 0
    System::Call '*(&i1 0,&i4 0,&i1 5)i.r0'
    System::Call 'advapi32::AllocateAndInitializeSid(i r0,i 2,i 32,i 544,i 0,i 0,i 0,i 0,i 0,i 0,*i .R0)i.r5'
    System::Free $0
    System::Call 'advapi32::CheckTokenMembership(i n,i R0,*i .R1)i.r5'
    StrCmp $5 0 ${Index}_Error
        StrCpy ${RESULT} $R1
        Goto ${Index}_End
${Index}_Error:
    StrCpy ${RESULT} -1
${Index}_End:
    System::Call 'advapi32::FreeSid(i R0)i.r5'
    !undef Index
!macroend

; NSIS Functions

Function GetParameters
    !insertmacro USER_GET_PARAMETERS
FunctionEnd

Function un.GetParameters
    !insertmacro USER_GET_PARAMETERS
FunctionEnd

Function un.CheckRunning
    ExecWait '"$INSTDIR\qtopiasyncagent.exe" --quit'
    Sleep 1000
    Push $0
    !define Index "Line${__LINE__}"
${Index}_Loop:
    FindWindow $0 "QWidget" "09F911029D74E35BD84156C5635688C0_QtSingleApplicationWindow"
    IntCmp $0 0 +3
        Sleep 1000
        Goto ${Index}_Loop
    !undef Index
    Pop $0 
FunctionEnd

; User Functions

Function CheckUnique
    ; Make sure only one instance of the installer runs at a time
    System::Call 'kernel32::CreateMutexA(i 0, i 0, t "QtopiaSyncAgentInstallerMutex") i .r1 ?e'
    Pop $0
    StrCmp $0 0 done
	MessageBox MB_OK|MB_ICONEXCLAMATION "This installer is already running."
	Abort 
    done:
FunctionEnd

!include "LogicLib.nsh"
!include "WinVer.nsh"
Function Check2000
    ${If} ${AtLeastWin2000}
        !insertmacro IsUserAdmin $0
        StrCmp $0 1 +3
            MessageBox MB_OK|MB_ICONEXCLAMATION "Qtopia Sync Agent can only be installed by an Administrator."
            Abort
    ${Else}
        MessageBox MB_OK|MB_ICONEXCLAMATION "Qtopia Sync Agent requires Windows 2000 or newer."
	Abort
    ${Endif}
FunctionEnd

Function SetupStartupItem
    CreateShortCut "$SMPROGRAMS\Startup\Qtopia Sync Agent.lnk" "$INSTDIR\qtopiasyncagent.exe" "--nosplash --tray" "$INSTDIR\qtopiasyncagent.exe" 1
FunctionEnd

Function un.MaybeSilent
    StrCmp $NESTED 1 0 +2
	Abort
FunctionEnd

Function un.AllPre
    StrCmp $NESTED 1 0 +3
	Call un.CheckRunning
        Abort
FunctionEnd

$${UpgradeFunctions}

