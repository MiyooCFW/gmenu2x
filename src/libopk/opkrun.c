#define _BSD_SOURCE 1
#define _XOPEN_SOURCE 1

#include "libopk.h"

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/loop.h>

#ifndef MY_NAME
#define MY_NAME "opkrun"
#endif

#ifndef VTCON_FILE
#define VTCON_FILE "/sys/devices/virtual/vtconsole/vtcon1/bind"
#endif

#ifndef LINKDEV_ALT_MAP_FILE
#define LINKDEV_ALT_MAP_FILE "/sys/devices/platform/linkdev/alt_key_map"
#endif

#ifndef JZ4770FB_ENABLE_DOWNSCALING_FILE
#define JZ4770FB_ENABLE_DOWNSCALING_FILE "/sys/devices/platform/jz-lcd.0/allow_downscaling"
#endif

#define NB_PARAMS_MAX 256

struct params {
	char *mountpoint, *exec[NB_PARAMS_MAX];
	bool needs_terminal, needs_joystick, needs_gsensor, needs_downscaling;
};


static const struct option options[] = {
	  {"help", no_argument, 0, 'h'},
	  {"metadata", required_argument, 0, 'm'},
	  {0, 0, 0, 0},
};

static const char *options_descriptions[] = {
	"Show this help and quit.",
	"Metadata file to use (default: first one found)",
};


static void usage(void)
{
	printf("Usage:\n\t" MY_NAME " [OPTIONS] OPK_FILE [ARGS ...]\n\nOptions:\n");

	for (size_t i = 0; options[i].name; i++)
		printf("\t-%c, --%s\n\t\t\t%s\n",
					options[i].val, options[i].name,
					options_descriptions[i]);
}

static struct OPK * open_opk(const char *filename, const char *metadata)
{
	struct OPK *opk = opk_open(filename);
	if (!opk) {
		fprintf(stderr, "Unable to open OPK\n");
		return NULL;
	}

	for (;;) {
		const char *meta_file;
		int ret = opk_open_metadata(opk, &meta_file);
		if (ret < 0) {
			fprintf(stderr, "Unable to open metadata file within OPK\n");
			goto close_opk;
		}

		if (!ret) {
			fprintf(stderr, "Metadata file not found in OPK\n");
			goto close_opk;
		}

		if (!metadata || !strcmp(metadata, meta_file))
			break;
	}

	return opk;

close_opk:
	opk_close(opk);
	return NULL;
}

static int read_params(struct OPK *opk, struct params *params)
{
	memset(params, 0, sizeof(*params));
	const char *exec_name = NULL, *name = NULL;
	size_t exec_name_len = 0, name_len = 0;

	for (;;) {
		const char *key, *val;
		size_t skey, sval;
		int ret = opk_read_pair(opk, &key, &skey, &val, &sval);
		if (ret < 0) {
			fprintf(stderr, "Unable to read key/value pair from metadata\n");
			return ret;
		}

		if (!ret)
			break;

		if (!strncmp(key, "Name", skey)) {
			name_len = sval;
			name = val;
			continue;
		}

		if (!strncmp(key, "Exec", skey)) {
			exec_name_len = sval;
			exec_name = val;
			continue;
		}

		if (!strncmp(key, "Terminal", skey)) {
			params->needs_terminal = !strncmp(val, "true", sval);
			continue;
		}

		if (!strncmp(key, "X-OD-NeedsJoystick", skey)) {
			params->needs_joystick =  !strncmp(val, "true", sval);
			continue;
		}

		if (!strncmp(key, "X-OD-NeedsGSensor", skey)) {
			params->needs_gsensor = !strncmp(val, "true", sval);
			continue;
		}

		if (!strncmp(key, "X-OD-NeedsDownscaling", skey)) {
			params->needs_downscaling = !strncmp(val, "true", sval);
			continue;
		}
	}

	if (!exec_name || !name) {
		fprintf(stderr, "Unable to find the executable name\n");
		return -1;
	}

	char *exec = malloc(exec_name_len + 1);
	memcpy(exec, exec_name, exec_name_len);
	exec[exec_name_len] = '\0';

	/* Split the Exec command into an array of parameters */
	char *ptr;
	unsigned int arg;
	for (ptr = exec, arg = 0; ptr && arg < NB_PARAMS_MAX - 1; arg++) {
		params->exec[arg] = ptr;
		ptr = strchr(ptr, ' ');

		if (ptr) {
			*ptr++ = '\0';
			while (*ptr == ' ') /* Skip eventual additional spaces */
				ptr++;
		}
	}

	params->exec[arg] = NULL;

	params->mountpoint = malloc(name_len + 6);
	sprintf(params->mountpoint, "/mnt/%.*s", (int) name_len, name);

	for (ptr = params->mountpoint + 5; *ptr; ptr++) {
		if (*ptr == '\'' || *ptr == '\\')
			*ptr = '_';
	}
	return 0;
}

