#pragma once

namespace Hooks
{

namespace GlueXML
{
void registerPostLoad(void(*func)());
void registerCharEnum(void(*func)());
}

namespace GlueLoad
{
void Initialize();
void Shutdown();
}

}
