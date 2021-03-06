/**********************************************************************
 *
 * BLCRT: MSX-DOS2 Banked Application Loader
 *
 * 0100H~7FFFH: Banked Area
 * 8000H~8FFFH: Banking Helper
 * 9000H~93FFH: Internal shared buffer (1KBytes)
 * 9400H~     : Shared Heap
 *
 * 100602 - First version
 *
 *********************************************************************/

#ifdef BL_DISABLE

#ifdef BL_DOS1

#asm

; For CP/M, MSX-DOS1, MSX-DOS2

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text
	defs	100h		;Base of CP/M's TPA

	global	start, _main, _exit, __Hbss, __Lbss, __argc_, startup
	global	_GetBdosVersion, _gcBdosMode

	jp	start		;By Tatsu
				;On MS-DOS, return to MS-DOS
start:	ld	hl,(6)		;base address of fdos
	ld	sp,hl		;stack grows downwards
	ld	de,__Lbss	;Start of BSS segment
	or	a		;clear carry
	ld	hl,__Hbss
	sbc	hl,de		;size of uninitialized data area
	ld	c,l
	ld	b,h
	dec	bc
	ld	l,e
	ld	h,d
	inc	de
	ld	(hl),0
	ldir			;clear memory
	ld	hl,nularg
	push	hl
	ld	hl,80h		;argument buffer
	ld	c,(hl)
	inc	hl
	ld	b,0
	add	hl,bc
	ld	(hl),0		;zero terminate it
	ld	hl,81h
	push	hl
	call	startup
	pop	bc		;unjunk stack
	pop	bc
	push	hl
	ld	hl,(__argc_)
	push	hl

	; By Yeongman
	; Check BDOS version
	call	_GetBdosVersion
	ld	a,h		; 0:CP/M, 1:MSX-DOS1, 2:MSX-DOS2
	ld	(_gcBdosMode),a

	call	_main
	push	hl
	call	_exit
	jp	0

	psect	data
nularg:	defb	0
	end	start

#endasm

#else	/* BL_DOS1 */

#asm

;-------------------------------------------------------------------------------
; Startup code for MSX-DOS2 Only
;
	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text
	defs	100h		;Base of CP/M's TPA

	global	start, _main, _exit, __Hbss, __Lbss, __argc_, startup, wrelop

	jp	start		;By Tatsu
				;On MS-DOS, return to MS-DOS
start:
	ld	hl,(6)		;base address of fdos
	ld	sp,hl		;stack grows downwards

	ld	c,6fH		;Check MSX-DOS Version By Tatsu
	call	0005H
	or	a
	jp	nz,notdos2
	ld	a,b
	cp	2
	jp	c,notdos2

	ld	de,__Lbss	;Start of BSS segment
	or	a		;clear carry
	ld	hl,__Hbss
	sbc	hl,de		;size of uninitialized data area
	ld	c,l
	ld	b,h
	dec	bc
	ld	l,e
	ld	h,d
	inc	de
	ld	(hl),0
	ldir			;clear memory
	ld	hl,nularg
	push	hl
	ld	hl,80h		;argument buffer
	ld	c,(hl)
	inc	hl
	ld	b,0
	add	hl,bc
	ld	(hl),0		;zero terminate it
	ld	hl,81h
	push	hl
	call	startup
	pop	bc		;unjunk stack
	pop	bc
	push	hl
	ld	hl,(__argc_)
	push	hl
	call	_main
	push	hl
	call	_exit
	jp	0

notdos2:
	ld	de,notdos2_mes
	ld	c,9
	call	0005H
	jp	0

notdos2_mes:
	defm	'ERROR: MSX-DOS2 required.'
	defb	0dH,0aH,'$'

	psect	data
nularg:	defb	0

#endasm

#endif

#else	/* BL_DISABLE */

/* #define BL_DEBUG */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <msxbdos.h>
#include <cpumode.h>
#include <blstd.h>
#include <blstdint.h>
#include <blstdmem.h>
#include <blstdslt.h>

/* Bank Call Routine */
#include <bankcall.h>

