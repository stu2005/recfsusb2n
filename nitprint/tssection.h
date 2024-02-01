#define SBUFMAX 4096

typedef struct {
	unsigned char buf[SBUFMAX];	/* section buffer */
	int sp;				/* section pointer */

	int tid;			/* table_id 8bit */
	int ssi;			/* section_syntax_indicator 1bit */
	int sl;				/* section_length 12bit */
	int tie;			/* table_id_extension 16bit */
	int vn;				/* version_number 5bit */
	int cni;			/* current_next_indicator 1bit */
	int sn;				/* section_number 8bit */
	int lsn;			/* last_section_number 8bit */
} TSSECTION;

int init_section(TSSECTION *ts) {
	ts->sp=-1;
	return 0;
}

int get_section(int spid,TSSECTION *ts,TSPACKET *tp) {
	int sf,pp,ps,pf,i;

	if(ts->sp==-2) {			/* packet end */
		ts->sp=-1;
		return 0;
	}
	if(tp->pid!=spid) return 0;		/* not match pid */
	if((tp->afc&0x01)==0) return 0;		/* no payload */
	if(tp->pusi==0 && ts->sp<0) return 0;	/* not start packet & not start section */
	if(tp->dc!=0) return 0;			/* duplicate packet */
	if(tp->tei!=0) {			/* error packet */
		ts->sp=-1;
		return 0;
	}
	if(tp->tsc!=0) {			/* scramble packet */
		ts->sp=-1;
		return 0;
	}
	if(tp->df!=0) ts->sp=-1;		/* packet drop */

	/* init payload pointer & size */
	sf=0;					/* section end flag */
	pp=4;					/* payload pointer */
	ps=184;					/* payload size */
	if(tp->afl>=0) {
		pp=4+1+tp->afl;
		ps=184-1-tp->afl;
	}
	if(tp->pusi==1) {
		pf=tp->pbuf[pp];
		pp++;
		ps--;
		if(ts->sp<0) {
			ts->sp=0;
			pp=pp+pf;
			ps=ps-pf;
		}
		else {
			ps=pf;
			sf=1;
		}
	}

	/* payload to section buffer */
	for(i=0;i<ps;i++) {
		if((ts->sp+i)>=SBUFMAX) break;
		if((pp+i)>=188) break;
		ts->buf[ts->sp+i]=tp->pbuf[pp+i];
	}
	ts->sp=ts->sp+ps;

	/* get table header */
	ts->tid=-1;
	ts->ssi=-1;
	ts->sl=-1;
	if(ts->sp>=3) {
		ts->tid=ts->buf[0];				/* table_id 8bit */
		ts->ssi=(ts->buf[1]&0x80)>>7;			/* section_syntax_indicator 1bit */
		ts->sl=((ts->buf[1]<<8)|ts->buf[2])&0x0fff;	/* section_length 12bit */
	}
	ts->tie=-1;
	ts->vn=-1;
	ts->cni=-1;
	ts->sn=-1;
	ts->lsn=-1;
	if(ts->sp>=8 && ts->ssi==1) {
		ts->tie=(ts->buf[3]<<8)|ts->buf[4];	/* table_id_extension 16bit */
		ts->vn=(ts->buf[5]&0x3e)>>1;		/* version_number 5bit */
		ts->cni=ts->buf[5]&0x01;		/* current_next_indicator 1bit */
		ts->sn=ts->buf[6];			/* section_number 8bit */
		ts->lsn=ts->buf[7];			/* last_section_number 8bit */
	}

	/* check section length */
	if(ts->sl>=0 && ts->sp>=(ts->sl+3)) sf=1;

	/* check section end */
	if(sf==1) ts->sp=-1;

	/* check packet end */
	if(sf==1 && (pp+ps)>=188) ts->sp=-2;

	return sf;
}

