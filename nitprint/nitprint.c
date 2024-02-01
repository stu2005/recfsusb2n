#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include "tspacket.h"
#include "tssection.h"
#include "tsarib.h"

int main(int argc,char *argv[]) {
	FILE *fp;
	TSPACKET tp;
	TSSECTION nit;
	TSSECTION sdt;
	int nid,ndl,tsll,tsid[48],onid[48],tdl;
	int dt,dl,f[48],op,wef,p,m,sr,fi;
	int sid,eudf,esf,epff,rs,fcm,dll;
	int st,spnl,snl;
	int bc,bi,ln,sy,ch,rn;
	int rm,ef,n,i,j,k;
	int nf,sf,cf[48];

	if(argc<2) {
		printf("usage: %s <file> [mode]\n",argv[0]);
		printf("  mode: default is 1\n");
		printf("    1: NIT only output\n");
		printf("    2: NIT only numeric output\n");
		printf("    3: SDT only output\n");
		printf("    4: NIT & SDT output\n");
		printf("    5: NIT & SDT numeric output\n");
		return 1;
	}
	fp=fopen(argv[1],"rb");
	if(fp==NULL) {
		printf("file open error.\n");
		return 1;
	}
	rm=1;
	if(argc>2) sscanf(argv[2],"%d",&rm);
	if(rm<1 || rm>5) {
		printf("mode error.\n");
		return 1;
	}
	init_packet(&tp);
	init_section(&nit);
	init_section(&sdt);
	for(i=0;i<48;i++) cf[i]=-1;
	nf=0;
	sf=0;
	while(1) {
		read_packet(&tp,fp);
		if(tp.bs<188) break;
		while(get_section(0x0010,&nit,&tp)==1 && nf==0) {				/* 0x0010 NIT */
			if(nit.tid==0x40 && (nit.sl-5-4)>=4 && nit.cni==1 && nit.sn==0) {	/* 5 (nid..lsn) / 4 (crc) / 4 (ndl,tsll) */
				nid=nit.tie;							/* network_id 16bit */
				ndl=((nit.buf[8]<<8)|nit.buf[9])&0x0fff;			/* network_descriptors_length 12bit */
				tsll=((nit.buf[10+ndl]<<8)|nit.buf[11+ndl])&0x0fff;		/* transport_stream_loop_length 12bit */
				i=0;
				while(tsll>=(i+6)) {						/* 6 (tsid,onid,tdl) */
					tsid[nf]=(nit.buf[12+ndl+i]<<8)|nit.buf[13+ndl+i];	/* transport_stream_id 16bit */
					onid[nf]=(nit.buf[14+ndl+i]<<8)|nit.buf[15+ndl+i]; 	/* original_network_id 16bit */
					tdl=((nit.buf[16+ndl+i]<<8)|nit.buf[17+ndl+i])&0x0fff;	/* transport_descriptors_length 12bit */
					j=0;
					while(tdl>=(j+2)) {					/* 2 (dt,dl) */
						dt=nit.buf[18+ndl+i+j];				/* descriptor_tag 8bit */
						dl=nit.buf[19+ndl+i+j];				/* descriptor_length 8bit */
						if(dt==0x43 && dl>=9) {				/* 0x43 Satellite delivery system descriptor */
							f[nf]=((nit.buf[20+ndl+i+j]&0xf0)>>4)*10+(nit.buf[20+ndl+i+j]&0x0f);
							f[nf]=f[nf]*100+((nit.buf[21+ndl+i+j]&0xf0)>>4)*10+(nit.buf[21+ndl+i+j]&0x0f);
							f[nf]=f[nf]*100+((nit.buf[22+ndl+i+j]&0xf0)>>4)*10+(nit.buf[22+ndl+i+j]&0x0f);
							f[nf]=f[nf]*100+((nit.buf[23+ndl+i+j]&0xf0)>>4)*10+(nit.buf[23+ndl+i+j]&0x0f);
												/* frequency 32bit */
							op=((nit.buf[24+ndl+i+j]&0xf0)>>4)*10+(nit.buf[24+ndl+i+j]&0x0f);
							op=op*100+((nit.buf[25+ndl+i+j]&0xf0)>>4)*10+(nit.buf[25+ndl+i+j]&0x0f);
												/* orbital_position 16bit */
							wef=(nit.buf[26+ndl+i+j]&0x80)>>7;	/* west_east_flag 1bit */
							p=(nit.buf[26+ndl+i+j]&0x60)>>5;	/* polarisation 2bit */
							m=nit.buf[26+ndl+i+j]&0x1f;		/* modulation 5bit */
							sr=((nit.buf[27+ndl+i+j]&0xf0)>>4)*10+(nit.buf[27+ndl+i+j]&0x0f);
							sr=sr*100+((nit.buf[28+ndl+i+j]&0xf0)>>4)*10+(nit.buf[28+ndl+i+j]&0x0f);
							sr=sr*100+((nit.buf[29+ndl+i+j]&0xf0)>>4)*10+(nit.buf[29+ndl+i+j]&0x0f);
							sr=sr*10+((nit.buf[30+ndl+i+j]&0xf0)>>4);
												/* symbol_rate 28bit */
							fi=nit.buf[30+ndl+i+j]&0x0f;		/* FEC_inner 4bit */
							if(rm==1 || rm==4) {
								printf("nid=%5d, ",nid);
								printf("tsid=%5d, onid=%5d, ",tsid[nf],onid[nf]);
								printf("f=%8d, op=%4d, wef=%1d, p=%1d, m=%2d, sr=%7d, fi=%2d\n",f[nf],op,wef,p,m,sr,fi);
							}
							if(rm==2) {
								ln=(tsid[nf]&0xf000)>>12;	/* TSID to NID under 4bit */
								sy=(tsid[nf]&0x0e00)>>9;	/* TSID to station open year */
								ch=(tsid[nf]&0x01f0)>>4;	/* TSID to channel */
								rn=tsid[nf]&0x0007;		/* TSID to relative TS number */
								bc=((f[nf]-1172748)/3836)*2+1;	/* frequency to BS ch */
								bi=f[nf]-1067800;		/* frequency to BS-IF frequency */
								printf("%02d\t%d\t%d\t%d\t%d\t0x%04X\n",ch,rn,f[nf]*10,bi*10,tsid[nf],tsid[nf]);
							}
						}
						j=j+2+dl;
					}
					i=i+6+tdl;
					if(nf<48) nf++;
					else printf("buffer over flow error.\n");
				}
			}
		}
		if((rm==1 || rm==2) && nf>0) break;
		while(get_section(0x0011,&sdt,&tp)==1 && nf>0 && sf>=0) {	/* 0x0011 SDT */
			sf=nf;
			if((sdt.tid==0x42 || sdt.tid==0x46) && (sdt.sl-5-4)>=4 && sdt.cni==1 && sdt.sn==0) {
				for(sf=0;sf<nf;sf++) {
					if(onid[sf]==((sdt.buf[8]<<8)|sdt.buf[9]) && tsid[sf]==sdt.tie && cf[sf]<0) break;
				}
			}
			i=0;
			while(i<(sdt.sl-5-3-4) && sf<nf) {
				cf[sf]=1;
				sid=(sdt.buf[11+i]<<8)|sdt.buf[12+i];		/* service_id 16bit */
				eudf=(sdt.buf[13+i]&0x1c)>>2;			/* EIT_user_defined_flags 3bit */
				esf=(sdt.buf[13+i]&0x02)>>1;			/* EIT_schedule_flag 1bit */
				epff=sdt.buf[13+i]&0x01;			/* EIT_present_following_flag 1bit */
				rs=(sdt.buf[14+i]&0xe0)>>5;			/* running_status 3bit */
				fcm=(sdt.buf[14+i]&0x10)>>4;			/* free_CA_mode 1bit */
				dll=((sdt.buf[14+i]<<8)|sdt.buf[15+i])&0x0fff;	/* descriptors_loop_length 12bit */
				j=0;
				while((j+1)<dll) {
					dt=sdt.buf[16+i+j];		/* descriptor_tag 8bit */
					dl=sdt.buf[17+i+j];		/* descriptor_length 8bit */
					if(dt==0x48 && dl>=3) {
						st=sdt.buf[18+i+j];
						spnl=sdt.buf[19+i+j];
						snl=sdt.buf[20+i+j+spnl];
						if(rm==3 || rm==4) {
							printf("tsid=%5d, onid=%5d, sid=%5d",tsid[sf],onid[sf],sid);
							printf(", eudf=%1d, esf=%1d, epff=%1d, rs=%1d, fcm=%1d",eudf,esf,epff,rs,fcm);
							printf(", st=0x%02X, spn=",st);
							print_arib(spnl,sdt.buf+20+i+j);
							printf(", sn=");
							print_arib(snl,sdt.buf+21+i+j+spnl);
							printf("\n");
						}
						if(rm==5 && st==1) {
							ln=(tsid[sf]&0xf000)>>12;	/* TSID to NID under 4bit */
							sy=(tsid[sf]&0x0e00)>>9;	/* TSID to station open year */
							ch=(tsid[sf]&0x01f0)>>4;	/* TSID to channel */
							rn=tsid[sf]&0x0007;		/* TSID to relative TS number */
							bc=((f[sf]-1172748)/3836)*2+1;	/* frequency to BS ch */
							bi=f[sf]-1067800;		/* frequency to BS-IF frequency */
							printf("%02d\t%d\t%d\t%d\t%d\t0x%04X\t%d\t",ch,rn,f[sf]*10,bi*10,tsid[sf],tsid[sf],sid);
							print_arib(snl,sdt.buf+21+i+j+spnl);
							printf("\n");
						}
					}
					j=j+2+dl;
				}
				i=i+5+dll;
			}
		}
		for(i=0;i<nf;i++) if(cf[i]<0) break;
		if(i>=nf && nf>0) break;
	}
	fclose(fp);
	return 0;
}

