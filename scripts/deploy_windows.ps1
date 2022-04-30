# qt directory
$env:Path += ";C:\Qt\6.3.0\mingw_64\bin"

# mingw directory
$env:Path += ";C:\Qt\Tools\mingw1120_64\bin"

# installer framework directory
$env:Path += ";C:\Qt\Tools\QtInstallerFramework\4.3\bin"

# NSIS installer framework
$env:Path += ";C:\Program Files (x86)\NSIS"

# get build type and path
$build_type = If ($args.Length -ge 1) { $args[0] } Else { 'Release' }
$build_path = "..\..\build-c64pp-*-$build_type"
$build_text = If ($build_type -Eq 'Release') { '' } Else { "-$build_type" }

# get version and construct package_name
$version = (Select-String -Path "$build_path\CMakeCache.txt" -Pattern "CMAKE_PROJECT_VERSION:STATIC=(.*)").Matches.Groups[-1]
$package_name = "C64++-$version$build_text"

echo "[*] prepare folders"
If (!(Test-Path "deployment")) { mkdir "deployment" | Out-Null }

$package_path = "$(pwd)\deployment\$package_name"
If (Test-Path $package_path) { Remove-Item $package_path -r }
mkdir $package_path | Out-Null

$portable_path = "$package_path\portable"
mkdir $portable_path | Out-Null

echo "[*] copy main executable"
Copy-Item "$build_path\C64++.exe" $portable_path

echo "[*] run qt deployment tool to construct dependencies"
& 'windeployqt' $portable_path '--no-translations' '--verbose=1'

echo "[*] copy license"
@"
blablabla bla bla blub
TODO: add license
"@ | Out-File -FilePath "$portable_path\license.txt" -Encoding utf8

echo "[*] zip portable release"
Compress-Archive -Path "$portable_path\*" -DestinationPath "$package_path\$package_name.zip"

echo "[*] write NSIS build script"
@"
!include Util.nsh
!macro RegisterExtensionCall _EXECUTABLE _EXTENSION _DESCRIPTION
  Push ```${_DESCRIPTION}``
  Push ```${_EXTENSION}``
  Push ```${_EXECUTABLE}``
  `${CallArtificialFunction} RegisterExtension_
!macroend
!macro UnRegisterExtensionCall _EXTENSION _DESCRIPTION
  Push ```${_EXTENSION}``
  Push ```${_DESCRIPTION}``
  `${CallArtificialFunction} UnRegisterExtension_
!macroend
!define RegisterExtension ``!insertmacro RegisterExtensionCall``
!define un.RegisterExtension ``!insertmacro RegisterExtensionCall``
!macro RegisterExtension
!macroend
!macro un.RegisterExtension
!macroend

!macro RegisterExtension_
  Exch `$R2 ;exe
  Exch
  Exch `$R1 ;ext
  Exch
  Exch 2
  Exch `$R0 ;desc
  Exch 2
  Push `$0
  Push `$1
  ReadRegStr `$1 HKCR `$R1 ""  ; read current file association
  StrCmp "`$1" "" NoBackup  ; is it empty
  StrCmp "`$1" "`$R0" NoBackup  ; is it our own
    WriteRegStr HKCR `$R1 "backup_val" "`$1"  ; backup current value
NoBackup:
  WriteRegStr HKCR `$R1 "" "`$R0"  ; set our file association
  ReadRegStr `$0 HKCR `$R0 ""
  StrCmp `$0 "" 0 Skip
    WriteRegStr HKCR "`$R0" "" "`$R0"
    WriteRegStr HKCR "`$R0\shell" "" "open"
    WriteRegStr HKCR "`$R0\DefaultIcon" "" "`$R2,0"
