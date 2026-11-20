#include "GlueLoadHooks.h"
#include <vector>

namespace Hooks
{

static std::vector<void(*)()> s_glueXmlPostLoad;
static std::vector<void(*)()> s_glueXmlCharEnum;

void GlueXML::registerPostLoad(void(*func)()) { s_glueXmlPostLoad.push_back(func); }
void GlueXML::registerCharEnum(void(*func)()) { s_glueXmlCharEnum.push_back(func); }

void GlueXML::notifyPostLoad()
{
    for (auto func : s_glueXmlPostLoad)
        func();
}

void GlueXML::notifyCharEnum()
{
    for (auto func : s_glueXmlCharEnum)
        func();
}

}