#ifdef BL_DEBUG
#define bl_dbg_pr(A)		printf(A)
#define bl_dbg_pr_x(A, X)	printf(A, X)
#else
#define bl_dbg_pr(A)
#define bl_dbg_pr_x(A, X)
#endif

#asm

;-------------------------------------------------------------------------------
; Startup code for MSX-DOS2 Only
;
	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text
	defs	100h		;Base of CP/M's TPA

	global	start, _exit, __Hbss, __Lbss, __argc_, startup, wrelop

	jp	start		;By Tatsu
				;On MS-DOS, return to MS-DOS
start:
	ld	hl,(6)		;base address of fdos
	ld	sp,hl		;stack grows downwards

	ld	c,6fH		;Check MSX-DOS Version By Tatsu
	call	0005H
	or	a
	jp	nz,notdos2
	ld	a,b
	cp	2
	jp	c,notdos2

	ld	de,__Lbss	;Start of BSS segment
	or	a		;clear carry
	ld	hl,__Hbss
	sbc	hl,de		;size of uninitialized data area
	ld	c,l
	ld	b,h
	dec	bc
	ld	l,e
	ld	h,d
	inc	de
	ld	(hl),0
	ldir			;clear memory

	ld	hl,BankCallBin
	ld	de,BankCall_entry
	ld	bc,BankCall_size
	ldir			;transfer BankCall

	ld	hl,nularg
	push	hl
	ld	hl,80h		;argument buffer
	ld	c,(hl)
	inc	hl
	ld	b,0
	add	hl,bc
	ld	(hl),0		;zero terminate it
	ld	hl,81h
	push	hl
	call	startup
	pop	bc		;unjunk stack
	pop	bc
	push	hl
	ld	hl,(__argc_)
	push	hl
	call	_bl_main
	push	hl
	call	_exit
	jp	0

notdos2:
	ld	de,notdos2_mes
	ld	c,9
	call	0005H
	jp	0

notdos2_mes:
	defm	'ERROR: MSX-DOS2 required.'
	defb	0dH,0aH,'$'

	psect	data
nularg:	defb	0
#endasm


#ifdef __Hhimem
#asm
_himem_end:	defw	__Hhimem
#endasm
extern unsigned short himem_end;
#else
#define himem_end	0x9400
#endif	/* __Hhimem */

static int16_t free_seg_no;

#ifndef BL_1BANK
struct bl_mem_seg_t {				/* Memory Segment Info. */
	short BankMax;
	uint8_t BankTbl[64];
};
static struct bl_mem_seg_t tMemSeg;
static int8_t bl_tsr_mode = 0;
static char pOvlName[64];
static long OvlSize;

#ifdef BL_TSR
static unsigned char mem_seg_size = sizeof(tMemSeg);
static char pTsrEnvName[14];			/* ENV name */
static char *pTsrEnv = NULL;			/* ENV data */
static int8_t bl_tsr_env_exist = 0;
void MakeTsrEnvName(void);
void put_seg_table(void);
void get_seg_table(void);
#endif
#endif

uint8_t MapperInit(void);
uint8_t MapperAllocUser(void);
uint8_t MapperAllocSys(void);
void    MapperFree(uint8_t SegNo);

uint8_t MapperGetPage0(void);
uint8_t MapperGetPage1(void);
uint8_t MapperGetPage2(void);
void    MapperPutPage0(uint8_t SegNo);
void    MapperPutPage1(uint8_t SegNo);
void    MapperPutPage2(uint8_t SegNo);
void    MapperPutPage1_DI(uint8_t SegNo);

void    MakeOvlName(void);
void    BankCallInit(void);
void    ISRInit(void);
void    ISRDeinit(void);

void    copy_256_p0_to_p2(void);
void    put_lmem_seg_table(struct bl_lmem_t *ptr);
void    get_lmem_seg_table(struct bl_lmem_t *ptr);

long    _fsize(int fd);
void    brk(void *addr);
int     main(int argc, char *argv[]);		/* main() */

