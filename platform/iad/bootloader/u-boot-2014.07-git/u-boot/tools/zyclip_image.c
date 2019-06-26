// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018-2019 Sphairon GmbH (a ZyXEL company)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <compiler.h>
#include <sys/stat.h>

#define Z_BOOT_SIGNATURE_1	0x1a0d020f
#define Z_BOOT_SIGNATURE_2	0x0f140000


enum image_types {
	IMAGE_NONE,
	IMAGE_ZLOADER,
};

struct zboot_hdr {
	__u32 signature1;
	__u32 signature2;
	__u32 upgrade_cnt;
	__u32 indicator;
	__u32 csum;
	__u32 csum_oob;
	__u8 reserved[1000];
};

struct args {
	enum image_types type;
	unsigned int page_size;
	unsigned int oob_size;
	unsigned int upgrade_cnt;
	unsigned int eb_size;
	const char *spl_bin;
	const char *out_bin;
};

static void usage_msg(const char *name)
{
	fprintf(stderr, "%s: [-h] -t type [-u upgrade-cnt] [-P page-size] [-O oob-size] -E eraseblock-size -s spl-bin -o out-bin\n",
		name);
	fprintf(stderr, " Image types:\n"
			"  z-loader - z-boot compatible image from SPL\n");
}

static enum image_types parse_image_type(const char *type)
{
	if (!type)
		return IMAGE_NONE;

	if (!strncmp(type, "z-loader", 12))
		return IMAGE_ZLOADER;

	return IMAGE_NONE;
}

static int parse_args(int argc, char *argv[], struct args *arg)
{
	int opt;

	memset(arg, 0, sizeof(*arg));

	while ((opt = getopt(argc, argv, "ht:u:P:O:E:s:o:")) != -1) {
		switch (opt) {
		case 'h':
			usage_msg(argv[0]);
			return 1;
		case 't':
			arg->type = parse_image_type(optarg);
			break;
		case 'u':
			arg->upgrade_cnt = strtoul(optarg, NULL, 0);
			break;
		case 'P':
			arg->page_size = strtoul(optarg, NULL, 0);
			break;
		case 'O':
			arg->oob_size = strtoul(optarg, NULL, 0);
			break;
		case 'E':
			arg->eb_size = strtoul(optarg, NULL, 0);
			break;
		case 's':
			arg->spl_bin = optarg;
			break;
		case 'o':
			arg->out_bin = optarg;
			break;
		default:
			fprintf(stderr, "Invalid option -%c\n", opt);
			goto parse_error;
		}
	}

	if (arg->type == IMAGE_NONE) {
		fprintf(stderr, "Invalid image type\n");
		goto parse_error;
	}

	if (arg->type == IMAGE_ZLOADER) {
		if (!arg->page_size) {
			fprintf(stderr, "Missing page size\n");
			goto parse_error;
		}

		if (!arg->oob_size) {
			fprintf(stderr, "Missing OOB size\n");
			goto parse_error;
		}

		if (!arg->eb_size) {
			fprintf(stderr, "Missing erase block size\n");
			goto parse_error;
		}

		if (arg->eb_size > 256 * 1024) {
			fprintf(stderr, "Erase block must be lesser or equal 256k\n");
			goto parse_error;
		}

		if (!arg->out_bin) {
			fprintf(stderr, "Missing output binary\n");
			goto parse_error;
		}

		if (!arg->spl_bin) {
			fprintf(stderr, "Missing SPL binary\n");
			goto parse_error;
		}
	}

	return 0;

parse_error:
	usage_msg(argv[0]);
	return -1;
}

static int open_input_bin(const char *name, size_t *size)
{
	struct stat sbuf;
	int ret, fd;

	fd = open(name, O_RDONLY);
	if (0 > fd) {
		fprintf(stderr, "Cannot open %s: %s\n", name,
			strerror(errno));
		return -1;
	}

	ret = fstat(fd, &sbuf);
	if (0 > ret) {
		fprintf(stderr, "Cannot fstat %s: %s\n", name,
			strerror(errno));
		return -1;
	}

	*size = sbuf.st_size;

	return fd;
}

