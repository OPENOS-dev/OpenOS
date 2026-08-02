/* Copyright 2020 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mosys/cli.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"

static void cmd_usage(struct platform_cmd *cmd)
{
	printf("usage: %s %s\n\n", cmd->name, cmd->usage ?: "");
}

static void usage(void)
{
	printf("usage: mosys [options] [commands]\n\n"
	       "  Options:\n"
	       "    -k            print data in key=value format\n"
	       "    -l            print data in long format\n"
	       "    -s [key]      print value for single key\n"
	       "    -v            verbose (can be used multiple times)\n"
	       "    -h            print this help\n"
	       "\n");
}

static void sub_list(struct platform_cmd *sub)
{
	struct platform_cmd *_sub;

	if (!sub)
		return;

	printf("  Commands:\n");
	for (_sub = sub->arg.sub; _sub && _sub->name; _sub++) {
		if (_sub->desc)
			printf("    %-12s  %s\n", _sub->name, _sub->desc);
	}
	printf("\n");
}

static int sub_main(struct platform_intf *intf, struct platform_cmd *sub,
		    int argc, char **argv)
{
	struct platform_cmd *cmd;

	if (!intf || !sub) {
		errno = ENOSYS;
		return -1;
	}

	switch (sub->type) {
	case ARG_TYPE_GETTER:
	case ARG_TYPE_SETTER:
		if (!sub->arg.func) {
			lprintf(LOG_ERR, "Undefined Function\n");
			return -1;
		}

		/* this is a function, it might have more args */
		if (argc > 0 && !strcmp(argv[0], "help")) {
			cmd_usage(sub);
			return 0;
		}

		/* run command handler */
		return sub->arg.func(intf, sub, argc, argv);

	case ARG_TYPE_SUB:
		if (!argc) {
			sub_list(sub);
			return -1; /* generic error */
		}

		/* this is a sub-command, we must have some more args */
		if (argc == 0 || strcmp(argv[0], "help") == 0) {
			sub_list(sub);
			return 0;
		}

		/* search for matching sub-command */
		for (cmd = sub->arg.sub; cmd && cmd->name; cmd++) {
			if (strlen(cmd->name) != strlen(argv[0]))
				continue;
			if (strcmp(cmd->name, argv[0]) == 0) {
				lprintf(LOG_DEBUG, "Subcommand %s (%s)\n",
					cmd->name, cmd->desc);
				return sub_main(intf, cmd, argc - 1, argv + 1);
			}
		}
		break;

	default:
		lprintf(LOG_ERR, "Unknown subcommand type\n");
		return -1;
	}

	lprintf(LOG_WARNING, "Command not found\n\n");
	sub_list(sub);

	errno = EINVAL; /* unknown or invalid subcommand argument */
	return -1;
}

static int intf_main(struct platform_intf *intf, int argc, char **argv)
{
	struct platform_cmd **_sub, *sub;
	bool do_list = false;

	if (!intf || !intf->sub) {
		lprintf(LOG_ERR, "No commands defined for this platform\n");
		errno = ENOSYS;
		return -1;
	}

	if (!argc || !strcmp(argv[0], "help")) {
		do_list = true;
		usage();
		printf("  Commands:\n");
	}

	/* go through subcommand list for this interface */
	for (_sub = intf->sub; _sub && *_sub; _sub++) {
		sub = *_sub;

		if (do_list) {
			printf("    %-12s  %s\n", sub->name, sub->desc);
			continue;
		}

		/* is this sub-command is the one they asked for? */
		if (!strcmp(sub->name, argv[0])) {
			lprintf(LOG_DEBUG, "Found command %s (%s)\n", sub->name,
				sub->desc);
			return sub_main(intf, sub, argc - 1, &(argv[1]));
		}
	}

	if (do_list) {
		if (!argc)
			return -1;
		return 0;
	}

	lprintf(LOG_WARNING, "Command not found\n\n");
	errno = ENOSYS;

	/* trigger a help listing */
	intf_main(intf, 0, NULL);
	return -1;
}

int mosys_main(int argc, char **argv)
{
	int rc, errsv;
	int argflag;
	int verbose = LOG_ERR;
	enum kv_pair_style style = KV_STYLE_VALUE;

	while ((argflag = getopt(argc, argv, "klvtSs:p:h")) > 0) {
		switch (argflag) {
		case 'k':
			style = KV_STYLE_PAIR;
			break;
		case 'l':
			style = KV_STYLE_LONG;
			break;
		case 's':
			style = KV_STYLE_SINGLE;
			if (!optarg) {
				usage();
				exit(EXIT_FAILURE);
			}
			kv_set_single_key(optarg);
			break;
		case 'v':
			verbose++;
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}

	mosys_set_kv_pair_style(style);

	/*
	 * Init the logging system and the default log output file (stderr).
	 * Anything logged with a level above the verbosity will not
	 * be logged at all. We use the number of "-v" arguments on
	 * the commandline as a bias against the default threshold.
	 * The more times you use "-v", the greater your logging level
	 * becomes, and the more information will be printed.
	 */
	mosys_log_init(argv[0], verbose, NULL);

	/* run command */
	errno = 0;
	rc = intf_main(&platform_intf, argc - optind, argv + optind);
	errsv = errno;
	if (rc < 0 && errsv == ENOSYS)
		lprintf(LOG_ERR, "Command not supported on this platform\n");

	mosys_log_halt();

	if (rc < 0 && errsv > 0)
		return errsv;
	return rc;
}
