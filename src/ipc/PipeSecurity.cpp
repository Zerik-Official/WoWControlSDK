#include "PipeSecurity.h"
#include <sddl.h>
#include <string>
#include <vector>

namespace PipeSecurity {

namespace {

std::string currentUserSid()
{
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
        return std::string();

    DWORD size = 0;
    GetTokenInformation(token, TokenUser, nullptr, 0, &size);
    std::vector<BYTE> buffer(size);
    std::string sid;
    if (size && GetTokenInformation(token, TokenUser, buffer.data(), size, &size))
    {
        char* text = nullptr;
        if (ConvertSidToStringSidA(reinterpret_cast<TOKEN_USER*>(buffer.data())->User.Sid, &text))
        {
            sid = text;
            LocalFree(text);
        }
    }
    CloseHandle(token);
    return sid;
}

SECURITY_ATTRIBUTES* build()
{
    static SECURITY_ATTRIBUTES attributes{};
    std::string sid = currentUserSid();
    if (sid.empty())
        return nullptr;

    std::string sddl = "D:P(A;;GA;;;SY)(A;;GA;;;" + sid + ")";
    PSECURITY_DESCRIPTOR descriptor = nullptr;
    if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(sddl.c_str(), SDDL_REVISION_1, &descriptor, nullptr))
        return nullptr;

    attributes.nLength              = sizeof(attributes);
    attributes.lpSecurityDescriptor = descriptor;
    attributes.bInheritHandle       = FALSE;
    return &attributes;
}

}

SECURITY_ATTRIBUTES* attributes()
{
    static SECURITY_ATTRIBUTES* cached = build();
    return cached;
}

}
