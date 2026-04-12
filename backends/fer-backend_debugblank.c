#include "../fe-api/render/fe-render-api.c"
#include <string.h>
/* debugblank v0.1.0+feR0.1.0
 * debug backend for fe-render-api
 */

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

