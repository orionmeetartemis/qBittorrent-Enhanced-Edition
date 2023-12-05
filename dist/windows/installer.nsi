Var uninstallerPath

Section "-hidden"

    ;Search if qBittorrent is already installed.
    FindFirst $0 $1 "$uninstallerPath\uninst.exe"
    FindClose $0
    StrCmp $1 "" done

    ;Run the uninstaller of the previous install.
    DetailPrint $(inst_unist)
    ExecWait '"$uninstallerPath\uninst.exe" /S _?=$uninstallerPath'
    Delete "$uninstallerPath\uninst.exe"
    RMDir "$uninstallerPath"

    done:

SectionEnd


Section $(inst_qbt_req) ;"qBittorrent (required)"

  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put files there
  File "qbittorrent.exe"
  File "qt.conf"

  ;Create 'translations' directory
  CreateDirectory $INSTDIR\translations
  ; Set output path to the installation\translations directory.
  SetOutPath "$INSTDIR\translations"
  ; Put files there
  File /r "translations\qt*.qm"
  ; Restore output path because it affects `CreateShortCut`. It affects the "Start in" field.
  SetOutPath $INSTDIR

  ; Write the installation path into the registry
  WriteRegStr HKLM "Software\qBittorrent" "InstallLocation" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qBittorrent" "DisplayName" "qBittorrent"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qBittorrent" "UninstallString" '"$INSTDIR\uninst.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qBittorrent" "DisplayIcon" '"$INSTDIR\qbittorrent.exe",0'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qBittorrent" "Publisher" "The qBittorrent project"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qBittorrent" "URLInfoAbout" "https://www.qbittorrent.org"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qBittorrent" "DisplayVersion" "${QBT_VERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qBittorrent" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qBittorrent" "NoRepair" 1
  WriteUninstaller "uninst.exe"
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qBittorrent" "EstimatedSize" "$0"

  ; qBittorrent ProgID
  WriteRegStr HKLM "Software\Classes\qBittorrent" "" "qBittorrent Torrent File"
  WriteRegStr HKLM "Software\Classes\qBittorrent" "FriendlyTypeName" "qBittorrent Torrent File"
  WriteRegStr HKLM "Software\Classes\qBittorrent\shell" "" "open"
  WriteRegStr HKLM "Software\Classes\qBittorrent\shell\open\command" "" '"$INSTDIR\qbittorrent.exe" "%1"'
  WriteRegStr HKLM "Software\Classes\qBittorrent\DefaultIcon" "" '"$INSTDIR\qbittorrent.exe",1'

SectionEnd

; Optional section (can be disabled by the user)
Section "Install with Debug Symbol File"

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  File "qbittorrent.pdb"
  Exec '"cmd.exe" /c COMPACT /C "$INSTDIR\qbittorrent.pdb"'

SectionEnd

Section /o $(inst_desktop) ;"Create Desktop Shortcut"

  CreateShortCut "$DESKTOP\qBittorrent.lnk" "$INSTDIR\qbittorrent.exe"

SectionEnd

Section $(inst_startmenu) ;"Create Start Menu Shortcut"

  CreateDirectory "$SMPROGRAMS\qBittorrent"
  CreateShortCut "$SMPROGRAMS\qBittorrent\qBittorrent.lnk" "$INSTDIR\qbittorrent.exe"
  CreateShortCut "$SMPROGRAMS\qBittorrent\$(inst_uninstall_link_description).lnk" "$INSTDIR\uninst.exe"

SectionEnd

Section /o $(inst_startup) ;"Start qBittorrent on Windows start up"

  !insertmacro UAC_AsUser_Call Function inst_startup_user ${UAC_SYNCREGISTERS}|${UAC_SYNCOUTDIR}|${UAC_SYNCINSTDIR}

SectionEnd

Function inst_startup_user

  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "qBittorrent" "$INSTDIR\qbittorrent.exe"

FunctionEnd

Section $(inst_torrent) ;"Open .torrent files with qBittorrent"

  ReadRegStr $0 HKLM "Software\Classes\.torrent" ""

  StrCmp $0 "qBittorrent" clear_errors 0
  ;Check if empty string
  StrCmp $0 "" clear_errors 0
  ;Write old value to OpenWithProgIds
  WriteRegStr HKLM "Software\Classes\.torrent\OpenWithProgIds" $0 ""

  clear_errors:
  ClearErrors

  WriteRegStr HKLM "Software\Classes\.torrent" "" "qBittorrent"
  WriteRegStr HKLM "Software\Classes\.torrent" "Content Type" "application/x-bittorrent"

  !insertmacro UAC_AsUser_Call Function inst_torrent_user ${UAC_SYNCREGISTERS}|${UAC_SYNCOUTDIR}|${UAC_SYNCINSTDIR}

  System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, p 0, p 0)'