static void enable_vtcon(void)
{
	FILE *f = fopen(VTCON_FILE, "w");
	if (!f) {
		perror("Unable to open vtcon file");
		return;
	}

	char one = '1';
	fwrite(&one, 1, 1, f);
	fclose(f);
}

static void enable_in_sysfs(const char *fn)
{
	FILE *f = fopen(fn, "w");
	if (!f)
		return;

	char yes = 'Y';
	fwrite(&yes, 1, 1, f);
	fclose(f);
}

static void enable_alt_key_map(void)
{
	enable_in_sysfs(LINKDEV_ALT_MAP_FILE);
}

static void enable_downscaling(void)
{
	enable_in_sysfs(JZ4770FB_ENABLE_DOWNSCALING_FILE);
}

static void enable_gsensor(void)
{
	system("/usr/sbin/gsensor --start");

	/* Fix for SDL apps to recognize the g-sensor */
	putenv("SDL_JOYSTICK_DEVICE=/dev/input/gsensor");
}

static char *get_url(const char *file)
{
	char *url = realpath(file, NULL);
	if (!url)
		return strdup(file);

	char *tmp = malloc(strlen(url) + sizeof "file://");
	sprintf(tmp, "file://%s", url);
	free(url);
	return tmp;
}

static int logetfree(void)
{
	int fd = open("/dev/loop-control", O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open '/dev/loop-control': %d\n", fd);
		return -1;
	}

	int devnr = ioctl(fd, LOOP_CTL_GET_FREE, NULL);
	if (devnr < 0) {
		fprintf(stderr, "Failed to acquire free loop device: %d\n", devnr);
	}

	close(fd);
	return devnr;
}

