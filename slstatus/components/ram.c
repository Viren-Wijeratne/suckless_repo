/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#include "../slstatus.h"
#include "../util.h"

#if defined(__linux__)
	#include <stdint.h>

	const char *
	ram_free(const char *unused)
	{
		uintmax_t free;
		FILE *fp;

		if (!(fp = fopen("/proc/meminfo", "r")))
			return NULL;

		if (lscanf(fp, "MemFree:", "%ju kB", &free) != 1) {
			fclose(fp);
			return NULL;
		}

		fclose(fp);
		return fmt_human(free * 1024, 1024);
	}

	const char *
	ram_perc(const char *unused)
	{
		uintmax_t total, free, buffers, cached, shmem, sreclaimable;
		int percent;
		FILE *fp;

		if (!(fp = fopen("/proc/meminfo", "r")))
			return NULL;

		if (lscanf(fp, "MemTotal:", "%ju kB", &total)  != 1 ||
		    lscanf(fp, "MemFree:", "%ju kB", &free)    != 1 ||
		    lscanf(fp, "Buffers:", "%ju kB", &buffers) != 1 ||
		    lscanf(fp, "Cached:", "%ju kB", &cached)   != 1 ||
		    lscanf(fp, "Shmem:", "%ju kB", &shmem)     != 1 ||
		    lscanf(fp, "SReclaimable:", "%ju kB", &sreclaimable) != 1) {
			fclose(fp);
			return NULL;
		}
		fclose(fp);

		if (total == 0)
			return NULL;

		percent = 100 * (total - free - buffers - cached - sreclaimable + shmem) / total;
		return bprintf("%d", percent);
	}

	const char *
	ram_total(const char *unused)
	{
		uintmax_t total;

		if (pscanf("/proc/meminfo", "MemTotal: %ju kB\n", &total)
		    != 1)
			return NULL;

		return fmt_human(total * 1024, 1024);
	}

	const char *
	ram_used(const char *unused)
	{
		uintmax_t total, free, buffers, cached, used, shmem, sreclaimable;
		FILE *fp;

		if (!(fp = fopen("/proc/meminfo", "r")))
			return NULL;

		if (lscanf(fp, "MemTotal:", "%ju kB", &total)  != 1 ||
		    lscanf(fp, "MemFree:", "%ju kB", &free)    != 1 ||
		    lscanf(fp, "Buffers:", "%ju kB", &buffers) != 1 ||
		    lscanf(fp, "Cached:", "%ju kB", &cached)   != 1 ||
		    lscanf(fp, "Shmem:", "%ju kB", &shmem)     != 1 ||
		    lscanf(fp, "SReclaimable:", "%ju kB", &sreclaimable) != 1) {
			fclose(fp);
			return NULL;
		}
		fclose(fp);

		used = total - free - buffers - cached - sreclaimable + shmem;
		return fmt_human(used * 1024, 1024);
	}
#endif
