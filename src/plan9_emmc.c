#include <device/gpio.h>
#include <device/dma.h>
#include <device/plan9_emmc.h>
#include <plibc/stdio.h>
#include <kernel/rpi-base.h>
#include <kernel/rpi-interrupts.h>
#include <kernel/systimer.h>
#include <kernel/rpi-mailbox-interface.h>

#define EMMCREGS	(PERIPHERAL_BASE + 0x300000)
#define Mhz 1000 * 1000

enum {
	Extfreq		= 100*Mhz,	/* guess external clock frequency if */
					/* not available from vcore */
	Initfreq	= 400000,	/* initialisation frequency for MMC */
	SDfreq		= 25*Mhz,	/* standard SD frequency */
	SDfreqhs	= 50*Mhz,	/* high speed frequency */
	DTO		= 14,		/* data timeout exponent (guesswork) */

	GoIdle		= 0,		/* mmc/sdio go idle state */
	MMCSelect	= 7,		/* mmc/sd card select command */
	Setbuswidth	= 6,		/* mmc/sd set bus width command */
	Switchfunc	= 6,		/* mmc/sd switch function command */
	Voltageswitch = 11,		/* md/sdio switch to 1.8V */
	IORWdirect = 52,		/* sdio read/write direct command */
	IORWextended = 53,		/* sdio read/write extended command */
	Appcmd = 55,			/* mmc/sd application command prefix */
};

enum {
	/* Controller registers */
	Arg2			= 0x00>>2,
	Blksizecnt		= 0x04>>2,
	Arg1			= 0x08>>2,
	Cmdtm			= 0x0c>>2,
	Resp0			= 0x10>>2,
	Resp1			= 0x14>>2,
	Resp2			= 0x18>>2,
	Resp3			= 0x1c>>2,
	Data			= 0x20>>2,
	Status			= 0x24>>2,
	Control0		= 0x28>>2,
	Control1		= 0x2c>>2,
	Interrupt		= 0x30>>2,
	Irptmask		= 0x34>>2,
	Irpten			= 0x38>>2,
	Control2		= 0x3c>>2,
	Forceirpt		= 0x50>>2,
	Boottimeout		= 0x70>>2,
	Dbgsel			= 0x74>>2,
	Exrdfifocfg		= 0x80>>2,
	Exrdfifoen		= 0x84>>2,
	Tunestep		= 0x88>>2,
	Tunestepsstd		= 0x8c>>2,
	Tunestepsddr		= 0x90>>2,
	Spiintspt		= 0xf0>>2,
	Slotisrver		= 0xfc>>2,

	/* Control0 */
	Hispeed			= 1<<2,
	Dwidth4			= 1<<1,
	Dwidth1			= 0<<1,

	/* Control1 */
	Srstdata		= 1<<26,	/* reset data circuit */
	Srstcmd			= 1<<25,	/* reset command circuit */
	Srsthc			= 1<<24,	/* reset complete host controller */
	Datatoshift		= 16,		/* data timeout unit exponent */
	Datatomask		= 0xF0000,
	Clkfreq8shift		= 8,		/* SD clock base divider LSBs */
	Clkfreq8mask		= 0xFF00,
	Clkfreqms2shift		= 6,		/* SD clock base divider MSBs */
	Clkfreqms2mask		= 0xC0,
	Clkgendiv		= 0<<5,		/* SD clock divided */
	Clkgenprog		= 1<<5,		/* SD clock programmable */
	Clken			= 1<<2,		/* SD clock enable */
	Clkstable		= 1<<1,	
	Clkintlen		= 1<<0,		/* enable internal EMMC clocks */

	/* Cmdtm */
	Indexshift		= 24,
	Suspend			= 1<<22,
	Resume			= 2<<22,
	Abort			= 3<<22,
	Isdata			= 1<<21,
	Ixchken			= 1<<20,
	Crcchken		= 1<<19,
	Respmask		= 3<<16,
	Respnone		= 0<<16,
	Resp136			= 1<<16,
	Resp48			= 2<<16,
	Resp48busy		= 3<<16,
	Multiblock		= 1<<5,
	Host2card		= 0<<4,
	Card2host		= 1<<4,
	Autocmd12		= 1<<2,
	Autocmd23		= 2<<2,
	Blkcnten		= 1<<1,

