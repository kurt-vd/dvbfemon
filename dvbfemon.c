#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>

const struct lookup {
	int mask;
	const char *name;
} dvbfe_status[] = {
	{ FE_HAS_SIGNAL, "SIGNAL", },
	{ FE_HAS_CARRIER, "CARRIER", },
	{ FE_HAS_VITERBI, "STABLE", },
	{ FE_HAS_SYNC, "SYNC", },
	{ FE_HAS_LOCK, "LOCK", },
	{ FE_TIMEDOUT, "TIMEDOUT", },
	{ FE_REINIT, "REINIT", },
	{},
};

int main(int argc, char *argv[])
{
	int fd, status;
	const struct lookup *lp;
	char *pstr, str[16];
	uint16_t sig, snr;
	uint32_t ucb;
	const char *file = "/dev/dvb/adapter0/frontend0";

	if (argc >= 2)
		file = argv[1];

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "open %s: %s\n", file, strerror(errno));
		return 1;
	}
	for (;; sleep(1)) {
		if (ioctl(fd, FE_READ_STATUS, &status) < 0) {
			fprintf(stderr, "fe read status: %s\n", strerror(errno));
			return 1;
		}
		pstr = str;
		for (lp = dvbfe_status; lp->name; ++lp)
			*pstr++ = (status & lp->mask) ? lp->name[0] : '_';
		*pstr = 0;

		if (ioctl(fd, FE_READ_SIGNAL_STRENGTH, &sig) < 0)
			sig = 0;
		if (ioctl(fd, FE_READ_SNR, &snr) < 0)
			snr = 0;
		if (ioctl(fd, FE_READ_UNCORRECTED_BLOCKS, &ucb) < 0)
			ucb = 0;
		printf("%s, sig %u, snr %u, ucb %u\n", str, sig, snr, ucb);
		/* flush output. This is necessary when stdout is not a terminal */
		fflush(stdout);
	}
	return 0;
}
