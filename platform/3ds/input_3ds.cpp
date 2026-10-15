#include <3ds.h>
#include <cstdlib>
#include "doomdef.h"
#include "d_event.h"
#include "c_cvars.h"
#include "c_bind.h"
#include "m_joy.h"
#include "i_system.h"
#include "m_misc.h"
#include "m_argv.h"
#include "doomstat.h"
#include "d_player.h"
#include "a_sharedglobal.h"
#include "g_game.h"
#include "i_3ds.h"
#include "st_start.h"
#include "c_dispatch.h"
#include "menu/menu.h"
#include <malloc.h>
#include <sys/stat.h>
#include <cstdlib>
CVAR(Bool, use_mouse, false, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR(Bool, m_noprescale, false, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR(Bool, m_filter, false, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR(Float, touch_sensitivity_3ds, 12.f, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR(Int, native_controls_layout_3ds, 0, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
EXTERN_CVAR(Float, m_pitch)
EXTERN_CVAR(Float, m_yaw)
EXTERN_CVAR(Float, mouse_sensitivity)
EXTERN_CVAR(Bool, invertmouse)
EXTERN_CVAR(Bool, cl_useskulltagmouse)
EXTERN_CVAR(Bool, cl_run)
CVAR(Int, native_touch_layout_3ds, 0, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
#include "bottom_ui_3ds.inc"
void I_ProcessJoysticks();
struct Button { u32 mask; int key; const char *command; };
static const Button buttons[] = {
    {KEY_A, KEY_PAD_A, "+attack"}, {KEY_B, KEY_PAD_B, "+jump"},
    {KEY_X, KEY_PAD_X, "+use"}, {KEY_Y, KEY_PAD_Y, "+crouch"},
    {KEY_L, KEY_PAD_LSHOULDER, "+attack"}, {KEY_R, KEY_PAD_RSHOULDER, "+altattack"},
    {KEY_ZL, KEY_PAD_LTRIGGER, "weapprev"}, {KEY_ZR, KEY_PAD_RTRIGGER, "weapnext"},
    {KEY_START, KEY_PAD_START, "menu_main"}, {KEY_SELECT, KEY_PAD_BACK, "togglemap"},
    {KEY_DUP, KEY_PAD_DPAD_UP, "weapnext"}, {KEY_DDOWN, KEY_PAD_DPAD_DOWN, "weapprev"},
    {KEY_DLEFT, KEY_PAD_DPAD_LEFT, "+moveleft"}, {KEY_DRIGHT, KEY_PAD_DPAD_RIGHT, "+moveright"}
};
static unsigned inputTestStart;
static bool irReady;
static unsigned inputElapsed() { return inputTestStart ? I_MSTime()-inputTestStart : 0; }
void I3DS_ReadSticks(int *x, int *y, int *rx, int *ry)
{
    circlePosition cp={}, cs={}; hidCircleRead(&cp);
    if (irReady) irrstCstickRead(&cs);
    *x=cp.dx; *y=cp.dy; *rx=cs.dx; *ry=cs.dy;
    if (Args->CheckParm("-3ds-test-input") && inputTestStart)
    {
        *x=*y=*rx=*ry=0;
        unsigned elapsed=inputElapsed();
        if (elapsed>=6000 && elapsed<7500) *y=156;
        if (elapsed>=9000 && elapsed<10500) { *rx=100; *ry=60; }
        if(elapsed>=10500 && elapsed<11000) {
            *x=100;
            static bool logged;
            if(!logged) { APlayerPawn *p=players[consoleplayer].mo; if(p) Printf("3DS PAD YAW TEST before x=%d y=%d angle=%u\n",p->x,p->y,p->angle); logged=true; }
        }
        if(elapsed>=11000) {
            static bool logged;
            if(!logged) { APlayerPawn *p=players[consoleplayer].mo; if(p) Printf("3DS PAD YAW TEST after x=%d y=%d angle=%u\n",p->x,p->y,p->angle); logged=true; }
        }
    }
}
void I_StartFrame() {}
void I_StartTic()
{
    // Printing or APT/audio restoration can redraw while input is being polled.
    // Never recursively initialize controls or process the same HID edges.
    static bool inTic;
    if(inTic) return;
    inTic=true;
    struct TicGuard { bool &busy; ~TicGuard() { busy=false; } } guard={inTic};
    struct InputTiming {
        unsigned start;
        InputTiming() : start(I_MSTime()) {}
        ~InputTiming() { extern void I3DS_RecordCost(unsigned,unsigned); I3DS_RecordCost(0,I_MSTime()-start); }
    } timing;
    svcSleepThread(200000);
    if (!I3DS_PumpEvents()) exit(0);
    I3DS_MemoryTick();
    static bool defaults;
    if (!defaults)
    {
        defaults=true;
        if(native_touch_layout_3ds<1) { cl_run=true; native_touch_layout_3ds=1; }
        irReady=R_SUCCEEDED(irrstInit());
        if (irReady) atterm(irrstExit);
        bool migrate=native_controls_layout_3ds<1;
        for (auto &b : buttons)
        {
            FString old=Bindings.GetBinding(b.key);
            if (b.command && (migrate || old.IsEmpty())) Bindings.SetBind(b.key, b.command);
            if(migrate) DoubleBindings.SetBind(b.key,"");
        }
        extern void I3DS_ApplyControlLayout(bool); I3DS_ApplyControlLayout(migrate);
        if(migrate) native_controls_layout_3ds=1;
        Printf("3DS controls: Circle Pad forward/yaw, D-pad weapons/strafe, C-stick IRRST=%s\n", irReady?"ready":"unavailable");
        Printf("3DS D-pad bindings: up=%s, right=%s\n", Bindings.GetBinding(KEY_PAD_DPAD_UP).GetChars(), Bindings.GetBinding(KEY_PAD_DPAD_RIGHT).GetChars());
    }
    hidScanInput();
    if (irReady) irrstScanInput();
    static u32 previous;
    static touchPosition previousTouch;
    static bool touching;
    static unsigned generation;
    if (generation!=I3DS_ResumeGeneration()) {
        for (auto &b : buttons) if (previous & b.mask) {
            event_t e={}; e.type=EV_KeyUp; e.data1=b.key; D_PostEvent(&e);
        }
        previous=0; touching=false; I3DS_ResetBottomTouch(); generation=I3DS_ResumeGeneration();
    }
    u32 held = hidKeysHeld();
    // Input is polled during map loading too. Timed replays start only once
    // the new map is playable, not against the previous map's actors/timer.
    bool testGameplayReady=gamestate==GS_LEVEL && gameaction==ga_nothing && level.maptime>0 && players[consoleplayer].mo;
    if(Args->CheckParm("-3ds-test-return-launcher") && (gamestate==GS_DEMOSCREEN || gamestate==GS_TITLELEVEL)) {
        extern int restart;
        static int titleRestart;
        if(restart>titleRestart) { titleRestart=restart; C_DoCommand("map MAP02"); }
    }
    if(Args->CheckParm("-3ds-test-return-launcher") && testGameplayReady) {
        extern int restart;
        static int lastRestart=-1;
        static unsigned started;
        if(lastRestart!=restart) { lastRestart=restart; started=I_MSTime(); }
        if(I_MSTime()-started>=2500) {
            Printf("3DS RETURN TEST: quitting playable game %d map=%s\n",restart+1,level.mapname);
            ST_Endoom();
        }
    }
    if(Args->CheckParm("-3ds-test-join") && testGameplayReady) {
        static unsigned started;
        static bool completed;
        if(!started) {
            if(!players[consoleplayer].bSpectating) I_FatalError("Join test must start as spectator");
            started=I_MSTime();
        }
        unsigned elapsed=I_MSTime()-started;
        if(!completed) {
            if((elapsed>=1000 && elapsed<1350) || (elapsed>=3000 && elapsed<3350)) held|=KEY_A;
            if(!players[consoleplayer].bSpectating) { completed=true; Printf("3DS JOIN TEST: PASS A menu and confirmation joined game\n"); }
            else if(elapsed>=12000) I_FatalError("A-button join regression: still spectating");
        }
        testGameplayReady=completed;
    }
    if(Args->CheckParm("-3ds-test-network") && (gamestate==GS_FULLCONSOLE || testGameplayReady) && !Args->CheckParm("-3ds-test-quit")) {
        static unsigned start;
        if(!start) start=I_MSTime();
        if(I_MSTime()-start>=20000) {
            Printf("3DS NETWORK TEST: normal exit after connection attempt\n"); ST_Endoom();
        }
    }
    if(Args->CheckParm("-3ds-test-menu") && (gamestate==GS_DEMOSCREEN || gamestate==GS_TITLELEVEL)) {
        static unsigned started, stage;
        if(!started) started=I_MSTime();
        unsigned t=I_MSTime()-started;
        if(t>=3000 && t<3200) held|=KEY_START;
        if(t>=6500 && t<6700) held|=KEY_A;
        if(t>=10000 && t<10200) held|=KEY_DDOWN;
        unsigned next=t>=11500?3:t>=8500?2:t>=5000?1:0;
        if(next>stage) {
            stage=next;
            Printf("3DS MENU TEST stage=%u active=%d class=%s heap=%u KiB\n",stage,(int)menuactive,DMenu::CurrentMenu?DMenu::CurrentMenu->GetClass()->TypeName.GetChars():"none",(unsigned)mallinfo().uordblks/1024);
            const char *paths[]={"","sdmc:/3ds/zandronum/screenshots/test-menu-main.png","sdmc:/3ds/zandronum/screenshots/test-menu-class.png","sdmc:/3ds/zandronum/screenshots/test-menu-move.png"};
            M_ScreenShot(paths[stage]);
        }
        if(t>=15000) { Printf("3DS MENU TEST completed.\n"); M_ClearMenus(); ST_Endoom(); exit(0); }
    }
    if(Args->CheckParm("-3ds-test-fatal") && testGameplayReady) {
        static unsigned started=I_MSTime();
        if(I_MSTime()-started>=2000) I_FatalError("Native fatal display test\nThis is an intentional diagnostic error.");
    }
    if(Args->CheckParm("-3ds-test-round")) {
        static bool intermission;
        static unsigned rounds;
        if(gamestate==GS_INTERMISSION && !intermission) {
            intermission=true; ++rounds;
            Printf("3DS ROUND TEST: intermission %u heap=%u KiB\n",rounds,(unsigned)mallinfo().uordblks/1024);
        }
        if(intermission && testGameplayReady) {
            intermission=false;
            Printf("3DS ROUND TEST: resumed %u map=%s heap=%u KiB\n",rounds,level.mapname,(unsigned)mallinfo().uordblks/1024);
            if(rounds>=2) { Printf("3DS ROUND TEST PASS: two results and next rounds\n"); Printf("3DS NETWORK TEST: normal exit after connection attempt\n"); Printf("3DS QUIT TEST: normal menu exit path\n"); ST_Endoom(); }
        }
    }
    if (irReady) held |= irrstKeysHeld();
    bool testTouch=Args->CheckParm("-3ds-test-touch") && inputTestStart && inputElapsed()>=18000 && inputElapsed()<18600;
    if(testTouch) held |= KEY_TOUCH;
    if(Args->CheckParm("-3ds-test-audio") && testGameplayReady) {
        static unsigned start, stage;
        if(!start) start=I_MSTime();
        unsigned elapsed=I_MSTime()-start;
        if(stage<4 && elapsed>=3000+stage*1500) {
            char commands[][32]={"playsound diag/wav","playsound diag/ogg","playsound diag/mp3","playsound diag/flac"};
            AddCommandString(commands[stage++]);
        }
        if(stage==4 && elapsed>=10000) { AddCommandString("changemus DIAGMUS"); ++stage; }
        if(stage==5 && elapsed>=18000) {
            ++stage; Printf("3DS AUDIO DECODER TEST completed.\n");
            if(!Args->CheckParm("-3ds-test-shotgun")) { ST_Endoom(); exit(0); }
        }
    }
    if(Args->CheckParm("-3ds-test-shotgun") && testGameplayReady) {
        static unsigned start, lastReport; static bool equipped, finished;
        if(!start) start=I_MSTime();
        unsigned elapsed=I_MSTime()-start;
        players[consoleplayer].cheats |= CF_GODMODE;
        if(elapsed>=6000 && !equipped) {
            AddCommandString("give NewShotgun; give Shell 50; use NewShotgun"); equipped=true;
        }
        if(elapsed>=8000 && elapsed<12000) held|=KEY_DUP;
        if(elapsed>=15000 && elapsed<135000) held|=KEY_ZR;
        if(elapsed-lastReport>=10000) {
            lastReport=elapsed; AddCommandString("give Shell 50");
            APlayerPawn *pawn=players[consoleplayer].mo;
            Printf("3DS SHOTGUN TEST elapsed=%u heap=%u KiB x=%d y=%d\n",elapsed,mallinfo().uordblks/1024,pawn?pawn->x:0,pawn?pawn->y:0);
        }
        if(elapsed>=138000 && !finished) {
            finished=true; Printf("3DS SHOTGUN TEST completed: 120 seconds of attack input.\n");
            M_ScreenShot("sdmc:/3ds/zandronum/screenshots/test-input.png");
            ST_Endoom(); exit(0);
        }
    }
    if (Args->CheckParm("-3ds-test-input") && testGameplayReady)
    {
        if (!inputTestStart) inputTestStart=I_MSTime();
        unsigned elapsed=inputElapsed();
        if (Args->CheckParm("-3ds-test-combat"))
        {
            players[consoleplayer].cheats |= CF_GODMODE;
            if (elapsed>=18000 && elapsed<30000) held |= KEY_L;
            if (elapsed>=30000 && elapsed<40000) held |= KEY_R;
            static bool combatDone;
            if (elapsed>=43000 && !combatDone)
            {
                Printf("3DS INPUT COMBAT TEST completed; heap-used=%d KiB\n", mallinfo().uordblks/1024);
                M_ScreenShot("sdmc:/3ds/zandronum/screenshots/test-input.png"); combatDone=true;
            }
        }
        if (elapsed>=12000 && elapsed<13500) held |= KEY_DLEFT;
        if (elapsed>=15000 && elapsed<16500) held |= KEY_DRIGHT;
        if(Args->CheckParm("-3ds-test-controls")) {
            const u32 masks[]={KEY_A,KEY_B,KEY_Y,KEY_X,KEY_L,KEY_R};
            const char *names[]={"A attack","B jump","Y crouch","X use","L attack","R altattack"};
            FButtonStatus *states[]={&Button_Attack,&Button_Jump,&Button_Crouch,&Button_Use,&Button_Attack,&Button_AltAttack};
            static unsigned checked;
            for(unsigned i=0;i<6;++i) {
                unsigned start=20000+i*2000;
                if(elapsed>=start && elapsed<start+1000) held|=masks[i];
                if(elapsed>=start+500 && !(checked & (1u<<i))) {
                    if(!states[i]->bDown) I_FatalError("Control regression: %s not active",names[i]);
                    Printf("3DS CONTROL TEST PASS: %s\n",names[i]); checked|=1u<<i;
                }
            }
            const u32 weaponMasks[]={KEY_ZL,KEY_ZR,KEY_DUP,KEY_DDOWN};
            const char *weaponNames[]={"ZL previous","ZR next","D-pad up next","D-pad down previous"};
            static unsigned weaponChecked;
            for(unsigned i=0;i<4;++i) {
                unsigned start=32000+i*2500;
                if(elapsed>=start && elapsed<start+1000) held|=weaponMasks[i];
                if(elapsed>=start+500 && !(weaponChecked & (1u<<i))) {
                    AWeapon *weapon=players[consoleplayer].PendingWeapon;
                    if(!weapon || weapon==WP_NOCHANGE) weapon=players[consoleplayer].ReadyWeapon;
                    Printf("3DS WEAPON TEST: %s -> %s\n",weaponNames[i],weapon?weapon->GetClass()->TypeName.GetChars():"none");
                    weaponChecked|=1u<<i;
                }
            }
        }
        static unsigned phase;
        unsigned next=elapsed>=17500?4:elapsed>=14000?3:elapsed>=11000?2:elapsed>=8000?1:0;
        if (next>phase)
        {
            APlayerPawn *pawn=players[consoleplayer].mo;
            if (pawn) Printf("3DS INPUT TEST phase=%u x=%d y=%d angle=%u pitch=%d\n", next,pawn->x,pawn->y,pawn->angle,pawn->pitch);
            phase=next;
            if (phase==4 && !Args->CheckParm("-3ds-test-combat")) { M_ScreenShot("sdmc:/3ds/zandronum/screenshots/test-input.png"); Printf("3DS INPUT TEST completed.\n"); }
        }
    }
    u32 changed = held ^ previous;
    for (auto &b : buttons)
        if (changed & b.mask)
        {
            event_t e = {};
            e.type = held & b.mask ? EV_KeyDown : EV_KeyUp;
            e.data1 = b.key;
            D_PostEvent(&e);
        }
    previous = held;
    I_ProcessJoysticks();
    if (Args->CheckParm("-3ds-test-title") && (gamestate==GS_TITLELEVEL || gamestate==GS_DEMOSCREEN)) {
        static unsigned titleStart=I_MSTime();
        static bool titleCaptured, transitioned;
        static bool musicStopped;
        if(Args->CheckParm("-3ds-test-title-stopmusic") && !musicStopped && I_MSTime()-titleStart>=45000) {
            char command[]="stopmus"; AddCommandString(command); musicStopped=true;
            Printf("3DS TITLE MUSIC STOP TEST: heap %u KiB\n",(unsigned)mallinfo().uordblks/1024);
        }
        if(!titleCaptured && I_MSTime()-titleStart>=5000) {
            M_ScreenShot("sdmc:/3ds/zandronum/screenshots/test-title.png");
            Printf("3DS TITLE TEST: heap-used=%d KiB\n",mallinfo().uordblks/1024); titleCaptured=true;
        }
        const char *soak=Args->CheckValue("-3ds-test-title-soak");
        unsigned transitionDelay=soak ? (unsigned)atoi(soak)*1000u : 10000u;
        if(soak && !Args->CheckParm("-3ds-test-transition") && I_MSTime()-titleStart>=transitionDelay) {
            Printf("3DS TITLE SOAK completed: %u seconds.\n",transitionDelay/1000);
            ST_Endoom(); exit(0);
        }
        if(Args->CheckParm("-3ds-test-transition") && !transitioned && I_MSTime()-titleStart>=transitionDelay) {
            Printf("3DS TRANSITION TEST: title to BYOCTEST\n");
            char command[]="map BYOCTEST"; AddCommandString(command); transitioned=true;
        }
    }
    if ((Args->CheckParm("-3ds-test-quit") || Args->CheckParm("-3ds-test-sleep")) && !Args->CheckParm("-3ds-test-return-launcher") && testGameplayReady) {
        static unsigned start=I_MSTime();
        static bool slept;
        if(!slept && I_MSTime()-start>=6000 && Args->CheckParm("-3ds-test-sleep")) { I3DS_TestSleep(); slept=true; }
        if (I_MSTime()-start >= (Args->CheckParm("-3ds-test-input")?46000u:7000u) && Args->CheckParm("-3ds-test-quit") && !Args->CheckParm("-3ds-test-round")) {
            if(Args->CheckParm("-3ds-test-network")) Printf("3DS NETWORK TEST: normal exit after connection attempt\n");
            Printf("3DS QUIT TEST: normal menu exit path\n"); ST_Endoom();
        }
    }
    touchPosition sampledTouch; hidTouchRead(&sampledTouch);
    if(testTouch) {
        sampledTouch.px=100+(inputElapsed()-18000)/10;
        sampledTouch.py=100-(inputElapsed()-18000)/20;
    }
    bool bottomConsumed;
    if(Args->CheckParm("-3ds-test-bottom") && testGameplayReady) { I3DS_TestBottom(); bottomConsumed=true; }
    else bottomConsumed=I3DS_BottomTouch(held & KEY_TOUCH,&sampledTouch);
    // Buttons own their complete touch gesture; they never turn the camera.
    if ((held & KEY_TOUCH) && !bottomConsumed && gamestate==GS_LEVEL && menuactive==MENU_Off)
    {
        touchPosition touch=sampledTouch;
        if(testTouch) {
            static bool centered;
            if(!centered && players[consoleplayer].mo) { players[consoleplayer].mo->pitch=0; centered=true; }
            touch.px=100+(inputElapsed()-18000)/10; touch.py=100-(inputElapsed()-18000)/20;
        }
        if (touching)
        {
            if(testTouch) {
                static bool started;
                if(!started && players[consoleplayer].mo) {
                    Printf("3DS TOUCH START TEST: pitch=%d invert=%d\n",players[consoleplayer].mo->pitch,(int)(bool)invertmouse);
                    started=true;
                }
            }
            event_t motion={}; motion.type=EV_Mouse;
            float gain=touch_sensitivity_3ds;
            if (gain<0.25f) gain=0.25f; if (gain>16.f) gain=16.f;
            motion.x=((int)touch.px-(int)previousTouch.px)*gain;
            // Mouse Y increases upwards; touchscreen coordinates increase downwards.
            motion.y=((int)previousTouch.py-(int)touch.py)*gain;
            // This surface is always a camera, including when PC mouse-look
            // is disabled or a strafe modifier is held. Engine limits remain.
            int look=int(motion.y * m_pitch * mouse_sensitivity * 16.0);
            if(invertmouse) look=-look;
            G_AddViewPitch(look, !cl_useskulltagmouse);
            G_AddViewAngle(int(motion.x * m_yaw * mouse_sensitivity * 8.0), !cl_useskulltagmouse);
            if(testTouch) {
                static bool logged;
                if(!logged && (motion.x || motion.y)) { Printf("3DS TOUCH TEST: gain=%.2f dx=%d dy=%d\n",gain,motion.x,motion.y); logged=true; }
            }
        }
        previousTouch=touch; touching=true;
    }
    else touching=false;
    if(Args->CheckParm("-3ds-test-touch") && inputTestStart && inputElapsed()>=19000) {
        static bool reported;
        APlayerPawn *pawn=players[consoleplayer].mo;
        if(!reported && pawn) { Printf("3DS TOUCH VIEW TEST: angle=%u pitch=%d\n",pawn->angle,pawn->pitch); reported=true; }
    }
    // Opt-in unattended emulator check. Normal play never takes screenshots here.
    static unsigned testStart;
    static bool captured;
    if (Args->CheckParm("-3ds-test") && testGameplayReady)
    {
        if (!testStart) testStart = I_MSTime();
        if (!captured && I_MSTime()-testStart >= 5000)
        {
            M_ScreenShot("sdmc:/3ds/zandronum/screenshots/test-map.png");
            Printf("3DS TEST: map running; framebuffer captured.\n");
            APlayerPawn *pawn = players[consoleplayer].mo;
            if (pawn) Printf("3DS TEST: pawn=%s x=%d y=%d health=%d heap-used=%d KiB\n", pawn->GetClass()->TypeName.GetChars(), pawn->x, pawn->y, pawn->health, mallinfo().uordblks/1024);
            captured = true;
        }
        static unsigned stage;
        unsigned elapsed = I_MSTime()-testStart;
        auto key = [](int code, bool down) { event_t e={}; e.type=down?EV_KeyDown:EV_KeyUp; e.data1=code; D_PostEvent(&e); };
        if (Args->CheckParm("-3ds-test-play"))
        {
            if (!stage && elapsed >= 6000) { key(0x11, true); key(KEY_PAD_RTRIGGER, true); stage=1; }
            if (stage == 1 && elapsed >= 7500) { key(0x11, false); key(KEY_PAD_RTRIGGER, false); stage=2; }
            if (stage == 2 && elapsed >= 9000)
            {
                APlayerPawn *pawn=players[consoleplayer].mo;
                if (pawn) Printf("3DS TEST: after input x=%d y=%d health=%d\n", pawn->x, pawn->y, pawn->health);
                G_SaveGame("sdmc:/3ds/zandronum/save/test.zds", "3DS emulator regression"); stage=3;
            }
            if (stage == 3 && elapsed >= 12000)
            {
                struct stat saved;
                if (stat("sdmc:/3ds/zandronum/save/test.zds", &saved) == 0) {
                    Printf("3DS TEST: saved %ld bytes; reloading.\n", (long)saved.st_size);
                    G_LoadGame("sdmc:/3ds/zandronum/save/test.zds", true);
                } else I_Error("3DS test save file was not created");
                stage=4;
            }
            if (stage == 4 && elapsed >= 15000)
            {
                M_ScreenShot("sdmc:/3ds/zandronum/screenshots/test-play.png");
                APlayerPawn *pawn=players[consoleplayer].mo;
                if (pawn) Printf("3DS TEST: after load x=%d y=%d health=%d\n", pawn->x, pawn->y, pawn->health);
                Printf("3DS TEST: play/save/load completed.\n"); stage=5;
            }
        }
    }
}
void I_SetMouseCapture() {}
void I_ReleaseMouseCapture() {}
void I_PutInClipboard(const char *) {}
FString I_GetFromClipboard(bool) { return FString(); }
bool I_SetCursor(FTexture *) { return false; }
