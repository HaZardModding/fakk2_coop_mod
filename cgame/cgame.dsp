# Microsoft Developer Studio Project File - Name="cgame" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=cgame - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cgame.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cgame.mak" CFG="cgame - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cgame - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cgame - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cgame - Win32 Demo Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cgame - Win32 German Demo Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/fakk2_code/fakk2_new/cgame", DYHAAAAA"
# PROP Scc_LocalPath "."
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cgame - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "CGAME_DLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /map /machine:I386 /out:"d:\games\fakk2\ctf\cgamex86.dll"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W4 /Gm /GR /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "CGAME_DLL" /D "__STL_DEBUG" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /debug /machine:I386 /out:"d:\games\fakk2\ctf\cgamex86.dll"
# SUBTRACT LINK32 /profile /map /nodefaultlib

!ELSEIF  "$(CFG)" == "cgame - Win32 Demo Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cgame___Win32_Demo_Release"
# PROP BASE Intermediate_Dir "cgame___Win32_Demo_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo_Release"
# PROP Intermediate_Dir "Demo_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "CGAME_DLL" /FR /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GR /GX /Zi /O2 /D "_WINDOWS" /D "CGAME_DLL" /D "NDEBUG" /D "WIN32" /D "PRE_RELEASE_DEMO" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /map /machine:I386 /out:"../Release/cgamex86.dll"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /map /machine:I386 /out:"d:\games\fakk2\test\cgamex86.dll"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "cgame - Win32 German Demo Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cgame___Win32_German_Demo_Release"
# PROP BASE Intermediate_Dir "cgame___Win32_German_Demo_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "cgame___Win32_German_Demo_Release"
# PROP Intermediate_Dir "cgame___Win32_German_Demo_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /Zi /O2 /D "_WINDOWS" /D "CGAME_DLL" /D "NDEBUG" /D "WIN32" /D "PRE_RELEASE_DEMO" /FR /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GR /GX /Zi /O2 /D "_WINDOWS" /D "CGAME_DLL" /D "NDEBUG" /D "WIN32" /D "PRE_RELEASE_DEMO" /D "GERMAN_VERSION" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /map /machine:I386 /out:"../Demo_Release/cgamex86.dll"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /map /machine:I386 /out:"d:\games\fakk2\test\cgamex86.dll"
# SUBTRACT LINK32 /debug

!ENDIF 

# Begin Target

# Name "cgame - Win32 Release"
# Name "cgame - Win32 Debug"
# Name "cgame - Win32 Demo Release"
# Name "cgame - Win32 German Demo Release"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\fgame\bg_misc.c
# End Source File
# Begin Source File

SOURCE=..\fgame\bg_pmove.c
# End Source File
# Begin Source File

SOURCE=.\cg_beam.cpp
# End Source File
# Begin Source File

SOURCE=.\cg_commands.cpp
# End Source File
# Begin Source File

SOURCE=.\cg_consolecmds.c
# End Source File
# Begin Source File

SOURCE=.\cg_drawtools.cpp
# End Source File
# Begin Source File

SOURCE=.\cg_ents.c
# End Source File
# Begin Source File

SOURCE=.\cg_lightstyles.cpp
# End Source File
# Begin Source File

SOURCE=.\cg_main.c
# End Source File
# Begin Source File

SOURCE=.\cg_marks.c
# End Source File
# Begin Source File

SOURCE=.\cg_modelanim.c
# End Source File
# Begin Source File

SOURCE=.\cg_player.cpp
# End Source File
# Begin Source File

SOURCE=.\cg_playerstate.c
# End Source File
# Begin Source File

SOURCE=.\cg_predict.c
# End Source File
# Begin Source File

SOURCE=.\cg_servercmds.c
# End Source File
# Begin Source File

SOURCE=.\cg_snapshot.c
# End Source File
# Begin Source File

SOURCE=.\cg_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\cg_specialfx.cpp
# End Source File
# Begin Source File

SOURCE=.\cg_swipe.cpp
# End Source File
# Begin Source File

SOURCE=.\cg_testemitter.cpp
# End Source File
# Begin Source File

SOURCE=.\cg_ui.cpp
# End Source File
# Begin Source File

SOURCE=.\cg_view.c
# End Source File
# Begin Source File

SOURCE=.\class.cpp
# End Source File
# Begin Source File

SOURCE=.\listener.cpp
# End Source File
# Begin Source File

SOURCE=..\fgame\q_math.c
# End Source File
# Begin Source File

SOURCE=..\fgame\q_mathsys.c
# End Source File
# Begin Source File

SOURCE=..\fgame\q_shared.c
# End Source File
# Begin Source File

SOURCE=.\script.cpp
# End Source File
# Begin Source File

SOURCE=.\str.cpp
# End Source File
# Begin Source File

SOURCE=..\win32\win_bounds.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\fgame\bg_local.h
# End Source File
# Begin Source File

SOURCE=..\fgame\bg_public.h
# End Source File
# Begin Source File

SOURCE=.\cg_commands.h
# End Source File
# Begin Source File

SOURCE=.\cg_local.h
# End Source File
# Begin Source File

SOURCE=.\cg_public.h
# End Source File
# Begin Source File

SOURCE=.\class.h
# End Source File
# Begin Source File

SOURCE=.\container.h
# End Source File
# Begin Source File

SOURCE=.\Linklist.h
# End Source File
# Begin Source File

SOURCE=.\listener.h
# End Source File
# Begin Source File

SOURCE=.\script.h
# End Source File
# Begin Source File

SOURCE=.\str.h
# End Source File
# Begin Source File

SOURCE=.\tr_types.h
# End Source File
# Begin Source File

SOURCE=.\vector.h
# End Source File
# End Group
# Begin Group "CTF"

# PROP Default_Filter ""
# Begin Group "CPP"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\cg_ctf_view.cpp
# End Source File
# Begin Source File

SOURCE=.\ctf_2d.cpp
# End Source File
# Begin Source File

SOURCE=.\ctf_cg_drawtools.cpp
# End Source File
# Begin Source File

SOURCE=.\CTF_ItemPickup.cpp
# End Source File
# Begin Source File

SOURCE=.\ctf_msg.cpp
# End Source File
# Begin Source File

SOURCE=.\ctf_reward.cpp
# End Source File
# Begin Source File

SOURCE=.\ctf_tech.cpp
# End Source File
# End Group
# Begin Group "H"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\cg_ctf_view.h
# End Source File
# Begin Source File

SOURCE=.\ctf_2d.h
# End Source File
# Begin Source File

SOURCE=.\ctf_cg_drawtools.h
# End Source File
# Begin Source File

SOURCE=.\CTF_ItemPickup.h
# End Source File
# Begin Source File

SOURCE=.\ctf_msg.h
# End Source File
# Begin Source File

SOURCE=.\ctf_reward.h
# End Source File
# Begin Source File

SOURCE=.\ctf_tech.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\cgame.def
# End Source File
# End Target
# End Project
