int print_arib(int snl,unsigned char *snc) {
	int i,j,k;
	int g[4],gl,gr,ss,jc,jh,jl;
	unsigned long long ac;

	/* ARIB 8単位符号をシフトJISに変換して出力する */
	g[0]=0x0142;	/* G0=漢字(2バイトGセット) */
	g[1]=0x004a;	/* G1=英数(1バイトGセット) */
	g[2]=0x0030;	/* G2=平仮名(1バイトGセット) */
	g[3]=0x0031;	/* G3=片仮名(1バイトGセット) */
	gl=0;		/* GL=G0 */
	gr=2;		/* GR=G2 */
	ss=-1;		/* シングルシフト解除 */
	ac=0;
	k=0;
	while(k<snl) {
		jc=0;
		ac=(ac<<8)|snc[k];
		k++;
		if(ac==0x0e) {				/* LS1 (GL=G1) */
			gl=1;
			ac=0;
		}
		else if(ac==0x0f) {			/* LS0 (GL=G0) */
			gl=0;
			ac=0;
		}
		else if(ac==0x19) {			/* SS2 (GL=G2 シングルシフト) */
			ss=gl;
			gl=2;
			ac=0;
		}
		else if(ac==0x1d) {			/* SS3 (GL=G3 シングルシフト) */
			ss=gl;
			gl=3;
			ac=0;
		}
		else if(ac==0x1b6e) {			/* LS2 (GL=G2) */
			gl=2;
			ac=0;
		}
		else if(ac==0x1b6f) {			/* LS3 (GL=G3) */
			gl=3;
			ac=0;
		}
		else if(ac==0x1b7c) {			/* LS3R (GR=G3) */
			gr=3;
			ac=0;
		}
		else if(ac==0x1b7d) {			/* LS2R (GR=G2) */
			gr=2;
			ac=0;
		}
		else if(ac==0x1b7e) {			/* LS1R (GR=G1) */
			gr=1;
			ac=0;
		}
		else if(ac==0x20) {			/* SP (空白) */
			jc=0x2121;
			ac=0;
		}
		else if(ac==0x89) {			/* MSZ (中型サイズ) */
			ac=0;
		}
		else if(ac==0x8a) {			/* NSZ (標準サイズ) */
			ac=0;
		}
		else if(ac>=0x21 && ac<=0x7e) {		/* GL (1バイト文字) */
			if(g[gl]==0x004a) jc=0x2300+ac;		/* 英数 */
			else if(g[gl]==0x0030) jc=0x2400+ac;	/* 平仮名 */
			else if(g[gl]==0x0031) jc=0x2500+ac;	/* 片仮名 */
			if((g[gl]&0x0f00)==0x0000) ac=0;
		}
		else if(ac>=0x2100 && ac<=0x7eff) {	/* GL (2バイト文字) */
			if((ac&0xff)>=0x21 && (ac&0xff)<=0x7e && g[gl]==0x0142) jc=ac;
			ac=0;
		}
		else if(ac>=0xa1 && ac<=0xfe) {		/* GR (1バイト文字) */
			if(g[gr]==0x004a) jc=0x2300+(ac&0x7f);		/* 英数 */
			else if(g[gr]==0x0030) jc=0x2400+(ac&0x7f);	/* 平仮名 */
			else if(g[gr]==0x0031) jc=0x2500+(ac&0x7f);	/* 片仮名 */
			if((g[gr]&0x0f00)==0x0000) ac=0;
		}
		else if(ac>=0xa100 && ac<=0xfeff) {	/* GR (2バイト文字) */
			if((ac&0xff)>=0xa1 && (ac&0xff)<=0xfe && g[gr]==0x0142) jc=ac&0x7f7f;
			ac=0;
		}
		/* 文字表示 */
		if(jc>0) {
			/* 文字変換 */
			if(jc==0x2321) jc=0x212a;	/* エクスクラメンション */
			else if(jc==0x2328) jc=0x214a;	/* カッコ始め */
			else if(jc==0x2329) jc=0x214b;	/* カッコ閉じ */
			else if(jc==0x232d) jc=0x215d;	/* マイナス */
			else if(jc==0x232f) jc=0x213f;	/* スラッシュ */
			else if(jc==0x2479) jc=0x213c;	/* 長音記号(平仮名) */
			else if(jc==0x2579) jc=0x213c;	/* 長音記号(片仮名) */
			else if(jc==0x247e) jc=0x2126;	/* 中黒記号(平仮名) */
			else if(jc==0x257e) jc=0x2126;	/* 中黒記号(平仮名) */

			/* シフトJIS変換 */
			jh=(((jc/256)-0x21)/2)+0x81;
			if(jh>=0xa0) jh=jh+0x40;
			if(((jc/256)%2)==0) {
				jl=(jc%256)-0x21+0x9f;
			}
			else {
				jl=(jc%256)-0x21+0x40;
				if(jl>=0x7f) jl=jl+0x01;
				}
			printf("%c%c",jh,jl);

			/* シングルシフト解除 */
			if(ss>=0) {
				gl=ss;
				ss=-1;
			}
		}
		if(ac>0xff) ac=0;
	}
	return 0;
}

