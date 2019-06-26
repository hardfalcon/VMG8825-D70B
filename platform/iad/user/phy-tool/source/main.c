/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2012 Daniel Schwierzeck <daniel.schwierzeck@sphairon.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/mii.h>

enum phytool_cmd {
    PHYTOOL_CMD_ADDRESS,
    PHYTOOL_CMD_READ,
    PHYTOOL_CMD_WRITE,
};

struct phytool_args {
    const char *ifname;
    unsigned int reg_offset;
    unsigned int reg_value;
    enum phytool_cmd cmd;
};

static const struct option long_options[] = {
    { "help",           no_argument,        NULL,   'h' },
    { "read",           required_argument,  NULL,   'r' },
    { "write",          required_argument,  NULL,   'w' },
    { "address",        no_argument,        NULL,   'a' },
    { "ifname",         required_argument,  NULL,   'i' },
    { "value",          required_argument,  NULL,   'v' },
    { NULL, 0, NULL, 0 }
};

static const char short_options[] = "hr:w:ai:v:";

static void usageMsg(const char *name)
{
    fprintf(stderr, "%s\n"
        "Options:\n"
        " -h, --help                print this help\n"
        " -r, --read <regnum>       read PHY register\n"
        " -w, --write <regnum>      write PHY register\n"
        " -a, --address             get PHY address\n"
        " -i, --ifname              interface PHY is bound to\n"
        " -v, --value               PHY register value to write\n",
        name);
}

static int parseArgs(int argc, char** argv, struct phytool_args *args)
{
    int c, opt_index;
    bool has_value = false;

    memset(args, 0, sizeof(*args));

    do {
        c = getopt_long(argc, argv, short_options, long_options, &opt_index);

        switch (c) {
        case 'h':
            usageMsg(argv[0]);
            return -1;
        case 'r':
            args->reg_offset = strtoul(optarg, NULL, 0);
            args->cmd = PHYTOOL_CMD_READ;
            break;
        case 'w':
            args->reg_offset = strtoul(optarg, NULL, 0);
            args->cmd = PHYTOOL_CMD_WRITE;
            break;
        case 'a':
            args->cmd = PHYTOOL_CMD_ADDRESS;
            break;
        case 'i':
            args->ifname = optarg;
            break;
        case 'v':
            args->reg_value = strtoul(optarg, NULL, 0);
            has_value = true;
            break;
        case -1:
            break;
        default:
            usageMsg(argv[0]);
            return -1;
        }
    } while (c != -1);

    if (!args->ifname) {
        fprintf(stderr, "no interface given\n");
        return -1;
    }

    if (args->cmd == PHYTOOL_CMD_WRITE && !has_value) {
        fprintf(stderr, "invalid value\n");
        return -1;
    }

    return 0;
}

static int doPhyAddress(int fd, struct ifreq *ifr)
{
    struct mii_ioctl_data *mii = (struct mii_ioctl_data*)(&ifr->ifr_data);
    int err;

    err = ioctl(fd, SIOCGMIIPHY, ifr);
    if (err) {
        perror("ioctl");
        return 1;
    }

    printf("%s: PHY address  0x%02x\n", ifr->ifr_name, mii->phy_id);

    return 0;
}

static int doPhyRead(int fd, struct ifreq *ifr)
{
    struct mii_ioctl_data *mii = (struct mii_ioctl_data*)(&ifr->ifr_data);
    int err;

    err = ioctl(fd, SIOCGMIIREG, ifr);
    if (err) {
        perror("ioctl");
        return 1;
    }

    printf("%s: PHY register 0x%02x = 0x%04x\n", ifr->ifr_name, mii->reg_num,
           mii->val_out);

    return 0;
}

static int doPhyWrite(int fd, struct ifreq *ifr)
{
    struct mii_ioctl_data *mii = (struct mii_ioctl_data*)(&ifr->ifr_data);
    int err;

    err = ioctl(fd, SIOCSMIIREG, ifr);
    if (err) {
        perror("ioctl");
        return 1;
    }

    printf("%s: PHY register 0x%02x = 0x%04x\n", ifr->ifr_name, mii->reg_num,
           mii->val_in);

    return 0;
}

static int doPhyCmd(const struct phytool_args *args)
{
    int fd, err;
    struct ifreq ifr;
    struct mii_ioctl_data *mii = (struct mii_ioctl_data*)(&ifr.ifr_data);

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, args->ifname);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("Cannot get control socket");
        return 1;
    }

    err = doPhyAddress(fd, &ifr);
    if (err) {
        close(fd);
        return 1;
    }

    switch (args->cmd) {
    case PHYTOOL_CMD_ADDRESS:
        break;

    case PHYTOOL_CMD_READ:
        mii->reg_num = args->reg_offset;
        err = doPhyRead(fd, &ifr);
        break;

    case PHYTOOL_CMD_WRITE:
        mii->reg_num = args->reg_offset;
        mii->val_in = args->reg_value;
        err = doPhyWrite(fd, &ifr);
        break;

    default:
        err = 1;
    }

    close(fd);

    return err;
}

int main(int argc, char** argv)
{
    struct phytool_args args;
    int err;

    err = parseArgs(argc, argv, &args);
    if (err)
        return 1;

    return doPhyCmd(&args);
}

