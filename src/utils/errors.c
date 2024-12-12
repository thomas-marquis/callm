#include "errors.h"
#include "logging.h"

char *
CallmStatusCode_string (CallmStatusCode code)
{
  if (code == ERROR)
    {
      return "ERROR";
    }
  else if (code == OK)
    {
      return "OK";
    }
  else if (code == NOT_IMPLEMENTED)
    {
      return "NOT_IMPLEMENTED";
    }
  else
    {
      LOGF_ERROR ("%u: unknown code", code);
    }

  return NULL;
}
