#pragma once
void I3DS_FatalScreen(const char *);
void I3DS_ReportExit(const char *);
bool I3DS_ConsoleActive();
bool I3DS_PumpEvents();
unsigned I3DS_ResumeGeneration();
void I3DS_AdjustTimer(unsigned long long);
void I3DS_TestSleep();
void I3DS_MemoryTick();
inline void I3DS_HomeTraceThread() {}
inline void I3DS_HomeTrace(const char *) {}
void I3DS_WaitFrame();
void I3DS_LoadingText(const char *);
void I3DS_EndLoadingDisplay();
void I3DS_ReadSticks(int *, int *, int *, int *);
bool I3DS_Diagnostics();
bool I3DS_NetworkEnabled();
bool I3DS_GameDisplayActive();
void I3DS_BottomLog(const char *);
void I3DS_DrawBottom();
bool I3DS_RequestLauncherReturn();
void I3DS_RestartLauncher();
void I3DS_ResetBottomScreen();
