#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "dtypes.h"
#include <mntent.h>
#include <dirent.h>
#include "diskID.h"

/**
 *Usually uuid are hex number separated by "-". this method read up to 8 hex
 *numbers skipping - characters.
 *@param uuid uuid as read in /dev/disk/by-uuid
 *@param buffer_out: unsigned char buffer[8] output buffer for result
 */
static void parseUUID(const char *uuid, unsigned char* buffer_out, unsigned int out_size) 
{
	size_t len;
	unsigned int i, j;
	char * hexuuid;
	unsigned char cur_character;
	//remove characters not in hex set
	len = strlen(uuid);
	hexuuid = (char *) malloc(sizeof(char) * strlen(uuid));
	memset(buffer_out, 0, out_size);
	memset(hexuuid, 0, sizeof(char) * strlen(uuid));

	for (i = 0, j = 0; i < len; i++) {
		if (isxdigit(uuid[i])) {
			hexuuid[j] = uuid[i];
			j++;
		} else {
			//skip
			continue;
		}
	}
	if (j % 2 == 1) {
		hexuuid[j++] = '0';
	}
	hexuuid[j] = '\0';

	// create an outsize-sized id string for the disk from the uuid
	for (i = 0; i < j / 2; i++) {
		sscanf(&hexuuid[i * 2], "%2hhx", &cur_character);
		buffer_out[i % out_size] = buffer_out[i % out_size] ^ cur_character;
	}

	// printf("parseUUID() : uuid is %s : %\n", hexuuid, buffer_out);
	free(hexuuid);
}

#define MAX_UNITS 20
FUNCTION_RETURN getDiskInfos(DiskInfo * diskInfos, size_t * disk_info_size) 
{
	struct stat mount_stat, sym_stat;
	/*static char discard[1024];
	 char device[64], name[64], type[64];
	 */
	char cur_dir[MAX_PATH];
	struct mntent *ent;

	int maxDrives, currentDrive, i, drive_found;
	__ino64_t *statDrives = NULL;
	DiskInfo *tmpDrives = NULL;
	FILE *aFile = NULL;
	DIR  *disk_by_uuid_dir = NULL, *disk_by_label = NULL;
	struct dirent *dir = NULL;
	FUNCTION_RETURN result;

	if (diskInfos != NULL) {
		maxDrives = *disk_info_size;
		tmpDrives = diskInfos;
	} else {
		maxDrives = MAX_UNITS;
		tmpDrives = (DiskInfo *) malloc(sizeof(DiskInfo) * maxDrives);
	}
	memset(tmpDrives, 0, sizeof(DiskInfo) * maxDrives);
	statDrives = (__ino64_t *) malloc(maxDrives * sizeof(__ino64_t ));
	memset(statDrives, 0, sizeof(__ino64_t ) * maxDrives);
	;

	aFile = setmntent("/proc/mounts", "r");
	if (aFile == NULL) {
		/*proc not mounted*/
		free(tmpDrives);
		free(statDrives);
		return FUNC_RET_ERROR;
	}

	currentDrive = 0;
	while (NULL != (ent = getmntent(aFile))) {
		if ((strncmp(ent->mnt_type, "ext", 3) == 0
				|| strncmp(ent->mnt_type, "xfs", 3) == 0
				|| strncmp(ent->mnt_type, "vfat", 4) == 0
				|| strncmp(ent->mnt_type, "ntfs", 4) == 0)
				&& ent->mnt_fsname != NULL
				&& strncmp(ent->mnt_fsname, "/dev/", 5) == 0) {
			if (stat(ent->mnt_fsname, &mount_stat) == 0) {
				drive_found = -1;
				for (i = 0; i < currentDrive; i++) {
					if (statDrives[i] == mount_stat.st_ino) {
						drive_found = i;
					}
				}
				if (drive_found == -1) {
					LOG_DEBUG("mntent: %s %s %d\n", ent->mnt_fsname, ent->mnt_dir,
							(unsigned long int)mount_stat.st_ino);
					strncpy(tmpDrives[currentDrive].device, ent->mnt_fsname, 255-1);
					statDrives[currentDrive] = mount_stat.st_ino;
					drive_found = currentDrive;
					currentDrive++;
				}
				if (strcmp(ent->mnt_dir, "/") == 0) {
					strcpy(tmpDrives[drive_found].label, "root");
					LOG_DEBUG("drive %s set to preferred\n", ent->mnt_fsname);
					tmpDrives[drive_found].preferred = true;
				}
			}
		}
	}
	endmntent(aFile);

	if (diskInfos == NULL) {
		*disk_info_size = currentDrive;
		free(tmpDrives);
		result = FUNC_RET_OK;
	} else if (*disk_info_size >= currentDrive) {
		disk_by_uuid_dir = opendir("/dev/disk/by-uuid");
		if (disk_by_uuid_dir == NULL) {
			LOG_WARN("Open /dev/disk/by-uuid fail");
			free(statDrives);
			return FUNC_RET_ERROR;
		}
		result = FUNC_RET_OK;
		*disk_info_size = currentDrive;
		while ((dir = readdir(disk_by_uuid_dir)) != NULL) {
			strcpy(cur_dir, "/dev/disk/by-uuid/");
			strncat(cur_dir, dir->d_name, 200);
			if (stat(cur_dir, &sym_stat) == 0) {
				for (i = 0; i < currentDrive; i++) {
					if (sym_stat.st_ino == statDrives[i]) {
						parseUUID(dir->d_name, tmpDrives[i].disk_sn, sizeof(tmpDrives[i].disk_sn));

						//VALGRIND_CHECK_VALUE_IS_DEFINED(tmpDrives[i].device);

						// printf("diskid %d %s %02x%02x%02x%02x\n", 
						//         i,
						// 		tmpDrives[i].device,
						// 		tmpDrives[i].disk_sn[0], tmpDrives[i].disk_sn[1], tmpDrives[i].disk_sn[2], tmpDrives[i].disk_sn[3]);
					}
				}
			}
		}
		closedir(disk_by_uuid_dir);

		disk_by_label = opendir("/dev/disk/by-label");
		if (disk_by_label != NULL) {
			while ((dir = readdir(disk_by_label)) != NULL) {
				strcpy(cur_dir, "/dev/disk/by-label/");
				strcat(cur_dir, dir->d_name);
				if (stat(cur_dir, &sym_stat) == 0) {
					for (i = 0; i < currentDrive; i++) {
						if (sym_stat.st_ino == statDrives[i]) {
							strncpy(tmpDrives[i].label, dir->d_name, 255-1);
							// printf("label %d %s %s\n", 
							//        i, 
							// 	   tmpDrives[i].label,
							// 	   tmpDrives[i].device);
						}
					}
				}
			}
			closedir(disk_by_label);
		}
	} else {
		result = FUNC_RET_BUFFER_TOO_SMALL;
	}
	/*
	 FILE *mounts = fopen(_PATH_MOUNTED, "r");
	 if (mounts == NULL) {
	 return ERROR;
	 }

	 while (fscanf(mounts, "%64s %64s %64s %1024[^\n]", device, name, type,
	 discard) != EOF) {
	 if (stat(device, &mount_stat) != 0)
	 continue;
	 if (filename_stat.st_dev == mount_stat.st_rdev) {
	 fprintf(stderr, "device: %s; name: %s; type: %s\n", device, name,
	 type);
	 }
	 }
	 */
	free(statDrives);
	return result;
}



