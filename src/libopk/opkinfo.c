#include "opk.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static bool display_info(char *package_path) {
	printf("=== %s\n", package_path);

	struct OPK *opk = opk_open(package_path);
	if (!opk) {
		fprintf(stderr, "Failed to open %s\n", package_path);
		printf("\n");
		return false;
	}

	bool ok = true;

	while (true) {
		const char *metadata_name;
		if (opk_open_metadata(opk, &metadata_name) <= 0)
			break;

		const char *key, *val;
		size_t skey, sval;
		printf("\n");
		printf("Metadata file: %s\n\n", metadata_name);
		while(opk_read_pair(opk, &key, &skey, &val, &sval) && key)
			printf("%.*s: %.*s\n", (int) skey, key, (int) sval, val);
	}

	opk_close(opk);

	printf("\n");
	return ok;
}

int main(int argc, char **argv) {
	if (argc == 1) {
		fprintf(stderr, "Usage: opkinfo app1.opk [app2.opk ...]\n");
		exit(2);
	}

	bool ok = true;
	for (int i = 1; i < argc; i++) {
		ok &= display_info(argv[i]);
	}
	exit(ok ? 0 : 1);
}