int bl_main(int argc, char *argv[])
{
	static int ret_val;
#ifndef BL_1BANK
	static uint8_t cFileHandle;
	static uint8_t seg, Page2Old;
	static int16_t SegCnt;
#endif
#ifdef R800ONLY
	if (get_msx_version() == MSXTR) {
		set_cpu_mode_tr(CPU_TR_R800_DRAM);
	} else {
		puts("ERROR: MSXturboR required.");
		return 0;
	}
#endif

	free_seg_no = MapperInit();
	bl_dbg_pr_x("[BL] Free seg = %d\n", free_seg_no);

#ifndef BL_1BANK
	MakeOvlName();				/* Get Full Path of *.OVL */

#ifdef BL_TSR
	MakeTsrEnvName();			/* Make Name of environment variable */
	pTsrEnv = getenv(pTsrEnvName);
	if (pTsrEnv && (pTsrEnv[0] == '*')) {
		bl_dbg_pr_x("[BL] Load memory info [%s] ...", pTsrEnvName);
		bl_tsr_mode = 1;
		bl_tsr_env_exist = 1;
		get_seg_table();		/* tMemSeg from pTsrEnv */
		bl_dbg_pr("Ok\n");
	}
#endif

	if (!bl_tsr_mode) {
		cFileHandle = open(pOvlName, O_RDONLY);		/* Open OVL File */
		if (cFileHandle == 0xFF) {			/* Not Found? */
			puts("ERROR: OVL not found");
			return 0;
		}

		OvlSize = _fsize(cFileHandle);
		tMemSeg.BankMax = (short)(OvlSize >> 15);
#ifdef BL_BMAX
		if (tMemSeg.BankMax > BL_BMAX)			/* Partial loading? */
			tMemSeg.BankMax = BL_BMAX;
#endif
		close(cFileHandle);

		if (tMemSeg.BankMax * 2 > free_seg_no) {
			puts("ERROR: Not enough memory");
			return 0;
		}

		bl_dbg_pr_x("[BL] Loading: %s\n", pOvlName);
	}
#endif

	/* Initialize Bank Caller */
	BankCallInit();

#ifndef BL_1BANK
	if (bl_tsr_mode) {
#asm
		; memcpy(Bank_idx_addr + 0x02, tMemSeg.BankTbl, sizeof(tMemSeg.BankTbl));
		LD	HL, _tMemSeg + 2		; HL = tMemSeg.BankTbl
		LD	DE, BankIndex_entry + 2		; DE = Bank_idx_addr + 0x02
		LD	BC, 64				; BC = sizeof(tMemSeg.BankTbl)
		LDIR
#endasm
		Page2Old = MapperGetPage2();

		for (SegCnt = 0; SegCnt < tMemSeg.BankMax * 2; SegCnt += 2) {
			MapperPutPage2(tMemSeg.BankTbl[SegCnt]);	/* Set segment */
			copy_256_p0_to_p2();
		}
		MapperPutPage2(Page2Old);		/* Set original segment */
	} else {
		/* Load Banked code via Page2 */
		cFileHandle = open(pOvlName, O_RDONLY);	/* re-open */
		Page2Old = MapperGetPage2();

#ifdef BL_TSR
		bl_dbg_pr("[BL] Allocate sys seg:");
#else
		bl_dbg_pr("[BL] Allocate usr seg:");
#endif
		for (SegCnt = 0; SegCnt < tMemSeg.BankMax * 2; SegCnt++, free_seg_no--) {
#ifdef BL_TSR
			seg = MapperAllocSys();		/* Allocate system segment */
#else
			seg = MapperAllocUser();	/* Allocate user segment */
#endif
			bl_dbg_pr_x(" %02X", seg);

			tMemSeg.BankTbl[SegCnt] = seg;
			*(Bank_idx_addr + 0x02 + SegCnt) = seg;

			MapperPutPage2(seg);    	/* Set segment */
			read(cFileHandle, (uint8_t *)0x8000, 0x4000);
			if (!(SegCnt & 0x01))		/* Page0 area?, Copy 0x0000~0x00FF */
				copy_256_p0_to_p2();

			MapperPutPage2(Page2Old);	/* Set original segment */
		}
		close(cFileHandle);
		bl_dbg_pr("\n");
	}
#endif
	/* Set DATA end to himem_end(over 0x9400) */
	brk((void *)himem_end);

	/* Install ISR */
	ISRInit();

	bl_dbg_pr_x("[BL] Free seg = %d\n", free_seg_no);

	/* Execute main() function */
	bl_dbg_pr("[BL] main() start\n");
	ret_val = main(argc, argv);
	bl_dbg_pr("[BL] main() done\n");

	/* Restore Original ISR */
	ISRDeinit();

#ifndef BL_1BANK
#ifdef BL_TSR
	if (bl_tsr_mode) {
		if (!bl_tsr_env_exist) {		/* TSR ENV not exist? */
			bl_dbg_pr_x("[BL] Save memory info [%s] ...", pTsrEnvName);
			*(unsigned char *)0x9000 = mem_seg_size;
			put_seg_table();		/* tMemSeg to temp heap */
			setenv(pTsrEnvName, (char *)0x9000);
			bl_dbg_pr("Ok\n");
		}
	} else {
		if (bl_tsr_env_exist)
			setenv(pTsrEnvName, "");	/* Clear ENV */

		/* Free all segment */
		bl_dbg_pr("[BL] Free segment:");
		for (SegCnt = 0; SegCnt < tMemSeg.BankMax * 2; SegCnt++, free_seg_no++) {
			MapperFree(tMemSeg.BankTbl[SegCnt]);
			bl_dbg_pr_x(" %02X", tMemSeg.BankTbl[SegCnt]);
		}
		bl_dbg_pr("\n");
	}
#else
	/* Free all segment */
	bl_dbg_pr("[BL] Free segment:");
	for (SegCnt = 0; SegCnt < tMemSeg.BankMax * 2; SegCnt++, free_seg_no++) {
		MapperFree(tMemSeg.BankTbl[SegCnt]);
		bl_dbg_pr_x(" %02X", tMemSeg.BankTbl[SegCnt]);
	}
	bl_dbg_pr("\n");
#endif
#endif
	bl_dbg_pr_x("[BL] Free seg = %d\n", free_seg_no);

	return ret_val;
}