	/* Interrupt */
	Acmderr		= 1<<24,
	Denderr		= 1<<22,
	Dcrcerr		= 1<<21,
	Dtoerr		= 1<<20,
	Cbaderr		= 1<<19,
	Cenderr		= 1<<18,
	Ccrcerr		= 1<<17,
	Ctoerr		= 1<<16,
	Err		= 1<<15,
	Cardintr	= 1<<8,
	Cardinsert	= 1<<6,		/* not in Broadcom datasheet */
	Readrdy		= 1<<5,
	Writerdy	= 1<<4,
	Datadone	= 1<<1,
	Cmddone		= 1<<0,

	/* Status */
	Bufread		= 1<<11,	/* not in Broadcom datasheet */
	Bufwrite	= 1<<10,	/* not in Broadcom datasheet */
	Readtrans	= 1<<9,
	Writetrans	= 1<<8,
	Datactive	= 1<<2,
	Datinhibit	= 1<<1,
	Cmdinhibit	= 1<<0,
};

static int cmdinfo[64] = {
[0]  Ixchken,
[2]  Resp136,
[3]  Resp48 | Ixchken | Crcchken,
[5]  Resp48,
[6]  Resp48 | Ixchken | Crcchken,
[7]  Resp48busy | Ixchken | Crcchken,
[8]  Resp48 | Ixchken | Crcchken,
[9]  Resp136,
[11] Resp48 | Ixchken | Crcchken,
[12] Resp48busy | Ixchken | Crcchken,
[13] Resp48 | Ixchken | Crcchken,
[16] Resp48,
[17] Resp48 | Isdata | Card2host | Ixchken | Crcchken,
[18] Resp48 | Isdata | Card2host | Multiblock | Blkcnten | Ixchken | Crcchken,
[24] Resp48 | Isdata | Host2card | Ixchken | Crcchken,
[25] Resp48 | Isdata | Host2card | Multiblock | Blkcnten | Ixchken | Crcchken,
[41] Resp48,
[52] Resp48 | Ixchken | Crcchken,
[53] Resp48	| Ixchken | Crcchken | Isdata,
[55] Resp48 | Ixchken | Crcchken,
};

#define USED(x) if(x);else{}
#define nelem(x)	(sizeof(x)/sizeof((x)[0]))

typedef struct Ctlr Ctlr;

struct Ctlr {
	int	fastclock;
	uint32_t	extclk;
	int	appcmd;
};

static Ctlr emmc;

static void mmcinterrupt();

static void
WR(int reg, uint32_t val)
{
	uint32_t *r = (uint32_t*)EMMCREGS;

	if(0)printf("WR %2.2ux %ux\n", reg<<2, val);
	MicroDelay(emmc.fastclock? 2 : 20);
	// coherence();
	r[reg] = val;
}

static uint32_t get_core_clock() {
    RPI_PropertyInit();
	RPI_PropertyAddTag(TAG_GET_CLOCK_RATE, 0x000000001);
    RPI_PropertyProcess();
	rpi_mailbox_property_t *mp = RPI_PropertyGet(TAG_GET_CLOCK_RATE);

    uint32_t clk_id = (uint32_t)(mp->data.buffer_32[0]);
    uint32_t clk_rate = (uint32_t)(mp->data.buffer_32[1]);
	printf(" clk_id: %d clk_rate: %d ", clk_id, clk_rate);
    return clk_rate;
}

static uint32_t clkdiv(uint32_t d)
{
	uint32_t v;

	// assert(d < 1<<10);
	v = (d << Clkfreq8shift) & Clkfreq8mask;
	v |= ((d >> 8) << Clkfreqms2shift) & Clkfreqms2mask;
	return v;
}

static void emmcclk(uint32_t freq)
{
	uint32_t *r;
	uint32_t div;
	int i;

	r = (uint32_t*)EMMCREGS;
	div = emmc.extclk / (freq<<1);
	if(emmc.extclk / (div<<1) > freq)
		div++;
	WR(Control1, clkdiv(div) |
		DTO<<Datatoshift | Clkgendiv | Clken | Clkintlen);
	for(i = 0; i < 1000; i++){
		MicroDelay(1);
		if(r[Control1] & Clkstable)
			break;
	}
	if(i == 1000)
		printf("emmc: can't set clock to %ud\n", freq);
}

// static int datadone(void)
// {
// 	int i;

// 	uint32_t *r = (uint32_t*)EMMCREGS;
// 	i = r[Interrupt];
// 	return i & (Datadone|Err);
// }

// static int cardintready(void)
// {
// 	int i;

// 	uint32_t *r = (uint32_t*)EMMCREGS;
// 	i = r[Interrupt];
// 	return i & Cardintr;
// }

