/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>

#include "../slstatus.h"
#include "../util.h"

#if defined(__linux__)
/*
 * https://www.kernel.org/doc/html/latest/power/power_supply_class.html
 */
	#include <limits.h>
	#include <stdint.h>
	#include <unistd.h>

	#define POWER_SUPPLY_CAPACITY "/sys/class/power_supply/%s/capacity"
	#define POWER_SUPPLY_STATUS   "/sys/class/power_supply/%s/status"
	#define POWER_SUPPLY_CHARGE   "/sys/class/power_supply/%s/charge_now"
	#define POWER_SUPPLY_ENERGY   "/sys/class/power_supply/%s/energy_now"
	#define POWER_SUPPLY_CURRENT  "/sys/class/power_supply/%s/current_now"
	#define POWER_SUPPLY_POWER    "/sys/class/power_supply/%s/power_now"

	static const char *
	pick(const char *bat, const char *f1, const char *f2, char *path,
	     size_t length)
	{
		if (esnprintf(path, length, f1, bat) > 0 &&
		    access(path, R_OK) == 0)
			return f1;

		if (esnprintf(path, length, f2, bat) > 0 &&
		    access(path, R_OK) == 0)
			return f2;

		return NULL;
	}

	const char *
	battery_perc(const char *bat)
	{
		int cap_perc;
		char path[PATH_MAX];

		if (esnprintf(path, sizeof(path), POWER_SUPPLY_CAPACITY, bat) < 0)
			return NULL;
		if (pscanf(path, "%d", &cap_perc) != 1)
			return NULL;

		return bprintf("%d", cap_perc);
	}

	const char *
	battery_state(const char *bat)
	{
		static struct {
			char *state;
			char *symbol;
		} map[] = {
			{ "Charging",    "+" },
			{ "Discharging", "-" },
			{ "Full",        "F" },
			{ "Not charging", "NC" },
		};
		size_t i;
		char path[PATH_MAX], state[13];

		if (esnprintf(path, sizeof(path), POWER_SUPPLY_STATUS, bat) < 0)
			return NULL;
		if (pscanf(path, "%12[a-zA-Z ]", state) != 1)
			return NULL;

		for (i = 0; i < LEN(map); i++)
			if (!strcmp(map[i].state, state))
				break;

		return (i == LEN(map)) ? "?" : map[i].symbol;
	}

	const char *
	battery_remaining(const char *bat)
	{
		uintmax_t charge_now, current_now, m, h;
		double timeleft;
		char path[PATH_MAX], state[13];

		if (esnprintf(path, sizeof(path), POWER_SUPPLY_STATUS, bat) < 0)
			return NULL;
		if (pscanf(path, "%12[a-zA-Z ]", state) != 1)
			return NULL;

		if (!pick(bat, POWER_SUPPLY_CHARGE, POWER_SUPPLY_ENERGY, path,
		          sizeof(path)) ||
		    pscanf(path, "%ju", &charge_now) < 0)
			return NULL;

		if (!strcmp(state, "Discharging")) {
			if (!pick(bat, POWER_SUPPLY_CURRENT, POWER_SUPPLY_POWER, path,
			          sizeof(path)) ||
			    pscanf(path, "%ju", &current_now) < 0)
				return NULL;

			if (current_now == 0)
				return NULL;

			timeleft = (double)charge_now / (double)current_now;
			h = timeleft;
			m = (timeleft - (double)h) * 60;

			return bprintf("%juh %jum", h, m);
		}

		return "";
	}
#endif
