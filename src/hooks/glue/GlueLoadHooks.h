#pragma once

namespace Hooks
{

namespace GlueXML
{
void registerPostLoad(void(*func)());
void registerCharEnum(void(*func)());

void notifyPostLoad();
void notifyCharEnum();
}

}
