#include <stdio.h>
#include <stdint.h>
#include "ls_app_version.h"

char *listen_get_fw_version()
{
	return APP_VERSION;
}