#ifndef BL_1BANK
void bl_get_ovl_info(char *name, short *bank_max)
{
	strcpy(name, pOvlName);
	*bank_max = tMemSeg.BankMax;
}

uint16_t bl_get_seg_info(short bank)
{
	return *((uint16_t *)Bank_idx_addr + bank);
}

void bl_tsr_on(void)
{
#ifdef BL_TSR
	bl_tsr_mode = 1;
#endif
}

void bl_tsr_off(void)
{
	bl_tsr_mode = 0;
}

int8_t bl_is_tsr_on(void)
{
	return bl_tsr_mode;
}
#endif

uint32_t bl_lmem_get_free(void)
{
	static uint32_t size_byte;

	size_byte = 0x4000;
	size_byte *= free_seg_no;

	return size_byte;
}

static uint8_t lmem_sys_seg = 0;
static uint8_t lmem_page_no;
static uint16_t lmem_offset;
struct bl_lmem_t *bl_lmem_alloc(uint32_t size)
{
	static struct bl_lmem_t *lmem;
	uint8_t page_no, n;

	/* printf("lmem alloc size = %lu bytes\n", size); */
	page_no = (uint8_t)((size + 0x3FFF) >> 14);	/* N page */
	if (!page_no || (page_no > free_seg_no)) {
		return NULL;
	}

	lmem = (struct bl_lmem_t *)malloc(sizeof(struct bl_lmem_t));
	if (lmem == NULL) {		/* not enough heap */
		return NULL;
	}

	free_seg_no -= page_no;
	lmem->page_max = page_no;
	lmem->sys_used = lmem_sys_seg;
	for (n = 0; n < page_no; n++) {
		if (lmem_sys_seg)
			lmem->page_tbl[n] = MapperAllocSys();
		else
			lmem->page_tbl[n] = MapperAllocUser();
		/* printf("seg no %02X\n", lmem->page_tbl[n]); */
	}
	/* printf("Free seg = %d\n", free_seg_no); */