Skip:
  WriteRegStr HKCR "`$R0\shell\open\command" "" '"`$R2" "%1"'
  WriteRegStr HKCR "`$R0\shell\edit" "" "Edit `$R0"
  WriteRegStr HKCR "`$R0\shell\edit\command" "" '"`$R2" "%1"'
  Pop `$1
  Pop `$0
  Pop `$R2
  Pop `$R1
  Pop `$R0
!macroend
!define UnRegisterExtension ``!insertmacro UnRegisterExtensionCall``
!define un.UnRegisterExtension ``!insertmacro UnRegisterExtensionCall``
!macro UnRegisterExtension
!macroend
!macro un.UnRegisterExtension
!macroend

!macro UnRegisterExtension_ 
  Exch `$R1 ;desc
  Exch
  Exch `$R0 ;ext
  Exch
  Push `$0
  Push `$1
  ReadRegStr `$1 HKCR `$R0 ""
  StrCmp `$1 `$R1 0 NoOwn ; only do this if we own it
  ReadRegStr `$1 HKCR `$R0 "backup_val"
  StrCmp `$1 "" 0 Restore ; if backup="" then delete the whole key
  DeleteRegKey HKCR `$R0
  Goto NoOwn
Restore:
  WriteRegStr HKCR `$R0 "" `$1
  DeleteRegValue HKCR `$R0 "backup_val"
  DeleteRegKey HKCR `$R1 ;Delete key with association name settings
NoOwn:
  Pop `$1
  Pop `$0
  Pop `$R1
  Pop `$R0
!macroend

var SM_Folder

VIProductVersion  "$version.0"
VIAddVersionKey "ProductName"  "C64++"
VIAddVersionKey "CompanyName"  "C64++"
VIAddVersionKey "LegalCopyright"  ""
VIAddVersionKey "FileDescription"  "C64 Emulator"
VIAddVersionKey "FileVersion"  "$version"
SetCompressor ZLIB
Name "C64++"
Caption "C64++"
OutFile "$package_path\$package_name.exe"
BrandingText "C64++"
XPStyle on
InstallDirRegKey "HKLM" "Software\Microsoft\Windows\CurrentVersion\App Paths\C64++.exe" ""
InstallDir "`$PROGRAMFILES\C64++"

!include "MUI.nsh"
!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "$portable_path\license.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "C64++"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\C64++"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "REG_START_MENU"
!insertmacro MUI_PAGE_STARTMENU Application `$SM_Folder
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "`$INSTDIR\C64++.exe"
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

Section -MainSection
SetShellVarContext all
SetOverwrite ifnewer
SetOutPath "`$INSTDIR"
File /r "$portable_path\*"
SectionEnd

Section "register .PRG extension" FileExt
`${registerExtension} "`$INSTDIR\C64++.exe" ".prg" "C64 PRG program"
SectionEnd
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT `${FileExt} "Link .PRG files to the C64++ Emulator"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section -Icons_Reg
SetOutPath "`$INSTDIR"
WriteUninstaller "`$INSTDIR\uninstall.exe"
!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
CreateDirectory "`$SMPROGRAMS\`$SM_Folder"
CreateShortCut "`$SMPROGRAMS\`$SM_Folder\C64++.lnk" "`$INSTDIR\C64++.exe"
!insertmacro MUI_STARTMENU_WRITE_END
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\C64++.exe" "" "`$INSTDIR\C64++.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\C64++"  "DisplayName" "C64++"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\C64++"  "UninstallString" "`$INSTDIR\uninstall.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\C64++"  "DisplayIcon" "`$INSTDIR\C64++.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\C64++"  "DisplayVersion" "$version"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\C64++"  "Publisher" "C64++"
SectionEnd

Section Uninstall
SetShellVarContext all
RmDir /r "`$INSTDIR"
!insertmacro MUI_STARTMENU_GETFOLDER "Application" `$SM_Folder
Delete "`$SMPROGRAMS\`$SM_Folder\C64++.lnk"
RmDir "`$SMPROGRAMS\`$SM_Folder"
`${unregisterExtension} ".prg" "C64 PRG program"
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\C64++.exe"
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\C64++"
SectionEnd
"@ | Out-File -FilePath "$package_path\Installer-script.nsi" -Encoding utf8
& "makensis" "$package_path\Installer-script.nsi"