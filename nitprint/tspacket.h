#define BUFMAX 2048			/* packet read buffer max size */

typedef struct {
	unsigned char buf[BUFMAX];	/* packet read buffer */
	int bs;				/* packet read buffer size */
	int bp;				/* packet read buffer pointer */
	long long ba;			/* packet read buffer offset address */

	unsigned char *pbuf;		/* packet buffer */

	int tei;			/* transport_error_indicator 1bit */
	int pusi;			/* payload_unit_start_indicator 1bit */
	int tp;				/* transport_priority 1bit */
	int pid;			/* PID 13bit */
	int tsc;			/* transport_scrambling_control 2bit */
	int afc;			/* adaptation_field_control 2bit */
	int cc;				/* continuity_counter 4bit */

	int afl;			/* adaptation_field_length 8bit */

	int di;				/* discontinuity_indicator 1bit */
	int rai;			/* random_access_indicator 1bit */
	int espi;			/* elementary_stream_priority_indicator 1bit */
	int pcrf;			/* PCR_flag 1bit */
	int opcrf;			/* OPCR_flag 1bit */
	int spf;			/* splicing_point_flag 1bit */
	int tpdf;			/* transport_private_data_flag 1bit */
	int afef;			/* adaptation_field_extension_flag 1bit */

	long long pcrb;			/* program_clock_reference_base 33bit */
	int pcre;			/* program_clock_reference_extension 9bit */

	unsigned char lbuf[188];	/* last packet buffer */
	int dc;				/* duplicate packet counter */

	int lcc[8192];			/* last continuity counter */
	int df;				/* packet drop flag */
} TSPACKET;

int init_packet(TSPACKET *tp) {
	int i;

	/* init read packet */
	tp->bs=0;
	tp->bp=0;
	tp->ba=0;

	/* init check duplicate packet */
	tp->dc=0;
	for(i=0;i<188;i++) tp->lbuf[i]=0x00;

	/* init check packet drop */
	for(i=0;i<8192;i++) tp->lcc[i]=-1;

	return 0;
}

int read_packet(TSPACKET *tp,FILE *fp) {
	int i;

	/* read packet */
	if(tp->bs>=188) {
		tp->bp=tp->bp+188;
		tp->bs=tp->bs-188;
		tp->ba=tp->ba+188;
	}
	while(tp->buf[tp->bp]!=0x47 || tp->bs<188) {
		while(tp->buf[tp->bp]!=0x47 && tp->bs>0) {
			tp->bp++;
			tp->bs--;
			tp->ba++;
		}
		if(tp->bs<188) {
			for(i=0;i<tp->bs;i++) tp->buf[i]=tp->buf[tp->bp+i];
			tp->bp=0;
			tp->bs=tp->bs+fread(tp->buf+tp->bs,1,BUFMAX-tp->bs,fp);
			if(tp->bs<188) break;
		}
	}
	tp->pbuf=tp->buf+tp->bp;

	/* get packet */
	tp->tei=(tp->pbuf[1]&0x80)>>7;			/* transport_error_indicator 1bit */
	tp->pusi=(tp->pbuf[1]&0x40)>>6;			/* payload_unit_start_indicator 1bit */
	tp->tp=(tp->pbuf[1]&0x20)>>5;			/* transport_priority 1bit */
	tp->pid=((tp->pbuf[1]<<8)|tp->pbuf[2])&0x1fff;	/* PID 13bit */
	tp->tsc=(tp->pbuf[3]&0xc0)>>6;			/* transport_scrambling_control 2bit */
	tp->afc=(tp->pbuf[3]&0x30)>>4;			/* adaptation_field_control 2bit */
	tp->cc=tp->pbuf[3]&0x0f;			/* continuity_counter 4bit */
	tp->afl=-1;
	if((tp->afc&0x02)!=0) tp->afl=tp->pbuf[4];	/* adaptation_field_length 8bit */
	tp->di=-1;
	tp->pcrf=-1;
	if(tp->afl>=1) {
		tp->di=(tp->pbuf[5]&0x80)>>7;		/* discontinuity_indicator 1bit */
		tp->rai=(tp->pbuf[5]&0x40)>>6;		/* random_access_indicator 1bit */
		tp->espi=(tp->pbuf[5]&0x20)>>5;		/* elementary_stream_priority_indicator 1bit */
		tp->pcrf=(tp->pbuf[5]&0x10)>>4;		/* PCR_flag 1bit */
		tp->opcrf=(tp->pbuf[5]&0x08)>>3;	/* OPCR_flag 1bit */
		tp->spf=(tp->pbuf[5]&0x04)>>2;		/* splicing_point_flag 1bit */
		tp->tpdf=(tp->pbuf[5]&0x02)>>1;		/* transport_private_data_flag 1bit */
		tp->afef=tp->pbuf[5]&0x01;		/* adaptation_field_extension_flag 1bit */
	}
	tp->pcrb=-1;
	tp->pcre=-1;
	if(tp->pcrf==1 && tp->afl>=7) {
		tp->pcrb=(((long long)tp->pbuf[6]<<32|(long long)tp->pbuf[7]<<24|tp->pbuf[8]<<16|tp->pbuf[9]<<8|tp->pbuf[10])&0xffffff80)>>7;
								/* program_clock_reference_base 33bit */
		tp->pcre=(tp->pbuf[10]<<8|tp->pbuf[11])&0x1ff;	/* program_clock_reference_extension 9bit */
	}

	/* check duplicate packet */
	if(tp->pcrf==1 && tp->afl>=7) for(i=0;i<6;i++) tp->lbuf[6+i]=tp->pbuf[6+i];
	for(i=0;i<188;i++) if(tp->pbuf[i]!=tp->lbuf[i]) break;
	if(i<188) tp->dc=0;
	else tp->dc++;
	if(tp->dc>1) tp->dc=0;
	if((tp->afc&0x01)==0 || tp->pid==0x1fff) tp->dc=0;
	for(i=0;i<188;i++) tp->lbuf[i]=tp->pbuf[i];

	/* check packet drop */
	tp->df=0;
	if(tp->pid!=0x1fff && tp->lcc[tp->pid]>=0 && tp->di!=1) {
		if((tp->afc&0x01)!=0 && tp->dc==0) {
			if(tp->cc!=((tp->lcc[tp->pid]+1)&0x0f)) tp->df=1;
		}
		else {
			if(tp->cc!=tp->lcc[tp->pid]) tp->df=1;
		}
	}
	tp->lcc[tp->pid]=tp->cc;

	return tp->bs;
}