	return lmem;
}

struct bl_lmem_t *bl_lmem_alloc_sys(uint32_t size)
{
	static struct bl_lmem_t *lmem;

	lmem_sys_seg = 1;		/* for system segment */
	lmem = bl_lmem_alloc(size);
	lmem_sys_seg = 0;

	return lmem;
}

void bl_lmem_free(struct bl_lmem_t *ptr)
{
	uint8_t n;

	if (ptr) {
		free_seg_no += ptr->page_max;
		for (n = 0; n < ptr->page_max; n++) {
			MapperFree(ptr->page_tbl[n]);
		}
		free(ptr);

/*		printf("Free seg = %d\n", free_seg_no);*/
	}
}

uint8_t bl_lmem_get_seg(struct bl_lmem_t *ptr, uint32_t addr32)
{
	return ptr->page_tbl[(uint8_t)(addr32 >> 14)];
}

void bl_lmem_export(struct bl_lmem_t *ptr, char *name)
{
	if (ptr->sys_used) {
		*(unsigned char *)0x9000 = ptr->page_max;
		put_lmem_seg_table(ptr);
		setenv(name, (char *)0x9000);
	}
}

struct bl_lmem_t *bl_lmem_import(char *name)
{
	static struct bl_lmem_t *lmem;
	static char *env;

	lmem = (struct bl_lmem_t *)malloc(sizeof(struct bl_lmem_t));
	if (lmem == NULL) {		/* not enough heap */
		return NULL;
	}

	env = getenv(name);
	if (env[0] == '*') {
		strcpy((char *)0x9000, env);
		get_lmem_seg_table(lmem);
		lmem->page_max = env[1] - 0x20;
		lmem->sys_used = 1;
	} else {			/* env not found */
		free(lmem);
		return NULL;
	}

	return lmem;
}

void bl_lmem_copy_to(struct bl_lmem_t *dest, uint32_t addr32, uint8_t *src, uint16_t size)
{
	/* uint8_t page1_seg_old = *(BankIndex_addr + 1); */
#asm
	DI
#endasm
	lmem_page_no = (uint8_t)(addr32 >> 14);
	lmem_offset = ((uint16_t)addr32 & 0x3FFF) | 0x4000;

	MapperPutPage1_DI(dest->page_tbl[lmem_page_no]);
	while (size--) {
		*((uint8_t *)lmem_offset++) = *src++;
		if ((lmem_offset == 0x8000) && (size > 1)) {	/* End of mapped area? */
			lmem_offset = 0x4000;
			lmem_page_no++;				/* for next mem page */
			MapperPutPage1_DI(dest->page_tbl[lmem_page_no]);
		}
	}
	/* MapperPutPage1(page1_seg_old); */
}

void bl_lmem_copy_from(uint8_t *dest, struct bl_lmem_t *src, uint32_t addr32, uint16_t size)
{
	/* uint8_t page1_seg_old = *(BankIndex_addr + 1); */
#asm
	DI
#endasm
	lmem_page_no = (uint8_t)(addr32 >> 14);
	lmem_offset = (((uint16_t)addr32) & 0x3FFF) | 0x4000;

	MapperPutPage1_DI(src->page_tbl[lmem_page_no]);
	while (size--) {
		*dest++ = *((uint8_t *)lmem_offset++);
		if ((lmem_offset == 0x8000) && (size > 1)) {	/* End of mapped area? */
			lmem_offset = 0x4000;
			lmem_page_no++;				/* for next mem page */
			MapperPutPage1_DI(src->page_tbl[lmem_page_no]);
		}
	}
	/* MapperPutPage1(page1_seg_old); */
}

#ifndef BL_1BANK
#asm
;-------------------------------------------------------------------------------
; Fill pOvlName[]
;
;void MakeOvlName(void);

		psect	text
		global	_getenv, _strcpy, _strlen
