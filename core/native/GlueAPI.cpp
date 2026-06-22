#include "GlueAPI.h"
#include "offsets/OffsetsLogin.h"
#include <cstdint>

namespace
{

void setAccepted(uintptr_t accepted, uintptr_t pending)
{
    *(uint32_t*)pending  = 0;
    *(uint32_t*)accepted = 1;
}

}

namespace WoW::Glue
{

void AcceptEULA()        { setAccepted(Offsets::Login::EULA_ACCEPTED, Offsets::Login::EULA_PENDING); }
void AcceptTOS()         { setAccepted(Offsets::Login::TOS_ACCEPTED, Offsets::Login::TOS_PENDING); }
void AcceptTermination() { setAccepted(Offsets::Login::TERM_ACCEPTED, Offsets::Login::TERM_PENDING); }
void AcceptScanning()    { setAccepted(Offsets::Login::SCAN_ACCEPTED, Offsets::Login::SCAN_PENDING); }
void AcceptContest()     { setAccepted(Offsets::Login::CONTEST_ACCEPTED, Offsets::Login::CONTEST_PENDING); }

void AcceptAll()
{
    AcceptEULA();
    AcceptTOS();
    AcceptTermination();
    AcceptScanning();
    AcceptContest();
}

}