static int open_output_bin(const char *name)
{
	int fd;

	fd = open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (0 > fd) {
		fprintf(stderr, "Cannot open %s: %s\n", name,
			strerror(errno));
		return -1;
	}

	return fd;
}

static unsigned long calc_checksum(const void *buf, size_t size)
{
	unsigned long csum = 0;
	const __u16 *data = buf;
	size_t n;

	for (n = 0; n < size; n += 2) {
		csum += cpu_to_be16(*data);
		if (csum > 0xffff)
			csum = (csum + 1) & 0xffff;
		++data;
	}

	return csum & 0xffff;
}

static int create_zloader(const struct args *arg)
{
	int out_fd, spl_fd, ret = 0;
	unsigned int i, pages, hdr_offset;
	size_t size, buf_size;
	ssize_t n;
	struct zboot_hdr *hdr;
	__u32 csum, csum_oob;
	__u8 *buf, *src;

	/* buffer size is erase block size + summed OOB sizes for all pages */
	pages = arg->eb_size / arg->page_size;
	buf_size = arg->eb_size + pages * arg->oob_size;
	buf = malloc(buf_size);
	if (!buf) {
		ret = -1;
		goto err;
	}

	/* unused flash areas should be 0xff */
	memset(buf, 0xff, buf_size);

	out_fd = open_output_bin(arg->out_bin);
	if (0 > out_fd) {
		ret = -1;
		goto err_out;
	}

	spl_fd = open_input_bin(arg->spl_bin, &size);
	if (0 > spl_fd) {
		ret = -1;
		goto err_spl;
	}

	/* SPL size must not overlap with Z-boot header */
	hdr_offset = arg->eb_size - sizeof(*hdr);
	if (size >= hdr_offset) {
		ret = -1;
		goto err_write;
	}

	n = read(spl_fd, buf, size);
	if (n < 0) {
		ret = -1;
		goto err_write;
	}

	/* initialise Z-Boot header */
	hdr = (struct zboot_hdr *)(buf + hdr_offset);
	memset(hdr, 0, sizeof(*hdr));
	hdr->signature1 = cpu_to_be32(Z_BOOT_SIGNATURE_1);
	hdr->signature2 = cpu_to_be32(Z_BOOT_SIGNATURE_2);

	/*
	 * raw checksum is over SPL + empty area + Z-Boot header (w/o
	 * other checksums or upgrade count
	 */
	csum = calc_checksum(buf, arg->eb_size);
	hdr->csum = cpu_to_be32(csum);
	hdr->upgrade_cnt = cpu_to_be32(arg->upgrade_cnt);

	/* insert empty OOB areas for each page */
	src = buf + arg->page_size;
	for (i = 1; i < pages; i++) {
		__u8 *dst = src + arg->oob_size;
		size_t mv_size = arg->eb_size - i * arg->page_size;
		memmove(dst, src, mv_size);
		memset(src, 0xff, arg->oob_size);
		src = dst + arg->page_size;
	}
	memset(src, 0xff, arg->oob_size);

	/* re-align Z-Boot header */
	hdr_offset = sizeof(*hdr) / arg->page_size;
	hdr_offset = (hdr_offset + 1) * arg->oob_size;
	hdr_offset = buf_size - sizeof(*hdr) - hdr_offset;
	hdr = (struct zboot_hdr *)(buf + hdr_offset);

	/* OOB checksum is over the updated buffer with OOB areas */
	csum_oob = calc_checksum(buf, buf_size);
	hdr->csum_oob = cpu_to_be32(csum_oob);

	n = write(out_fd, buf, buf_size);
	if (n < 0)
		ret = -1;

err_write:
	close(spl_fd);
err_spl:
	close(out_fd);
err_out:
	free(buf);
err:
	return ret;
}

int main(int argc, char *argv[])
{
	int ret;
	struct args arg;

	ret = parse_args(argc, argv, &arg);
	if (ret)
		goto done;

	switch (arg.type) {
	case IMAGE_ZLOADER:
		ret = create_zloader(&arg);
		break;
	default:
		fprintf(stderr, "Image type not implemented\n");
		ret = -1;
		break;
	}

done:
	if (ret >= 0)
		return EXIT_SUCCESS;

	return EXIT_SUCCESS;
}