int emmcinit(void)
{
	uint32_t *r;
	uint32_t clk;

	clk = get_core_clock();
	if(clk == 0){
		clk = Extfreq;
		printf("emmc: assuming external clock %lud Mhz\n", clk/1000000);
	}
	emmc.extclk = clk;
	r = (uint32_t*)EMMCREGS;
	if(0)printf("emmc control %8.8ux %8.8ux %8.8ux\n",
		r[Control0], r[Control1], r[Control2]);
	WR(Control1, Srsthc);
	MicroDelay(10);
	while(r[Control1] & Srsthc)
		;
	WR(Control1, Srstdata);
	MicroDelay(10);
	WR(Control1, 0);
	return 0;
}

int emmcinquiry(char *inquiry, int inqlen)
{
	uint32_t *r;
	uint32_t ver;
	USED(inquiry);
	USED(inqlen);
	r = (uint32_t*)EMMCREGS;
	ver = r[Slotisrver] >> 16;
	printf("Arasan eMMC SD Host Controller %2.2x Version %2.2x",	ver&0xFF, ver>>8);
	return 0;
}

static void mmc_interrupt_handler() {
	printf(" ");
}

void emmcenable(void)
{
	emmcclk(Initfreq);
	WR(Irpten, 0);
	WR(Irptmask, ~0);
	WR(Interrupt, ~0);
	// intrenable(IRQmmc, mmcinterrupt, nil, 0, "mmc");
	register_irq_handler(62, mmc_interrupt_handler, mmcinterrupt);
}

// static int sdiocardintr(int wait)
// {
// 	uint32_t *r;
// 	int i;

// 	r = (uint32_t*)EMMCREGS;
// 	WR(Interrupt, Cardintr);
// 	while(((i = r[Interrupt]) & Cardintr) == 0){
// 		if(!wait)
// 			return 0;
// 		WR(Irpten, r[Irpten] | Cardintr);
// 		// sleep(&emmc.cardr, cardintready, 0);
// 	}
// 	WR(Interrupt, Cardintr);
// 	return i;
// }

