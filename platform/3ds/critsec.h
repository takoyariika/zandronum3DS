// libctru mutex for Zandronum's FCriticalSection interface.
// Include only synchronization headers to avoid HID KEY_* macro collisions.
#ifndef CRITSEC_H
#define CRITSEC_H
extern "C" {
#include <3ds/types.h>
#include <3ds/synchronization.h>
}
class FCriticalSection
{
public:
    FCriticalSection() { LightLock_Init(&lock); }
    void Enter() { LightLock_Lock(&lock); }
    void Leave() { LightLock_Unlock(&lock); }
private:
    LightLock lock;
};
#endif