static int losetup(const char *loop, const char *file)
{
	unsigned int i;
	int filefd, loopfd, ret;

	filefd = open(file, O_RDONLY);
	if (filefd < 0) {
		fprintf(stderr, "losetup: cannot open '%s': %d\n", file, filefd);
		return -1;
	}

	/* We try to open the loop device just a bit after it was created.
	 * Give some time to udev so that it can set the proper rights. */
	for (i = 0; i < 100; i++) {
		loopfd = open(loop, O_RDONLY);
		if (loopfd < 0 && errno == EACCES)
			usleep(10000);
		else
			break;
	}
	if (loopfd < 0) {
		fprintf(stderr, "losetup: cannot open '%s': %d\n", loop, loopfd);
		close(filefd);
		return loopfd;
	}

	ret = ioctl(loopfd, LOOP_SET_FD, (void *)(intptr_t)filefd);
	if (ret < 0) {
		fprintf(stderr, "Cannot setup loop device '%s': %d\n", loop, ret);
		close(loopfd);
		close(filefd);
		return ret;
	}

	close(filefd);
	return loopfd;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Error: Too few arguments given.\n\n");
		usage();
		return EXIT_SUCCESS;
	}

	int c, option_index = 0, arg_index = 1;
	const char *metadata = NULL;

	while ((c = getopt_long(argc, argv, "+hm:",
						options, &option_index)) != -1) {
		switch (c) {
			case 'h':
				usage();
				return EXIT_SUCCESS;

			case 'm':
				metadata = optarg;
				arg_index += 2;
				break;

			case '?':
				return EXIT_FAILURE;
		}
	}

	if (arg_index >= argc) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		usage();
		return EXIT_FAILURE;
	}

	const char *opk_name = argv[arg_index];
	struct OPK *opk = open_opk(opk_name, metadata);
	if (!opk)
		return EXIT_FAILURE;

	struct params params;
	int ret = read_params(opk, &params);
	opk_close(opk);
	if (ret < 0)
		return EXIT_FAILURE;

	char **opk_argv = argv + arg_index + 1;
	int opk_argc = argc - arg_index - 1;
	if (opk_argc > NB_PARAMS_MAX - 2)
		opk_argc = NB_PARAMS_MAX - 2;

	char *args[NB_PARAMS_MAX];
	memset(args, 0, sizeof(args));

	/* This loop is used to replace the [%f %F %u %U] tokens
	 * with the filenames passed as parameter of opkrun */
	char **ptr;
	unsigned int arg;
	for (arg = 0, ptr = params.exec; *ptr && arg < NB_PARAMS_MAX; ptr++, arg++) {
		if (!strcmp("%f", *ptr)) {
			if (!opk_argc) {
				fprintf(stderr, "WARNING: OPK requires a file as parameter, but none was given\n");
			} else {
				args[arg] = realpath(*opk_argv++, NULL);
				if (--opk_argc)
					fprintf(stderr, "WARNING: OPK requires only one file as parameter\n");
			}
		} else if (!strcmp("%F", *ptr)) {
			while (opk_argc && arg < NB_PARAMS_MAX) {
				args[arg++] = realpath(*opk_argv++, NULL);
				opk_argc--;
			}
			arg--; /* Compensate the arg++ in the 'for' */
		} else if (!strcmp("%u", *ptr)) {
			if (!opk_argc) {
				fprintf(stderr, "WARNING: OPK requires an URL as parameter, but none was given\n");
			} else {
				args[arg] = get_url(*opk_argv++);
				if (--opk_argc)
					fprintf(stderr, "WARNING: OPK requires only one URL as parameter\n");
			}
		} else if (!strcmp("%U", *ptr)) {
			while (opk_argc && arg < NB_PARAMS_MAX) {
				args[arg++] = get_url(*opk_argv++);
				opk_argc--;
			}
			arg--; /* Compensate the arg++ in the 'for' */
		} else {
			args[arg] = strdup(*ptr);
		}
	}

	free(params.exec[0]);

	umount(params.mountpoint);
	mkdir(params.mountpoint, 0755);

	int devnr = logetfree();
	if (devnr < 0)
		return devnr;

	char loop_dev[9 + 10 + 1];
	sprintf(loop_dev, "/dev/loop%i", devnr);

	int loopfd = losetup(loop_dev, opk_name);
	if (loopfd < 0) {
		perror("Failed to losetup");
		return loopfd;
	}

	ret = mount(loop_dev, params.mountpoint, "squashfs", MS_NODEV | MS_NOSUID | MS_RDONLY, 0);
	if (ret < 0) {
		perror("Unable to mount OPK");
		free(params.mountpoint);
		return EXIT_FAILURE;
	}

	chdir(params.mountpoint);

	if (params.needs_terminal)
		enable_vtcon();

	if (params.needs_joystick)
		enable_alt_key_map();

	if (params.needs_gsensor)
		enable_gsensor();

	if (params.needs_downscaling)
		enable_downscaling();

	pid_t son = fork();
	if (!son) {
		/* Drop privileges */
		seteuid(getuid());

		if (!access(args[0], X_OK)) /* Not in the root of the OPK */
			execv(args[0], args);   /* Maybe in the $PATH? */
		execvp(args[0], args);
	}

	int status;
	waitpid(son, &status, 0);

	chdir("/");
	umount(params.mountpoint);
	rmdir(params.mountpoint);
	free(params.mountpoint);

	ioctl(loopfd, LOOP_CLR_FD, (void *)0);
	close(loopfd);

	for (char **ptr = args; *ptr; ptr++)
		free(*ptr);

	return status;
}