int emmccmd(uint32_t cmd, uint32_t arg, uint32_t *resp)
{
	uint32_t *r;
	uint32_t c;
	int i;
	uint64_t now;

	r = (uint32_t*)EMMCREGS;
	// assert(cmd < nelem(cmdinfo) && cmdinfo[cmd] != 0);
	c = (cmd << Indexshift) | cmdinfo[cmd];
	/*
	 * CMD6 may be Setbuswidth or Switchfunc depending on Appcmd prefix
	 */
	if(cmd == Switchfunc && !emmc.appcmd)
		c |= Isdata|Card2host;
	if(cmd == IORWextended){
		if(arg & (1<<31))
			c |= Host2card;
		else
			c |= Card2host;
		if((r[Blksizecnt]&0xFFFF0000) != 0x10000)
			c |= Multiblock | Blkcnten;
	}
	/*
	 * GoIdle indicates new card insertion: reset bus width & speed
	 */
	if(cmd == GoIdle){
		WR(Control0, r[Control0] & ~(Dwidth4|Hispeed));
		emmcclk(Initfreq);
	}
	if(r[Status] & Cmdinhibit){
		printf("emmccmd: need to reset Cmdinhibit intr %ux stat %ux\n",
			r[Interrupt], r[Status]);
		WR(Control1, r[Control1] | Srstcmd);
		while(r[Control1] & Srstcmd)
			;
		while(r[Status] & Cmdinhibit)
			;
	}
	if((r[Status] & Datinhibit) &&
	   ((c & Isdata) || (c & Respmask) == Resp48busy)){
		printf("emmccmd: need to reset Datinhibit intr %ux stat %ux\n",
			r[Interrupt], r[Status]);
		WR(Control1, r[Control1] | Srstdata);
		while(r[Control1] & Srstdata)
			;
		while(r[Status] & Datinhibit)
			;
	}
	WR(Arg1, arg);
	if((i = (r[Interrupt] & ~Cardintr)) != 0){
		if(i != Cardinsert)
			printf("emmc: before command, intr was %ux\n", i);
		WR(Interrupt, i);
	}
	WR(Cmdtm, c);
	now = timer_getTickCount64();
	while(((i=r[Interrupt])&(Cmddone|Err)) == 0)
		if(timer_getTickCount64() - now > 1000)
			break;
	if((i&(Cmddone|Err)) != Cmddone){
		if((i&~(Err|Cardintr)) != Ctoerr)
			printf("emmc: cmd %ux arg %ux error intr %ux stat %ux\n", c, arg, i, r[Status]);
		WR(Interrupt, i);
		if(r[Status]&Cmdinhibit){
			WR(Control1, r[Control1]|Srstcmd);
			while(r[Control1]&Srstcmd)
				;
		}
		// error(Eio);
		printf(" Error: \n");
		return -1;
	}
	WR(Interrupt, i & ~(Datadone|Readrdy|Writerdy));
	switch(c & Respmask){
	case Resp136:
		resp[0] = r[Resp0]<<8;
		resp[1] = r[Resp0]>>24 | r[Resp1]<<8;
		resp[2] = r[Resp1]>>24 | r[Resp2]<<8;
		resp[3] = r[Resp2]>>24 | r[Resp3]<<8;
		break;
	case Resp48:
	case Resp48busy:
		resp[0] = r[Resp0];
		break;
	case Respnone:
		resp[0] = 0;
		break;
	}
	if((c & Respmask) == Resp48busy){
		WR(Irpten, r[Irpten]|Datadone|Err);
		MicroDelay(3000);
		i = r[Interrupt];
		if((i & Datadone) == 0)
			printf("emmcio: no Datadone after CMD%d\n", cmd);
		if(i & Err)
			printf("emmcio: CMD%d error interrupt %ux\n",
				cmd, r[Interrupt]);
		WR(Interrupt, i);
	}
	/*
	 * Once card is selected, use faster clock
	 */
	if(cmd == MMCSelect){
		MicroDelay(1);
		emmcclk(SDfreq);
		MicroDelay(1);
		emmc.fastclock = 1;
	}
	if(cmd == Setbuswidth){
		if(emmc.appcmd){
			/*
			 * If card bus width changes, change host bus width
			 */
			switch(arg){
			case 0:
				WR(Control0, r[Control0] & ~Dwidth4);
				break;
			case 2:
				WR(Control0, r[Control0] | Dwidth4);
				break;
			}
		}else{
			/*
			 * If card switched into high speed mode, increase clock speed
			 */
			if((arg&0x8000000F) == 0x80000001){
				MicroDelay(1);
				emmcclk(SDfreqhs);
				MicroDelay(1);
			}
		}
	}else if(cmd == IORWdirect && (arg & ~0xFF) == (1U<<31|0<<28|7U<<9)){
		switch(arg & 0x3){
		case 0:
			WR(Control0, r[Control0] & ~Dwidth4);
			break;
		case 2:
			WR(Control0, r[Control0] | Dwidth4);
			//WR(Control0, r[Control0] | Hispeed);
			break;
		}
	}
	emmc.appcmd = (cmd == Appcmd);
	return 0;
}

void emmciosetup(int write, void *buf, int bsize, int bcount)
{
	USED(write);
	USED(buf);
	WR(Blksizecnt, bcount<<16 | bsize);
}

void emmcio(int write, uint8_t *buf, int len)
{
	uint32_t *r;
	int i;

	r = (uint32_t*)EMMCREGS;
	// assert((len&3) == 0);
	// okay(1);
	// if(waserror()){
	// 	okay(0);
	// 	nexterror();
	// }
	if(write)
		dma_start(4, 11, MEM_TO_DEV, buf, &r[Data], len);
	else
		dma_start(4, 11, DEV_TO_MEM, &r[Data], buf,len);
	
	if(dma_wait(4) < 0) {
		printf("DMA ERROR while transferring data. \n");
		return;
	}

	// if(!write)
	// 	cachedinvse(buf, len);
	WR(Irpten, r[Irpten]|Datadone|Err);
	// tsleep(&emmc.r, datadone, 0, 3000);
	MicroDelay(3000);
	i = r[Interrupt]&~Cardintr;
	if((i & Datadone) == 0){
		printf("emmcio: Datadone error %d timeout intr %ux stat %ux\n",
			write, i, r[Status]);
		WR(Interrupt, i);
		// error(Eio);
		return;
	}
	if(i & Err){
		printf("emmcio: CMD err %d error intr %ux stat %ux\n",
			write, r[Interrupt], r[Status]);
		WR(Interrupt, i);
		// error(Eio);
		return;
	}
	if(i)
		WR(Interrupt, i);
	// poperror();
	// okay(0);
}

static void mmcinterrupt()
{	
	uint32_t *r;
	int i;

	r = (uint32_t*)EMMCREGS;
	i = r[Interrupt];
	// if(i&(Datadone|Err))
	// 	printf(" Data done or error intr \n");
	// if(i&Cardintr)
	// 	printf(" Card intr \n");
	WR(Irpten, r[Irpten] & ~i);
}