_MakeOvlName:
		PUSH	IX

		LD	HL, _str_program
		PUSH	HL
		CALL	_getenv			; HL <- full path
		PUSH	HL
		LD	HL, _pOvlName
		PUSH	HL
		CALL	_strcpy
		CALL	_strlen			; HL <- length
		POP	BC			; cleanup stack
		POP	BC
		POP	BC

		LD	DE, _pOvlName - 3
		ADD	HL, DE
		LD	(HL), 'O'
		INC	HL
		LD	(HL), 'V'
		INC	HL
		LD	(HL), 'L'

		POP IX
		RET

_str_program:	DEFB	'P','R','O','G','R','A','M',0
#endasm
#endif

#asm
;-------------------------------------------------------------------------------
; Initialize Banking Helper Routine
;
;void BankCallInit(void);

_BankCallInit:
		CALL _MapperGetPage0
		LD E,L				; Page0 segment
		CALL _MapperGetPage1
		LD D,L				; Page1 segment
		LD (BankIndex_entry),DE		; Set Bank0 table
_BankCallInit_P0a:
		LD HL, 0			; LD HL,(_MapperPUT_P0)
		JP BankInit_entry

;-------------------------------------------------------------------------------
; Interrupt Service Routine
;
;void ISRInit(void);
;void ISRDeinit(void);

_ISRInit:	JP ISRInit_entry
_ISRDeinit:	JP ISRDeinit_entry

;-------------------------------------------------------------------------------
; Initialize Mapper Routine
;
;uint8_t MapperInit(void)

_MapperInit:
		PUSH IY
		PUSH IX

		LD A,000H
		LD DE,0402H			; parameter
		CALL 0FFCAH			; EXTBIO
		LD (_MapperSegTotal),A		; Total number of segment
		LD A,C
		LD (_MapperSegFree),A		; Free segment number
		LD (_MapperTblAddr),HL		; Base address

		LD (_MapperAlloc_a + 1),HL	; ALL_SEG
		LD DE,3
		ADD HL,DE			; FRE_SEG
		LD (_MapperFree_a + 1),HL
		ADD HL,DE			; RD_SEG
		ADD HL,DE			; WR_SEG
		ADD HL,DE			; CAL_SEG
		ADD HL,DE			; CALLS
		ADD HL,DE			; PUT_PH
		ADD HL,DE			; GET_PH
		ADD HL,DE			; PUT_P0
		LD (_BankCallInit_P0a + 1),HL
		LD (_MapperPutPage0a + 1),HL
		ADD HL,DE			; GET_P0
		LD (_MapperGetPage0a + 1),HL
		ADD HL,DE			; PUT_P1
		LD (_MapperPutPage1a + 1),HL
		LD (_MapperPut_P1d_a + 1),HL
		ADD HL,DE			; GET_P1
		LD (_MapperGetPage1a + 1),HL
		ADD HL,DE			; PUT_P2
		LD (_MapperPutPage2a + 1),HL
		ADD HL,DE			; GET_P2
		LD (_MapperGetPage2a + 1),HL
;		ADD HL,DE			; PUT_P3
;		ADD HL,DE			; GET_P3

		POP IX
		POP IY

		LD L,C				; return value (Free segment number)
		RET
_MapperSegTotal:
		DEFB 0
_MapperSegFree:
		DEFB 0
_MapperTblAddr:
		DEFW 0


;-------------------------------------------------------------------------------
; Allocate & Free Mapper Page
;
;uint8_t MapperAllocUser(void)
;uint8_t MapperAllocSys(void)
;void MapperFree(uint8_t SegNo)

_MapperAllocUser:
		LD A,000H			; for user segment
		LD B,000H			; for primary mapper
		JR _MapperAlloc

_MapperAllocSys:
		LD A,001H			; for system segment
		LD B,000H			; for primary mapper

_MapperAlloc:
		PUSH IY
		PUSH IX
_MapperAlloc_a:
		CALL 0				; CALL ALL_SEG
		JR NC,_MapperAlloc_ok
		LD A,0FFH			; FF means error
_MapperAlloc_ok:
		LD L,A				; return value (allocated segment number)
		JR _Mapper_ret

_MapperFree:
		POP BC				; Return Addr
		POP DE				; E = SegNo
		PUSH DE
		PUSH BC

		PUSH IY
		PUSH IX

		LD A,E				; SegNo
		LD B,000H
