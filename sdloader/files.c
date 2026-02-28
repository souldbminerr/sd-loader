#include "files.h"
#include <libs/fatfs/ff.h>
#include <utils/btn.h>
#include <utils/types.h>
#include <tui.h>

const char* drive_friendly_names[4] = {
    [SDLOADER_DRIVE_BOOT1]     = "BOOT 1",
    [SDLOADER_DRIVE_BOOT1_1MB] = "BOOT 1",
    [SDLOADER_DRIVE_SD]        = "SD",
    [SDLOADER_DRIVE_GPP]       = "GPP",
};

const char* drive_names[4] = {
    [SDLOADER_DRIVE_SD]        = "0:",
    [SDLOADER_DRIVE_BOOT1_1MB] = "2:",
    [SDLOADER_DRIVE_BOOT1]     = "1:",
    [SDLOADER_DRIVE_GPP]       = "3:",
};

static FATFS fs;
static u8 cur_drive = SDLOADER_DRIVE_INVALID;

FRESULT unmount_drive(){
    if(cur_drive != SDLOADER_DRIVE_INVALID){
        return f_mount(NULL, drive_names[cur_drive], 0);
    }
    return FR_OK;
}

static FRESULT mount_drive(u8 drive){
    if(cur_drive != drive){
        unmount_drive();
        cur_drive = SDLOADER_DRIVE_INVALID;
        FRESULT res = f_mount(&fs, drive_names[drive], 1);
        if(res == FR_OK){
            cur_drive = drive;
        }
        return res;
    }
    return FR_OK;
}

FRESULT open_file_on(const char *path, FIL *f, u8 drive){
    FRESULT res;
    if(drive > 3){
        return FR_INVALID_DRIVE;
    }
    res = mount_drive(drive);
    if(res != FR_OK){
        return res;
    }

    res = f_chdrive(drive_names[drive]);
    if(res != FR_OK){
        return res;
    }
    return f_open(f, path, FA_READ | FA_OPEN_EXISTING);
}

FRESULT open_file_on_any(const char *path, FIL *f, u8 *drive){
    FRESULT res;
    *drive = SDLOADER_DRIVE_INVALID;
    for(u32 i = 0; i < 4; i++){
        res = mount_drive(i);
        if(res != FR_OK){
            continue;
        }
        res = open_file_on(path, f, i);
        if(res == FR_NO_FILE){
            continue;
        }
        *drive = i;
        return res;
    }
    return FR_NO_FILE;
}

static void build_path(char *out, u32 out_size, const char *drive, const char *fname){
    u32 i = 0;
    // copy drive prefix e.g. "0:"
    for(u32 j = 0; drive[j] && i < out_size - 1; j++){
        out[i++] = drive[j];
    }
    // append slash
    if(i < out_size - 1){
        out[i++] = '/';
    }
    // append filename
    for(u32 j = 0; fname[j] && i < out_size - 1; j++){
        out[i++] = fname[j];
    }
    out[i] = '\0';
}

FRESULT find_and_open_file_on(const char *pattern, char *found_path,
                               u32 path_size, FIL *f, u8 drive){
    if(drive > 3){
        return FR_INVALID_DRIVE;
    }

    FRESULT res = mount_drive(drive);
    if(res != FR_OK){
        return res;
    }

    res = f_chdrive(drive_names[drive]);
    if(res != FR_OK){
        return res;
    }

    DIR dir;
    FILINFO fno;

    res = f_findfirst(&dir, &fno, "/", pattern);
    if(res == FR_OK && fno.fname[0] != '\0'){
        build_path(found_path, path_size, drive_names[drive], fno.fname);
        res = f_open(f, found_path, FA_READ | FA_OPEN_EXISTING);
    } else {
        res = FR_NO_FILE;
    }

    f_closedir(&dir);
    return res;
}

FRESULT find_and_open_file_on_any(const char *pattern, char *found_path,
                                   u32 path_size, FIL *f, u8 *drive){
    FRESULT res = FR_NO_FILE;
    *drive = SDLOADER_DRIVE_INVALID;

    for(u32 i = 0; i < 4; i++){
        res = mount_drive(i);
        if(res != FR_OK){
            continue;
        }
        res = find_and_open_file_on(pattern, found_path, path_size, f, i);
        if(res == FR_OK){
            *drive = i;
            return FR_OK;
        }
    }

    return FR_NO_FILE;
}