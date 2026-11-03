/* See LICENSE file for copyright and license details. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../slstatus.h"
#include "../util.h"

#if defined(__linux__)
	static int
	get_swap_info(long *s_total, long *s_free, long *s_cached)
	{
		FILE *fp;
		struct {
			const char *name;
			const size_t len;
			long *var;
		} ent[] = {
			{ "SwapTotal",  sizeof("SwapTotal") - 1,  s_total  },
			{ "SwapFree",   sizeof("SwapFree") - 1,   s_free   },
			{ "SwapCached", sizeof("SwapCached") - 1, s_cached },
		};
		size_t line_len = 0, i, left;
		char *line = NULL;

		/* get number of fields we want to extract */
		for (i = 0, left = 0; i < LEN(ent); i++)
			if (ent[i].var)
				left++;

		if (!(fp = fopen("/proc/meminfo", "r"))) {
			warn("fopen '/proc/meminfo':");
			return 1;
		}

		/* read file line by line and extract field information */
		while (left > 0 && getline(&line, &line_len, fp) >= 0) {
			for (i = 0; i < LEN(ent); i++) {
				if (ent[i].var &&
				    !strncmp(line, ent[i].name, ent[i].len)) {
					sscanf(line + ent[i].len + 1,
					       "%ld kB\n", ent[i].var);
					left--;
					break;
				}
			}
		}
		free(line);
		if (ferror(fp)) {
			warn("getline '/proc/meminfo':");
			return 1;
		}

		fclose(fp);
		return 0;
	}

	const char *
	swap_free(const char *unused)
	{
		long free;

		if (get_swap_info(NULL, &free, NULL))
			return NULL;

		return fmt_human(free * 1024, 1024);
	}

	const char *
	swap_perc(const char *unused)
	{
		long total, free, cached;

		if (get_swap_info(&total, &free, &cached) || total == 0)
			return NULL;

		return bprintf("%d", 100 * (total - free - cached) / total);
	}

	const char *
	swap_total(const char *unused)
	{
		long total;

		if (get_swap_info(&total, NULL, NULL))
			return NULL;

		return fmt_human(total * 1024, 1024);
	}

	const char *
	swap_used(const char *unused)
	{
		long total, free, cached;

		if (get_swap_info(&total, &free, &cached))
			return NULL;

		return fmt_human((total - free - cached) * 1024, 1024);
	}
#endif
