#ifndef __KENAND_H
#define __KENAND_H

typedef struct _BLDR_HEADER{
	WORD Magic;
	WORD Build;
	WORD Qfe;
	WORD Flags;
	DWORD Entry;
	DWORD Size;
} BLDR_HEADER, *PBLDR_HEADER;

typedef struct _BLDR_FLASH{ // magic 0xFF4f
	BLDR_HEADER blHeader;
	char achCopyright[64];
	BYTE abReserved[16]; // 0x0 filled
	DWORD dwKeyVaultSize; // size of the keyvault
	DWORD dwSysUpdateAddr; // offset to first cf
	WORD wSysUpdateCount; // number of patch slots
	WORD wKeyVaultVersion; // 0x0712
	DWORD dwKeyVaultAddr; // offset of the keyvault from 0
	DWORD dwPatchSlotSize; // if == 0 then = 0x10000, patch slot size 
	DWORD dwSmcConfigAddr; // 0x0
	DWORD dwSmcBootSize; // size of smc.bin
	DWORD dwSmcBootAddr; // offset of smc.bin from 0
} BLDR_FLASH, *PBLDR_FLASH;

#endif // __KENAND_H
