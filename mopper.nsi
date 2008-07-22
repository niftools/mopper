;
; Mopper Self-Installer for Windows
; (NifTools - http://niftools.sourceforge.net) 
; (NSIS - http://nsis.sourceforge.net)
;
; Copyright (c) 2008, NIF File Format Library and Tools
; All rights reserved.
; 
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are
; met:
; 
;     * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;     * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation ; and/or other materials provided with the
;       distribution.
;     * Neither the name of the NIF File Format Library and Tools project
;       nor the names of its contributors may be used to endorse or promote
;       products derived from this software without specific prior written
;       permission.
; 
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
; IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
; THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
; PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
; CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
; EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
; PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
; LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
; NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

SetCompressor /SOLID lzma

!include "MUI.nsh"

!define VERSION "0.0.1"

Name "Mopper ${VERSION}"

; define installer pages
!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_NOAUTOCLOSE

!define MUI_WELCOMEPAGE_TEXT  "This wizard will guide you through the installation of Mopper ${VERSION}.\r\n\r\nIt is recommended that you close all other applications.\r\n\r\nNote to Win2k/XP users: you require administrator privileges to install Mopper successfully."
!insertmacro MUI_PAGE_WELCOME

!insertmacro MUI_PAGE_LICENSE LICENSE.TXT

!define MUI_DIRECT1ORYPAGE_TEXT_TOP "Use the field below to specify the folder where you want the Mopper files to be copied to. To specify a different folder, type a new name or use the Browse button to select an existing folder."
!define MUI_DIRECTORYPAGE_TEXT_DESTINATION "Installation Folder"
!define MUI_DIRECTORYPAGE_VARIABLE $INSTDIR
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_LINK "Visit us at http://niftools.sourceforge.net/"
!define MUI_FINISHPAGE_LINK_LOCATION "http://niftools.sourceforge.net/"
!insertmacro MUI_PAGE_FINISH

!define MUI_WELCOMEPAGE_TEXT  "This wizard will guide you through the uninstallation of Mopper ${VERSION}.\r\n\r\nClick Next to continue."
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
; Languages
 
!insertmacro MUI_LANGUAGE "English"
    
;--------------------------------
;Language Strings

;Description
LangString DESC_SecCopyUI ${LANG_ENGLISH} "Copy all required files to the application folder."

;--------------------------------
; Data

OutFile "mopper-${VERSION}-windows.exe"
InstallDir "$PROGRAMFILES\NifTools\Mopper"
BrandingText "http://niftools.sourceforge.net/"
;Icon inst.ico
;UninstallIcon inst.ico ; TODO create uninstall icon
ShowInstDetails show
ShowUninstDetails show

Section
  SectionIn RO

  SetShellVarContext all

  ; Install documentation files
  SetOutPath $INSTDIR
  File Release\mopper.exe
  File LICENSE.TXT

  ; Write the installation path into the registry so other tools can find the mopper
  WriteRegStr HKLM SOFTWARE\NifToolsMopper "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys & uninstaller for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NifToolsMopper" "DisplayName" "NifTools Mopper (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NifToolsMopper" "UninstallString" "$INSTDIR\uninstall.exe"
  SetOutPath $INSTDIR
  WriteUninstaller "uninstall.exe"
SectionEnd

Section "Uninstall"
  SetShellVarContext all
  SetAutoClose false

  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NifToolsMopper"
  DeleteRegKey HKLM "SOFTWARE\NifToolsMopper"

  ; remove program files and program directory
  Delete "$INSTDIR\*.*"
  RMDir "$INSTDIR"

SectionEnd