SectionEnd

Function inst_torrent_user

  ReadRegStr $0 HKCU "Software\Classes\.torrent" ""

  StrCmp $0 "qBittorrent" clear_errors 0
  ;Check if empty string
  StrCmp $0 "" clear_errors 0
  ;Write old value to OpenWithProgIds
  WriteRegStr HKCU "Software\Classes\.torrent\OpenWithProgIds" $0 ""

  clear_errors:
  ClearErrors

  WriteRegStr HKCU "Software\Classes\.torrent" "" "qBittorrent"
  WriteRegStr HKCU "Software\Classes\.torrent" "Content Type" "application/x-bittorrent"

FunctionEnd

Section $(inst_magnet) ;"Open magnet links with qBittorrent"

  WriteRegStr HKLM "Software\Classes\magnet" "" "URL:Magnet link"
  WriteRegStr HKLM "Software\Classes\magnet" "Content Type" "application/x-magnet"
  WriteRegStr HKLM "Software\Classes\magnet" "URL Protocol" ""
  WriteRegStr HKLM "Software\Classes\magnet\DefaultIcon" "" '"$INSTDIR\qbittorrent.exe",1'
  WriteRegStr HKLM "Software\Classes\magnet\shell" "" "open"
  WriteRegStr HKLM "Software\Classes\magnet\shell\open\command" "" '"$INSTDIR\qbittorrent.exe" "%1"'

  !insertmacro UAC_AsUser_Call Function inst_magnet_user ${UAC_SYNCREGISTERS}|${UAC_SYNCOUTDIR}|${UAC_SYNCINSTDIR}

  System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, p 0, p 0)'

SectionEnd

Function inst_magnet_user

  WriteRegStr HKCU "Software\Classes\magnet" "" "URL:Magnet link"
  WriteRegStr HKCU "Software\Classes\magnet" "Content Type" "application/x-magnet"
  WriteRegStr HKCU "Software\Classes\magnet" "URL Protocol" ""
  WriteRegStr HKCU "Software\Classes\magnet\DefaultIcon" "" '"$INSTDIR\qbittorrent.exe",1'
  WriteRegStr HKCU "Software\Classes\magnet\shell" "" "open"
  WriteRegStr HKCU "Software\Classes\magnet\shell\open\command" "" '"$INSTDIR\qbittorrent.exe" "%1"'

FunctionEnd

Section $(inst_firewall)

  DetailPrint $(inst_firewallinfo)
  nsisFirewallW::AddAuthorizedApplication "$INSTDIR\qbittorrent.exe" "qBittorrent"

SectionEnd

Section $(inst_pathlimit) ;"Disable Windows path length limit (260 character MAX_PATH limitation, requires Windows 10 1607 or later)"

  WriteRegDWORD HKLM "SYSTEM\CurrentControlSet\Control\FileSystem" "LongPathsEnabled" 1

SectionEnd

;--------------------------------

Function .onInit

  !insertmacro Init "installer"
  !insertmacro MUI_LANGDLL_DISPLAY

  !ifndef QBT_USES_QT6
    ${IfNot} ${AtLeastWin7}
      MessageBox MB_OK|MB_ICONEXCLAMATION $(inst_requires_win7)
      Abort
    ${EndIf}
  !else
    ${IfNot} ${AtLeastWaaS} 1809 ; Windows 10 (1809) / Windows Server 2019. Min supported version by Qt6
      MessageBox MB_OK|MB_ICONEXCLAMATION $(inst_requires_win10)
      Abort
    ${EndIf}
  !endif

  !ifdef QBT_IS_X64
    ${IfNot} ${RunningX64}
      MessageBox MB_OK|MB_ICONEXCLAMATION $(inst_requires_64bit)
      Abort
    ${EndIf}
  !endif

  ;Search if qBittorrent is already installed.
  FindFirst $0 $1 "$INSTDIR\uninst.exe"
  FindClose $0
  StrCmp $1 "" done

  ;Copy old value to var so we can call the correct uninstaller
  StrCpy $uninstallerPath $INSTDIR

  ;Inform the user
  MessageBox MB_OKCANCEL|MB_ICONINFORMATION $(inst_uninstall_question) /SD IDOK IDOK done
  Quit

  done:

FunctionEnd

Function check_instance

  check:
  FindProcDLL::FindProc "qbittorrent.exe"
  StrCmp $R0 "1" 0 notfound
  MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION $(inst_warning) IDRETRY check IDCANCEL done

  done:
  Abort

  notfound:

FunctionEnd

Function PageFinishRun

  !insertmacro UAC_AsUser_ExecShell "" "$INSTDIR\qbittorrent.exe" "" "" ""

FunctionEnd

Function .onInstSuccess
  SetErrorLevel 0
FunctionEnd
