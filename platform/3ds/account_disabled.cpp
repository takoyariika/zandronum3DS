// Initial 3DS build has no account authentication. Fail before sending SRP data.
// This is a disabled feature, not an alternative cryptographic implementation.
#include "network/srp.h"
#include "i_system.h"
SRPUser *srp_user_new(SRP_HashAlgorithm, SRP_NGType, const char *, const unsigned char *, int, const char *, const char *, int)
{
    I_Error("Account login is unavailable in this 3DS build.");
    return NULL;
}
void srp_user_delete(SRPUser *) {}
int srp_user_is_authenticated(SRPUser *) { return 0; }
int srp_user_get_session_key_length(SRPUser *) { return 0; }
void srp_user_start_authentication(SRPUser *, const char **, const unsigned char **, int *)
{ I_Error("Account login is unavailable in this 3DS build."); }
void srp_user_process_challenge(SRPUser *, const unsigned char *, int, const unsigned char *, int, const unsigned char **, int *)
{ I_Error("Account login is unavailable in this 3DS build."); }
void srp_user_verify_session(SRPUser *, const unsigned char *)
{ I_Error("Account login is unavailable in this 3DS build."); }
