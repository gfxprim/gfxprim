//SPDX-License-Identifier: LGPL-2.0-or-later

/*

    Copyright (C) 2007-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <string.h>
#include <mntent.h>
#include <sys/vfs.h>
#include <fcntl.h>

#include <gfxprim.h>

/*
 * List of misc filesystem to exclude from the listing.
 */
#define IS_DUMMY(fs_type) (            \
	!strcmp(fs_type, "autofs") ||  \
	!strcmp(fs_type, "none") ||    \
	!strcmp(fs_type, "proc") ||    \
	!strcmp(fs_type, "tmpfs") ||   \
	!strcmp(fs_type, "devtmpfs") ||   \
	!strcmp(fs_type, "usbfs") ||   \
	!strcmp(fs_type, "debugfs") || \
	!strcmp(fs_type, "sysfs") ||   \
	!strcmp(fs_type, "devpts") ||  \
	!strcmp(fs_type, "rootfs") ||  \
	!strcmp(fs_type, "cgroup") ||  \
	!strcmp(fs_type, "cgroup2") || \
	!strcmp(fs_type, "fusectl") || \
	!strcmp(fs_type, "subfs") ||   \
	!strcmp(fs_type, "rpc_pipefs") ||   \
	!strcmp(fs_type, "mqueue")     \
)

#define MOUNT "/proc/mounts"

static void *fs_widget_groups;

struct fs_info_widgets {
	gp_widget *dev_label;
	gp_widget *used;
	gp_widget *avail_label;
	gp_widget *grid;

	unsigned int bsize;
	unsigned int blocks;
	unsigned int bfree;
	unsigned int bavail;

	struct fs_info_widgets *next;
};

static gp_widget *load_fsinfo(void)
{
	FILE *f = setmntent(MOUNT, "r");
	struct statfs sfs;
	struct mntent *mnt;
	struct fs_info_widgets *infos = NULL, *last = NULL;
	unsigned int infos_cnt = 0;

	while ((mnt = getmntent(f))) {
		if (IS_DUMMY(mnt->mnt_type))
			continue;

		if (statfs(mnt->mnt_dir, &sfs) < 0)
			continue;

		GP_DEBUG(0, "%s: bsize %llu blocks %llu free %llu avail %llu",
		           mnt->mnt_fsname, (long long unsigned)sfs.f_bsize,
			   (long long unsigned)sfs.f_blocks,
			   (long long unsigned)sfs.f_bfree,
			   (long long unsigned)sfs.f_bavail);

		if (sfs.f_blocks == 0)
			continue;

		if (gp_htable_get(fs_widget_groups, mnt->mnt_fsname))
			continue;

		struct fs_info_widgets *widgets = malloc(sizeof(struct fs_info_widgets));
		if (!widgets)
			continue;

		widgets->dev_label = gp_widget_label_printf_new(0, "%s mounted on %s", mnt->mnt_fsname, mnt->mnt_dir);
		widgets->used = gp_widget_pbar_new(sfs.f_blocks - sfs.f_bavail, sfs.f_blocks, GP_WIDGET_PBAR_PERCENTS);
		widgets->used->align = GP_HFILL | GP_VCENTER;
		widgets->grid = gp_widget_grid_new(1, 3, 0);

		widgets->grid->grid->frame = 1;
		widgets->grid->align = GP_HFILL;

		char size_avail[128];
		char size_total[128];

		gp_str_file_size(size_avail, sizeof(size_avail), sfs.f_bavail * sfs.f_bsize);
		gp_str_file_size(size_total, sizeof(size_total), sfs.f_blocks * sfs.f_bsize);

		widgets->avail_label = gp_widget_label_printf_new(0, "%s free of %s", size_avail, size_total);

		gp_widget_grid_put(widgets->grid, 0, 0, widgets->dev_label);
		gp_widget_grid_put(widgets->grid, 0, 1, widgets->used);
		gp_widget_grid_put(widgets->grid, 0, 2, widgets->avail_label);

		if (!last)
			infos = widgets;
		else
			last->next = widgets;

		last = widgets;
		last->next = NULL;

		infos_cnt++;

		gp_htable_put(fs_widget_groups, widgets, mnt->mnt_fsname);
	}

	gp_widget *grid = gp_widget_grid_new(1, infos_cnt, 0);

	struct fs_info_widgets *fs_i;
	unsigned int i = 0;
	for (fs_i = infos; fs_i; fs_i = fs_i->next) {
		gp_widget_grid_put(grid, 0, i, fs_i->grid);
		i++;
	}

	return grid;
}

static int open_proc_mounts(void)
{
	int fd = open("/proc/mounts", O_RDONLY);

	return fd;
}

static int proc_mounts_event(gp_fd *self)
{
	gp_widget *layout, *old_layout;

	(void) self;

	gp_htable_free(fs_widget_groups);
	fs_widget_groups = gp_htable_new(0, GP_HTABLE_COPY_KEY);

	layout = load_fsinfo();

	if (!layout)
		return 0;

	old_layout = gp_widget_layout_replace(layout);

	gp_widget_free(old_layout);

	return 0;
}

gp_app_info app_info = {
	.name = "Disk Free",
	.desc = "Disk free space example",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://gfxprim.ucw.cz",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2007-2023"},
		{}
	}
};

int main(int argc, char *argv[])
{
	fs_widget_groups = gp_htable_new(0, GP_HTABLE_COPY_KEY);

	gp_widget *layout = load_fsinfo();

	int fd_proc = open_proc_mounts();
	gp_fd fd = {
		.fd = fd_proc,
		.event = proc_mounts_event,
		.events = GP_POLLPRI,
	};

	gp_widget_poll_add(&fd);

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
