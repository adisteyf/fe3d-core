#include "../src/fe-render-api.c"
#include <string.h>

FeRenderAPI fe_renderapi_version_debugblank(void)
{
  FeRenderAPI ferapi;
  strcat(ferapi.api_name, "DebugBlank");
  strcat(ferapi.desc, "DebugBlank v0.0.0 test description");
  ferapi.major = 0;
  ferapi.minor = 1;
  ferapi.patch = 0;

  return ferapi;
}

