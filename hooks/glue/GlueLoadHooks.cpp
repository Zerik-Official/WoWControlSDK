#include "GlueLoadHooks.h"
#include "../base/DetourHelper.h"
#include "offsets/OffsetsLogin.h"
#include <vector>

namespace Hooks
{

static std::vector<void(*)()> s_glueXmlPostLoad;
static std::vector<void(*)()> s_glueXmlCharEnum;

static void LoadGlueXML_bulk()
{
    for (auto func : s_glueXmlPostLoad)
        func();
}

static void (*LoadGlueXML_orig)() = (decltype(LoadGlueXML_orig))Offsets::Login::LOAD_GLUE_XML;
static void __declspec(naked) LoadGlueXML_hk()
{
    __asm {
        pop ebx;
        mov esp, ebp;
        pop ebp;

        pushad;
        pushfd;
        call LoadGlueXML_bulk;
        popfd;
        popad;
        ret;
    }
}

void GlueXML::registerPostLoad(void(*func)()) { s_glueXmlPostLoad.push_back(func); }

static void LoadCharacters_bulk()
{
    for (auto func : s_glueXmlCharEnum)
        func();
}

static void (*LoadCharacters_orig)() = (decltype(LoadCharacters_orig))Offsets::Login::LOAD_CHARACTERS;
static void __declspec(naked) LoadCharacters_hk()
{
    __asm {
        add esp, 8;
        pop esi;

        pushad;
        pushfd;
        call LoadCharacters_bulk;
        popfd;
        popad;

        ret;
    }
}

void GlueXML::registerCharEnum(void(*func)()) { s_glueXmlCharEnum.push_back(func); }

void GlueLoad::Initialize()
{
    Detail::attach(LoadGlueXML_orig, LoadGlueXML_hk);
    Detail::attach(LoadCharacters_orig, LoadCharacters_hk);
}

void GlueLoad::Shutdown()
{
    Detail::detach(LoadGlueXML_orig, LoadGlueXML_hk);
    Detail::detach(LoadCharacters_orig, LoadCharacters_hk);
}

}
