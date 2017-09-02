# Microsoft Developer Studio Project File - Name="DM2EXE" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DM2EXE - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DM2EXE.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DM2EXE.mak" CFG="DM2EXE - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DM2EXE - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DM2EXE - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DM2EXE - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gz /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"../Release/DM2.exe"

!ELSEIF  "$(CFG)" == "DM2EXE - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Gz /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../Debug/DM2.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "DM2EXE - Win32 Release"
# Name "DM2EXE - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "SettingDlg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SettingDialog\AboutDlg.c
# End Source File
# Begin Source File

SOURCE=.\SettingDialog\ActionDlg.c
# End Source File
# Begin Source File

SOURCE=.\SettingDialog\AdvanceDlg.c
# End Source File
# Begin Source File

SOURCE=.\SettingDialog\CustomDlg.c
# End Source File
# Begin Source File

SOURCE=.\SettingDialog\ExceptionDlg.c
# End Source File
# Begin Source File

SOURCE=.\SettingDialog\FavMenuDlg.c
# End Source File
# Begin Source File

SOURCE=.\SettingDialog\GeneralDlg.c
# End Source File
# Begin Source File

SOURCE=.\SettingDialog\HotKeyDlg.c
# End Source File
# Begin Source File

SOURCE=.\SettingDialog\PluginsDlg.c
# End Source File
# Begin Source File

SOURCE=.\SettingDialog\SettingDlg.c
# End Source File
# Begin Source File

SOURCE=.\SettingDialog\WinMangerDlg.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\DM2.c
# End Source File
# Begin Source File

SOURCE=.\dm2.rc
# End Source File
# Begin Source File

SOURCE=.\dm2WndProc.c
# End Source File
# Begin Source File

SOURCE=.\float.c
# End Source File
# Begin Source File

SOURCE=.\GetSetting.c
# End Source File
# Begin Source File

SOURCE=.\MImage.c
# End Source File
# Begin Source File

SOURCE=.\MultiLanguage.c
# End Source File
# Begin Source File

SOURCE=.\Plugins.c
# End Source File
# Begin Source File

SOURCE=..\tools.c
# End Source File
# Begin Source File

SOURCE=.\URLCtrl.c
# End Source File
# Begin Source File

SOURCE=.\util.c
# End Source File
# Begin Source File

SOURCE=.\wnddata.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DM2.h
# End Source File
# Begin Source File

SOURCE=.\dm2WndProc.h
# End Source File
# Begin Source File

SOURCE=.\float.h
# End Source File
# Begin Source File

SOURCE=.\GetSetting.h
# End Source File
# Begin Source File

SOURCE=..\global.h
# End Source File
# Begin Source File

SOURCE=.\MImage.h
# End Source File
# Begin Source File

SOURCE=.\MultiLanguage.h
# End Source File
# Begin Source File

SOURCE=.\Plugins.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SettingDialog\SettingDlg.h
# End Source File
# Begin Source File

SOURCE=..\tools.h
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# Begin Source File

SOURCE=.\wnddata.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Align.bmp
# End Source File
# Begin Source File

SOURCE=..\dm2.ico
# End Source File
# Begin Source File

SOURCE=..\dm2.jpg
# End Source File
# Begin Source File

SOURCE=.\TreeDir.ico
# End Source File
# Begin Source File

SOURCE=.\TreeItem.ico
# End Source File
# End Group
# End Target
# End Project
