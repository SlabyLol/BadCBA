#ifndef BADCBA_FLASH_H
#define BADCBA_FLASH_H

/* Returns 1 if any USB is mounted and readable */
int  flash_usb_present(void);

/* Try common USB mount points, return first working path or NULL */
const char *flash_find_usb(void);

/* Backup original coldboot files from flash to HDD */
int  flash_backup_originals(void);

/* Install converted coldboot files to /dev_blind/vsh/resource/ */
int  flash_install_to_vsh(void);

/* Restore previously backed up coldboot files */
int  flash_restore_backup(void);

/* Status message for GUI */
const char *flash_get_status(void);

#endif
