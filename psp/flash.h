#ifndef BADCBA_PSP_FLASH_H
#define BADCBA_PSP_FLASH_H

int  flash_backup(void);
int  flash_install(void);
int  flash_restore(void);
const char *flash_status(void);

#endif
