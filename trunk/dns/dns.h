/* dns.h */

#include "public.h"



char *progname;
char cli_user_msg[MESSAGE_LENGTH];

struct s_dns
{
	char dnssrv[IP_LENGTH + 1];
	char dnssrv2[IP_LENGTH +1];
	char hostname[NAME_LENGTH +1];
}dns;

int cli_dns_init();
int cli_dns_usage();
int cli_dns_exit();

