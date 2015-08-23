/* command "dns" */

#include "dns.h"

int
main(int argc, char **argv)
{
	int rc, nrow, ncol;
	sqlite *dbname;
	char *errmsg = NULL, **result = NULL;
	struct in_addr in_ip;
	FILE *fp1, *fp2;
	char line[256];

/* get program name */
	progname = cli_get_progname(argv[0]);

/* print version information */
	cli_print_version();

/* initialize */
	cli_dns_init();

/* open configuration database */
	if((dbname = sqlite_open(DB_CONF, 0, &errmsg)) == NULL)
	{
		cli_print_debug_msg("Error: %s\n", errmsg);
		exit(3);
	}

/* parse command line */
	if(argc < 2)
	{
		cli_dns_usage();
		cli_dns_exit(1);
	}
	
	if(!strcmp(argv[1], "set") && (argc == 4 || argc == 5) && !strcmp(argv[2], "ip"))
	{
		if(cli_check_keyword("<single_ip>", argv[3]))
		{
			cli_get_user_msg("public", 1, cli_user_msg);
			printf(cli_user_msg, argv[3]);
			printf("\n");
			cli_dns_exit(1);
		}
		inet_aton(argv[3], &in_ip);
		strcpy(dns.dnssrv, inet_ntoa(in_ip));
		
		if(argc == 5)
		{
			if(cli_check_keyword("<single_ip>", argv[4]))
			{
				cli_get_user_msg("public", 1, cli_user_msg);
				printf(cli_user_msg, argv[4]);
				printf("\n");
				cli_dns_exit(1);
			}
			inet_aton(argv[4], &in_ip);
			strcpy(dns.dnssrv2, inet_ntoa(in_ip));
		}
		
		if((fp1 = fopen(FILE_RESOLV_CONF, "r")) == NULL)
		{
			cli_print_debug_msg("Error: open file \"%s\"\n", FILE_RESOLV_CONF);
			cli_dns_exit(3);
		}
		flock(fileno(fp1), LOCK_EX);
		if((fp2 = fopen(FILE_RESOLV_CONF_TEMP, "w+")) == NULL)
		{
			cli_print_debug_msg("Error: open file \"%s\"\n", FILE_RESOLV_CONF_TEMP);
			cli_dns_exit(3);
		}
		flock(fileno(fp2), LOCK_EX);
		while(fgets(line, 256, fp1) != NULL)
		{
			if(strstr(line, "domain") == line)
			{
				fputs(line, fp2);
			}
		}
		if(strlen(dns.dnssrv))
		{
			sprintf(line, "nameserver\t%s\n", dns.dnssrv);
			fputs(line, fp2);
		}
		if(strlen(dns.dnssrv2))
		{
			sprintf(line, "nameserver\t%s\n", dns.dnssrv2);
			fputs(line, fp2);
		}
		flock(fileno(fp1), LOCK_UN);
		fclose(fp1);
		flock(fileno(fp2), LOCK_UN);
		fclose(fp2);
		remove(FILE_RESOLV_CONF);
		rename(FILE_RESOLV_CONF_TEMP, FILE_RESOLV_CONF);
		
		rc = sqlite_exec_printf(dbname, "update %s set dnssrv = %Q, dnssrv2 = %Q", 0, 0, 0, TABLE_DNS, dns.dnssrv, dns.dnssrv2);
		if(rc != SQLITE_OK)
		{
			cli_dns_exit(3);
		}
		
		if(strlen(dns.dnssrv2))
		{
			fw_log_write(FWLOG_DEV_MNG, LOG_NOTICE, "mod=%s act=set ip=\"%s %s\" result=0", progname, dns.dnssrv, dns.dnssrv2);
		}
		else
		{
			fw_log_write(FWLOG_DEV_MNG, LOG_NOTICE, "mod=%s act=set ip=%s result=0", progname, dns.dnssrv);
		}
	}

	else if(!strcmp(argv[1], "unset") && argc == 2)
	{
		if((fp1 = fopen(FILE_RESOLV_CONF, "w+")) == NULL)
		{
			cli_print_debug_msg("Error: open file \"%s\"\n", FILE_RESOLV_CONF);
			cli_dns_exit(3);
		}
		fclose(fp1);
		
		
		rc = sqlite_exec_printf(dbname, "update %s set dnssrv = '', dnssrv2 = ''", 0, 0, 0, TABLE_DNS);
		if(rc != SQLITE_OK)
		{
			cli_dns_exit(3);
		}
		
		fw_log_write(FWLOG_DEV_MNG, LOG_NOTICE, "mod=%s act=unset result=0", progname);
	}
	else if(!strcmp(argv[1], "show") && argc == 2)
	{
		rc = sqlite_get_table_printf(dbname, "select * from %s", &result, &nrow, &ncol, &errmsg, TABLE_DNS);
		if(rc != SQLITE_OK)
		{
			cli_dns_exit(3);
		}
		if(nrow > 0)
		{
			if(strlen(result[ncol]) || strlen(result[ncol + 1]))
			{
				printf("DNS 1: %s\n", result[ncol]);
				printf("DNS 2: %s\n", result[ncol + 1]);
			}
			else
			{
				printf("DNS are not set\n");
			}			
		}
	}

	else if(!strcmp(argv[1], "startup") && argc == 2)
	{
		rc = sqlite_get_table_printf(dbname, "select * from %s", &result, &nrow, &ncol, &errmsg, TABLE_DNS);
		if(rc != SQLITE_OK)
		{
			cli_dns_exit(3);
		}
		if(nrow > 0)
		{
			strcpy(dns.dnssrv, result[ncol]);
			strcpy(dns.dnssrv2, result[ncol + 1]);
		}
		
		if(strlen(dns.dnssrv))
		{
			if((fp1 = fopen(FILE_RESOLV_CONF, "r")) == NULL)
			{
				cli_print_debug_msg("Error: open file \"%s\"\n", FILE_RESOLV_CONF);
				cli_dns_exit(3);
			}
			flock(fileno(fp1), LOCK_EX);
			if((fp2 = fopen(FILE_RESOLV_CONF_TEMP, "w+")) == NULL)
			{
				cli_print_debug_msg("Error: open file \"%s\"\n", FILE_RESOLV_CONF_TEMP);
				cli_dns_exit(3);
			}
			flock(fileno(fp2), LOCK_EX);
			while(fgets(line, 256, fp1) != NULL)
			{
				if(strstr(line, "domain") == line)
				{
					fputs(line, fp2);
				}
			}
			if(strlen(dns.dnssrv))
			{
				sprintf(line, "nameserver\t%s\n", dns.dnssrv);
				fputs(line, fp2);
			}
			if(strlen(dns.dnssrv2))
			{
				sprintf(line, "nameserver\t%s\n", dns.dnssrv2);
				fputs(line, fp2);
			}
			flock(fileno(fp1), LOCK_UN);
			fclose(fp1);
			flock(fileno(fp2), LOCK_UN);
			fclose(fp2);
			remove(FILE_RESOLV_CONF);
			rename(FILE_RESOLV_CONF_TEMP, FILE_RESOLV_CONF);
		}
		

	}
	else
	{
		cli_dns_usage();
		cli_dns_exit(1);
	}
	sqlite_close(dbname);
	cli_dns_exit(0);
	exit(0);
}

int
cli_dns_init()
{
	memset(dns.dnssrv, 0, sizeof(dns.dnssrv));
	memset(dns.dnssrv2, 0, sizeof(dns.dnssrv2));
	
	return(0);
}

int
cli_dns_usage()
{
	printf("Usage:\n\n"
	       "dns set ip <ip> [ <ip> ]\n\n"
	       //"dns set hostname <name>\n\n"
	       "dns unset\n\n"
	       "dns show\n\n");
	       //"dns startup\n\n");
	
	return(0);
}

int
cli_dns_exit(int value)
{
	exit(value);
}