_MapperFree_a:
		CALL 0				; CALL FRE_SEG
_Mapper_ret:
		POP IX
		POP IY
		EI
		RET

;-------------------------------------------------------------------------------
; Get Mapper Page
;
;uint8_t MapperGetPage0(void)
;uint8_t MapperGetPage1(void)
;uint8_t MapperGetPage2(void)

_MapperGetPage0:
		PUSH IY
		PUSH IX
_MapperGetPage0a:
		CALL 0				; CALL GET_P0
		JR _MapperGetPage_ret

_MapperGetPage1:
		PUSH IY
		PUSH IX
_MapperGetPage1a:
		CALL 0				; CALL GET_P1
		JR _MapperGetPage_ret

_MapperGetPage2:
		PUSH IY
		PUSH IX
_MapperGetPage2a:
		CALL 0				; CALL GET_P2
;		JR _MapperGetPage_ret

_MapperGetPage_ret:
		POP IX
		POP IY

		LD H,0
		LD L,A				; Segment No
		EI
		RET

;-------------------------------------------------------------------------------
;void MapperPutPage0(uint8_t SegNo)
;void MapperPutPage1(uint8_t SegNo)
;void MapperPutPage2(uint8_t SegNo)

_MapperPutPage0:
_MapperPutPage0a:
		LD HL,0				; LD HL,(_MapperPUT_P0)
		JR _MapperPutPageN

_MapperPutPage1:
_MapperPutPage1a:
		LD HL,0				; LD HL,(_MapperPUT_P1)
		JR _MapperPutPageN

_MapperPutPage2:
_MapperPutPage2a:
		LD HL,0				; LD HL,(_MapperPUT_P2)
;		JR _MapperPutPageN

_MapperPutPageN:
		POP BC				; Return Addr
		POP DE				; E = SegNo
		PUSH DE
		PUSH BC

		PUSH IY
		PUSH IX

		LD A,E				; SegNo
		LD (_MapperPut_Pn_a + 1),HL
_MapperPut_Pn_a:
		CALL 0				; CALL PUT_Pn

		POP IX
		POP IY
		EI
		RET

;-----------------------------------------------------------------
;void MapperPutPage1_DI(uint8_t SegNo)
;L = SegNo
_MapperPutPage1_DI:
		DI
		PUSH IY
		PUSH IX

		LD A,L				; SegNo
_MapperPut_P1d_a:
		CALL 0				; CALL PUT_P1

		POP IX
		POP IY
		RET

;-------------------------------------------------------------------------------
; Shared Heap Management
;
;void *bl_calloc(uint16_t, uint16_t)
;void  bl_free(void *)
;void *bl_malloc(uint16_t)
;void *bl_realloc(void *, uint16_t)
;void *bl_get_memtop(void)
		GLOBAL _bl_calloc,_bl_free,_bl_malloc,_bl_realloc,_bl_get_memtop
		GLOBAL _calloc,_free,_malloc,_realloc,memtop

_bl_calloc:	JP _calloc
_bl_free:	JP _free
_bl_malloc:	JP _malloc
_bl_realloc:	JP _realloc
_bl_get_memtop:	LD HL,(memtop)
		RET
#endasm


#ifndef BL_1BANK
#asm
;-------------------------------------------------------------------------------
; Copy DOS system scratch 256bytes to 8000H
; Used: BC, DE, HL
;void copy_256_p0_to_p2(void)
_copy_256_p0_to_p2:
		LD HL,00000H
		LD DE,08000H
		LD BC,256
		LDIR
		RET
#endasm

#ifdef BL_TSR
#asm
;-------------------------------------------------------------------------------
; Fill pTsrEnvName[]
;
;void MakeTsrEnvName(void);

		psect	text
		global	_strcpy, _strlen
_MakeTsrEnvName:
		PUSH	IX

		LD	HL, _pOvlName
		LD	DE, _pOvlName
