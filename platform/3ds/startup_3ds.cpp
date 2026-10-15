#include <cstdlib>
#include "st_start.h"
#include "i_system.h"
#include "c_cvars.h"
#include "i_3ds.h"
FStartupScreen *StartScreen;
CVAR(Int, showendoom, 0, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
static void DeleteStartup() { delete StartScreen; StartScreen = NULL; }
FStartupScreen *FStartupScreen::CreateInstance(int max_progress)
{
    atterm(DeleteStartup);
    return new FStartupScreen(max_progress);
}
extern void I3DS_ReportExit(const char *);
void ST_Endoom() {
    if(I3DS_RequestLauncherReturn()) return;
    I3DS_ReportExit("Explicit engine/menu quit"); exit(0);
}
