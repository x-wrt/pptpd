/*
 * $Id: pptpd-logwtmp.c,v 1.6 2013/02/07 00:37:39 quozl Exp $
 * pptpd-logwtmp.c - pppd plugin to update wtmp for a pptpd user
 *
 * Copyright 2004 James Cameron.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */
#include <unistd.h>
#include <utmp.h>
#include <string.h>
#include <pppd/pppd.h>
#include <pppd/options.h>
#include <linux/limits.h>
#include "patchlevel.h"

char pppd_version[] = VERSION;

static char pptpd_original_ip[PATH_MAX+1];
static bool pptpd_logwtmp_strip_domain = 0;

static option_t options[] = {
  { "pptpd-original-ip", o_string, pptpd_original_ip,
    "Original IP address of the PPTP connection",
    OPT_STATIC, NULL, PATH_MAX },
  { "pptpd-logwtmp-strip-domain", o_bool, &pptpd_logwtmp_strip_domain,
    "Strip domain from username before logging", OPT_PRIO | 1 },
  { NULL }
};

static const char *reduce(const char *user)
{
  char *sep;
  if (!pptpd_logwtmp_strip_domain) return user;

  sep = strstr(user, "//"); /* two slash */
  if (sep != NULL) user = sep + 2;
  sep = strstr(user, "\\"); /* or one backslash */
  if (sep != NULL) user = sep + 1;
  return user;
}

static void ip_up(void *opaque, int arg)
{
  const char *user = reduce(ppp_peer_authname(NULL, 0));
  if (debug_on())
    notice("pptpd-logwtmp.so ip-up %s %s %s", ppp_ifname(), user,
	   pptpd_original_ip);
  logwtmp(ppp_ifname(), user, pptpd_original_ip);
}

static void ip_down(void *opaque, int arg)
{
  if (debug_on())
    notice("pptpd-logwtmp.so ip-down %s", ppp_ifname());
  logwtmp(ppp_ifname(), "", "");
}

void plugin_init(void)
{
  ppp_add_options(options);
  ppp_add_notify(NF_IP_UP, ip_up, NULL);
  ppp_add_notify(NF_IP_DOWN, ip_down, NULL);
  if (debug_on())
    notice("pptpd-logwtmp: $Version$");
}