_MakeTsrEnvName_l:
		LD	A, (HL)
		AND	A
		JR	Z, _MakeTsrEnvName_0
		CP	'\'
		INC	HL
		JR	NZ, _MakeTsrEnvName_l
		LD	D, H
		LD	E, L			; DE <- filename only
		JR	_MakeTsrEnvName_l
_MakeTsrEnvName_0:
		PUSH	DE
		LD	HL, _pTsrEnvName
		PUSH	HL
		CALL	_strcpy
		CALL	_strlen			; HL <- length
		POP	BC			; cleanup stack
		POP	BC

		LD	DE, _pTsrEnvName - 4
		ADD	HL, DE
		LD	(HL), '_'
		INC	HL
		LD	(HL), 'T'
		INC	HL
		LD	(HL), 'S'
		INC	HL
		LD	(HL), 'R'

		POP IX
		RET

;-------------------------------------------------------------------------------
; Put memory segment information to env-string
;
;void put_seg_table(void)
		GLOBAL _put_seg_table
_put_seg_table:
		PUSH BC
		PUSH DE
		PUSH HL

		LD HL,_tMemSeg
		LD DE,09000H
		LD A,(HL)			; BankMax value
		INC A
		RLCA				; info *2 bytes

		JP _put_seg_main

;-------------------------------------------------------------------------------
; Get memory segment information from env-string
;
;void get_seg_table(void)
		GLOBAL _get_seg_table
_get_seg_table:
		PUSH BC
		PUSH DE
		PUSH HL

		LD HL,(_pTsrEnv)
		LD DE,_tMemSeg
		LD A,(_mem_seg_size)

		JP _get_seg_main

#endasm
#endif	/* BL_TSR */

#endif	/* BL_1BANK */

#asm
;-------------------------------------------------------------------------------
; Put lmem segment information to env-string
;
;void put_lmem_seg_table(struct bl_lmem_t *ptr)
_put_lmem_seg_table:
		PUSH BC
		PUSH DE
		PUSH HL				; HL = ptr

		LD DE,09000H
		LD A,(DE)			; segment count

		JP _put_seg_main

;-------------------------------------------------------------------------------
; Get lmem segment information from env-string
;
;void get_lmem_seg_table(struct bl_lmem_t *ptr)
_get_lmem_seg_table:
		PUSH BC
		PUSH DE
		PUSH HL				; HL = ptr

		EX DE,HL			; DE = ptr
		LD HL,09001H			; Skip Head '*'
		LD A,(HL)
		SUB 020H			; segment count
		DEC HL				; HL = 9000H

		JP _get_seg_main

;-------------------------------------------------------------------------------
; _put_seg_main
; HL = segment table address
; DE = target environment string address
; A  = segment count
_put_seg_main:
		LD B,A				; Loop counter
		LD A,'*'			; Add Head '*'
		LD (DE),A
		INC DE
		LD A,B
		ADD A,020H			; count + 0x20
		LD (DE),A
		INC DE
_put_seg_loop:
		LD A,(HL)
		AND 00FH			; Low 4bits
		OR 040H
		LD (DE),A
		INC DE
		LD A,(HL)
		RRA
		RRA
		RRA
		RRA
		AND 00FH			; High 4bits
		OR 040H
		LD (DE),A
		INC DE
		INC HL
		DJNZ _put_seg_loop

		XOR A
		LD (DE),A			; Null-end string

		POP HL
		POP DE
		POP BC
		RET

;-------------------------------------------------------------------------------
; _get_seg_main
; HL = environment string address
; DE = target segment table address
; A  = segment count
_get_seg_main:
		LD B,A
		INC HL				; Skip Head '*'
		INC HL				; Skip (count + 0x20)
_get_seg_loop:
		LD A,(HL)
		INC HL
		AND A
		JR Z,_get_seg_end		; end?
		AND 00FH			; Low 4bits
		LD C,A
		LD A,(HL)
		INC HL
		RLA
		RLA
		RLA
		RLA
		AND 0F0H			; High 4bits
		OR C
		LD (DE),A
		INC DE
		DJNZ _get_seg_loop
_get_seg_end:
		POP HL
		POP DE
		POP BC
		RET
#endasm

;