FUNCTION_RETURN generate_disk_pc_id(PcIdentifier * identifiers,
                                    	   unsigned int * num_identifiers, 
										   bool use_label) 
{
	// printf("-------------------------------------\n");
	// printf("generate_disk_pc_id() : Inside man \n");
	size_t disk_num, available_disk_info = 0;
	FUNCTION_RETURN result_diskinfos;
	unsigned int i, j;
	int defined_identifiers;
	char firstChar;
	DiskInfo * diskInfos;

	result_diskinfos = getDiskInfos(NULL, &disk_num);
	//printf("generate_disk_pc_id() : Num of Disks : %d \n", disk_num);

	if (result_diskinfos != FUNC_RET_OK) {
		return result_diskinfos;
	}
	diskInfos = (DiskInfo*) malloc(disk_num * sizeof(DiskInfo));
	memset(diskInfos,0,disk_num * sizeof(DiskInfo));
	result_diskinfos = getDiskInfos(diskInfos, &disk_num);

	if (result_diskinfos != FUNC_RET_OK) {
		free(diskInfos);
		return result_diskinfos;
	}
	for (i = 0; i < disk_num; i++) {
		firstChar = use_label ? diskInfos[i].label[0] : diskInfos[i].disk_sn[0];
		available_disk_info += firstChar == 0 ? 0 : 1;
	}

	defined_identifiers = *num_identifiers;

	*num_identifiers = available_disk_info;

	if (identifiers == NULL) {
		free(diskInfos);
		return FUNC_RET_OK;
	} else if (available_disk_info > defined_identifiers) {
		free(diskInfos);
		return FUNC_RET_BUFFER_TOO_SMALL;
	}

	j = 0;
	for (i = 0; i < disk_num; i++) {
		if (use_label) {
			if (diskInfos[i].label[0] != 0) {
				memset(identifiers[j], 0, sizeof(PcIdentifier)); //!!!!!!!
				strncpy((char*)identifiers[j], diskInfos[i].label, sizeof(PcIdentifier));
				j++;
			}
		} else {
			if (diskInfos[i].disk_sn[0] != 0) {
				memcpy(identifiers[j], &diskInfos[i].disk_sn[2], sizeof(PcIdentifier));
				j++;
			}
		}
		//printf("generate_disk_pc_id() : ID num %d - %s \n", i, identifiers[j]);
	}
	free(diskInfos);
	//printf("-------------------------------------\n");
	return FUNC_RET_OK;
}