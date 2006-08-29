/
/ c code tables-- compile to register
/

.globl	_regtab

.data
_regtab=.
	106.;	cr106
	30.;	cr70
	31.;	cr70
	32.;	cr32
	33.;	cr32
	37.;	cr37
	38.;	cr37
	98.;	cr100
	99.;	cr100
	80.;	cr80
	40.;	cr40
	41.;	cr40	/ - like +
	42.;	cr42
	43.;	cr43
	44.;	cr43
	45.;	cr45
	46.;	cr40
	55.; 	cr40
	48.;	cr40
	49.;	cr49
	70.;	cr70
	71.;	cr70
	72.;	cr72
	73.;	cr73
	74.;	cr74
	75.;	cr75
	76.;	cr72
	78.;	cr70
	85.;	cr70
	79.;	cr79
	102.;	cr102
	51.;	cr51
	52.;	cr52
	56.;	cr56
	57.;	cr57
	58.;	cr58
	59.;	cr59
	91.;	cr91
	82.;	cr82
	83.;	cr82
	84.;	cr82
	86.;	cr86
	87.;	cr86
	88.;	cr86
	0
.text

/ goto
cr102=.+2; 0
.byte 20,0;.byte 77,0
L1
.data
L1:<jmp>;.byte 301;<\n>
<\0>
.text
.byte 177,0;.byte 77,0
L2
.data
L2:<GB>
<jmp>;.byte 243;<(I)\n>
<\0>
.text
/ call
cr100=.+2; 0
.byte 20,0;.byte 77,0
L3
.data
L3:<jsr>;.byte 360;<c,MA\n>
<\0>
.text
.byte 177,0;.byte 77,0
L4
.data
L4:<GB>
<jsr>;.byte 360;<c,#(I)\n>
<\0>
.text
.byte 77,0;.byte 77,0
L5
.data
L5:<GA>
<jsr>;.byte 360;<c,(I)\n>
<\0>
.text
/ addressible
cr106=.+2; 0
.byte 4,0;.byte 77,0
L6
.data
L6:<clr>;.byte 311;<\n>
<\0>
.text
.byte 4,4;.byte 77,0
L7
.data
L7:<clrf>;.byte 311;<\n>
<\0>
.text
.byte 20,0;.byte 77,0
L8
.text;.byte 20,5;.byte 77,0
L8
.data
L8:<movC>;.byte 301;<,I\n>
<\0>
.text
.byte 20,4;.byte 77,0
L9
.data
L9:<movof>;.byte 301;<,I\n>
<\0>
.text
.byte 177,0;.byte 77,0
L10
.text;.byte 177,5;.byte 77,0
L10
.data
L10:<GB>
<movC>;.byte 243;<(I),I\n>
<\0>
.text
.byte 177,4;.byte 77,0
L11
.data
L11:<GB>
<movof>;.byte 243;<(I),I\n>
<\0>
.text
.byte 20,10;.byte 77,0
L12
.data
L12:<mov>;.byte 301;<+,I+\n>
<mov>;.byte 301;<,I\n>
<\0>
.text
.byte 177,10;.byte 77,0
L13
.data
L13:<GB>
<mov>;.byte 243;<+2(I),I+\n>
<mov>;.byte 243;<(I),I\n>
<\0>
.text
/ ++,-- postfix
cr32=.+2; 0
.byte 20,0;.byte 5,0
L14
.data
L14:<movC>;.byte 301;<',I\n>
<M'C>;.byte 301;<\n>
<\0>
.text
.byte 20,1;.byte 77,0
L15
.data
L15:<mov>;.byte 301;<',I\n>
<M>;.byte 302;<,A\n>
<\0>
.text
.byte 124,0;.byte 5,0
L16
.data
L16:<GJ>
<movC>;.byte 243;<(J),I\n>
<M'C>;.byte 243;<(J)\n>
<\0>
.text
.byte 177,0;.byte 5,0
L17
.data
L17:<GB>
<movC>;.byte 243;<(I),-(sp)\n>
<M'C>;.byte 243;<(I)\n>
<movC>;.byte 250;<sp)+,I\n>
<\0>
.text
.byte 124,1;.byte 77,0
L18
.data
L18:<GJ>
<mov>;.byte 243;<(J),I\n>
<M>;.byte 302;<,#(J)\n>
<\0>
.text
.byte 177,1;.byte 77,0
L19
.data
L19:<GB>
<mov>;.byte 243;<(I),-(sp)\n>
<M>;.byte 302;<,#(I)\n>
<mov>;.byte 250;<sp)+,I\n>
<\0>
.text
.byte 20,10;.byte 5,0
L20
.data
L20:<GA>
<M>;.byte 244;<1,A+\n>
<V>;.byte 301;<\n>
<\0>
.text
.byte 124,10;.byte 5,0
L21
.data
L21:<GJ>
<mov>;.byte 243;<+2(J),I+\n>
<mov>;.byte 243;<(J),I\n>
<M>;.byte 244;<1,#+2(J)\n>
<V>;.byte 243;<(J)\n>
<\0>
.text
.byte 177,10;.byte 5,0
L22
.data
L22:<GB>
<mov>;.byte 243;<+2(I),-(sp)\n>
<mov>;.byte 243;<(I),-(sp)\n>
<add>;.byte 244;<1,#+2(I)\n>
<V>;.byte 243;<(I)\n>
<mov>;.byte 250;<sp)+,I\n>
<mov>;.byte 250;<sp)+,I+\n>
<\0>
.text
/ - unary, ~
cr37=.+2; 0
.byte 77,0;.byte 77,0
L23
.text;.byte 77,4;.byte 77,0
L23
.data
L23:<GA>
<MP>;.byte 311;<\n>
<\0>
.text
.byte 77,10;.byte 77,0
L24
.data
L24:<GA>
<M>;.byte 311;<\n>
<M>;.byte 311;<+\n>
<V>;.byte 311;<\n>
<\0>
.text
/ =
cr80=.+2; 0
.byte 20,0;.byte 77,0
L25
.text;.byte 20,5;.byte 77,4
L25
.data
L25:<KA>
<movC>;.byte 311;<,A\n>
<\0>
.text
.byte 20,4;.byte 77,4
L26
.data
L26:<KA>
<movfo>;.byte 311;<,A\n>
<\0>
.text
.byte 177,5;.byte 20,4
L27
.data
L27:<GB>
<KA>
<movf>;.byte 311;<,#(I)\n>
<\0>
.text
.byte 177,0;.byte 20,0
L28
.data
L28:<GB>
<movC>;.byte 302;<,#(I)\n>
<movC>;.byte 243;<(I),I\n>
<\0>
.text
.byte 177,4;.byte 20,4
L29
.data
L29:<GB>
<KA>
<movfo>;.byte 311;<,#(I)\n>
<\0>
.text
.byte 177,0;.byte 24,0
L30
.data
L30:<GB>
<KI>
<movC>;.byte 312;<,#(I)\n>
<movC>;.byte 312;<,I\n>
<\0>
.text
.byte 124,5;.byte 77,4
L31
.data
L31:<KA>
<GJ>
<movf>;.byte 311;<,#(J)\n>
<\0>
.text
.byte 124,4;.byte 77,4
L32
.data
L32:<KA>
<GJ>
<movfo>;.byte 311;<,#(J)\n>
<\0>
.text
.byte 177,0;.byte 77,0
L33
.text;.byte 177,5;.byte 77,4
L33
.data
L33:<GD>
<KA>
<movC>;.byte 311;<,*(sp)+\n>
<\0>
.text
.byte 177,4;.byte 77,4
L34
.data
L34:<GD>
<KA>
<movfo>;.byte 311;<,*(sp)+\n>
<\0>
.text
/ +, -, |, &~, <<
cr40=.+2; 0
.byte 77,0;.byte 4,0
L35
.data
L35:<GA>
<\0>
.text
.byte 77,0;.byte 5,0
L36
.data
L36:<GA>
<M'>;.byte 311;<\n>
<\0>
.text
.data;add1:;.text;.byte 77,0;.byte 20,1
L37
.text;.byte 77,4;.byte 20,5
L37
.data
L37:<GA>
<MD>;.byte 302;<,I\n>
<\0>
.text
.data;add2:;.text;.byte 77,0;.byte 124,1
L38
.text;.byte 77,4;.byte 124,5
L38
.data
L38:<GA>
<KJ>
<MD>;.byte 242;<(J),I\n>
<\0>
.text
.data;add3:;.text;.byte 77,0;.byte 24,0
L39
.text;.byte 77,4;.byte 24,4
L39
.data
L39:<GA>
<KI>
<MP>;.byte 312;<,I\n>
<\0>
.text
.data;add4:;.text;.byte 77,0;.byte 177,1
L40
.text;.byte 77,4;.byte 177,5
L40
.data
L40:<KD>
<GA>
<MD>;.byte 252;<(sp)+,I\n>
<\0>
.text
.data;add5:;.text;.byte 77,0;.byte 77,0
L41
.text;.byte 77,4;.byte 77,4
L41
.data
L41:<KC>
<GA>
<MP>;.byte 250;<sp)+,I\n>
<\0>
.text
.byte 77,10;.byte 10,0
L42
.data
L42:<GA>
<M>;.byte 302;<,I+\n>
<V>;.byte 311;<\n>
<\0>
.text
.byte 77,10;.byte 20,10
L43
.data
L43:<GA>
<M>;.byte 302;<,I\n>
<M>;.byte 302;<+,I+\n>
<V>;.byte 311;<\n>
<\0>
.text
.byte 77,10;.byte 24,10
L44
.data
L44:<GA>
<KI>
<M>;.byte 312;<+,I+\n>
<V>;.byte 311;<\n>
<M>;.byte 312;<,I\n>
<\0>
.text
.byte 77,10;.byte 77,10
L45
.data
L45:<KC>
<GA>
<M>;.byte 250;<sp)+,I\n>
<M>;.byte 250;<sp)+,I+\n>
<V>;.byte 311;<\n>
<\0>
.text
/ ^ -- xor
cr49=.+2; 0
.byte 77,0;.byte 24,0
L46
.text;L46=add3

.byte 77,0;.byte 77,0
L47
.data
L47:<GC>
<KA>
<xor>;.byte 311;<,(sp)\n>
<mov>;.byte 250;<sp)+,I\n>
<\0>
.text
/ >> (all complicated cases taken care of by << -)
cr45=.+2; 0
.byte 77,0;.byte 5,0
L48
.data
L48:<GA>
<asr>;.byte 311;<\n>
<\0>
.text
/ * -- I must be odd on integers
cr42=.+2; 0
.byte 77,0;.byte 20,1
L49
.text;.byte 77,4;.byte 20,5
L49
.text;L49=add1

.byte 77,0;.byte 124,1
L50
.text;.byte 77,4;.byte 124,5
L50
.text;L50=add2

.byte 77,0;.byte 24,0
L51
.text;.byte 77,4;.byte 24,4
L51
.text;L51=add3

.byte 77,0;.byte 77,0
L52
.text;.byte 77,4;.byte 77,4
L52
.text;L52=add5

/ / I must be odd on integers
cr43=.+2; 0
.byte 77,0;.byte 20,1
L53
.data
L53:<GA>
<T>
<sxt>;.byte 311;<-\n>
<div>;.byte 302;<,I-\n>
<\0>
.text
.byte 77,0;.byte 124,1
L54
.data
L54:<GA>
<T>
<sxt>;.byte 311;<-\n>
<KJ>
<div>;.byte 242;<(J),I-\n>
<\0>
.text
.byte 77,0;.byte 24,0
L55
.data
L55:<GA>
<T>
<sxt>;.byte 311;<-\n>
<KI>
<div>;.byte 312;<,I-\n>
<\0>
.text
.byte 77,0;.byte 77,0
L56
.data
L56:<KC>
<GA>
<T>
<sxt>;.byte 311;<-\n>
<div>;.byte 250;<sp)+,I-\n>
<\0>
.text
.byte 77,4;.byte 20,5
L57
.text;L57=add1

.byte 77,4;.byte 124,5
L58
.text;L58=add2

.byte 77,4;.byte 24,4
L59
.text;L59=add3

.byte 77,4;.byte 77,4
L60
.text;L60=add5

/ =+, =-, =|, =&~
cr70=.+2; 0
.data;addq1:;.text;.byte 20,1;.byte 20,1
L61
.data
L61:<M>;.byte 302;<,A'\n>
<mov>;.byte 301;<,I\n>
<\0>
.text
.data;addq1a:;.text;.byte 20,0;.byte 20,1
L62
.text;.byte 20,5;.byte 20,5
L62
.data
L62:<movC>;.byte 301;<',I\n>
<MP>;.byte 302;<,I\n>
<movC>;.byte 311;<,A\n>
<\0>
.text
.data;addq2:;.text;.byte 20,1;.byte 177,1
L63
.data
L63:<KB>
<M>;.byte 242;<(I),A'\n>
<mov>;.byte 301;<,I\n>
<\0>
.text
.data;addq3:;.text;.byte 20,1;.byte 77,0
L64
.data
L64:<KA>
<M>;.byte 311;<,A'\n>
<mov>;.byte 301;<,I\n>
<\0>
.text
.data;addq4:;.text;.byte 124,1;.byte 177,1
L65
.data
L65:<KB>
<GJ>
<M>;.byte 242;<(I),#(J)\n>
<mov>;.byte 243;<(J),I\n>
<\0>
.text
.data;addq4a:;.text;.byte 20,5;.byte 24,4
L66
.data
L66:<movf>;.byte 301;<',I\n>
<KI>
<MP>;.byte 312;<,I\n>
<movf>;.byte 311;<,A\n>
<\0>
.text
.data;addq5:;.text;.byte 20,0;.byte 77,0
L67
.text;.byte 20,5;.byte 77,4
L67
.data
L67:<KC>
<movC>;.byte 301;<',I\n>
<MP>;.byte 250;<sp)+,I\n>
<movC>;.byte 311;<,A\n>
<\0>
.text
.data;addq6:;.text;.byte 20,4;.byte 77,4
L68
.data
L68:<KC>
<movof>;.byte 301;<',I\n>
<MP>;.byte 250;<sp)+,I\n>
<movfo>;.byte 311;<,A\n>
<\0>
.text
.data;addq7:;.text;.byte 124,1;.byte 77,0
L69
.data
L69:<KA>
<GJ>
<M>;.byte 311;<,#(J)\n>
<mov>;.byte 243;<(J),I\n>
<\0>
.text
.data;addq8:;.text;.byte 177,1;.byte 77,0
L70
.data
L70:<KC>
<GB>
<M>;.byte 250;<sp)+,#(I)\n>
<mov>;.byte 243;<(I),I\n>
<\0>
.text
.data;addq9:;.text;.byte 177,0;.byte 77,0
L71
.data
L71:<GD>
<KC>
<movC>;.byte 252;<2(sp),I\n>
<MP>;.byte 250;<sp)+,I\n>
<movC>;.byte 311;<,*(sp)+\n>
<\0>
.text
.data;addq9a:;.text;.byte 177,5;.byte 77,4
L72
.data
L72:<KC>
<GB>
<movC>;.byte 243;<(I),I\n>
<MP>;.byte 250;<sp)+,I\n>
<movC>;.byte 311;<,#(I)\n>
<\0>
.text
.data;addq10:;.text;.byte 177,4;.byte 77,4
L73
.data
L73:<KC>
<GB>
<movof>;.byte 243;<(I),J\n>
<MP>;.byte 250;<sp)+,J\n>
<movfo>;.byte 312;<,#(I)\n>
<movf>;.byte 312;<,I\n>
<\0>
.text
/ =*, =<< (for integer multiply, I must be odd)
cr72=.+2; 0
.byte 20,0;.byte 20,1
L74
.text;.byte 20,5;.byte 20,5
L74
.text;L74=addq1a

.byte 20,4;.byte 77,4
L75
.text;L75=addq6

.byte 20,5;.byte 24,4
L76
.text;L76=addq4a

.byte 20,0;.byte 77,0
L77
.text;.byte 20,5;.byte 77,4
L77
.text;L77=addq5

.byte 177,0;.byte 77,0
L78
.text;L78=addq9

.byte 177,5;.byte 77,4
L79
.text;L79=addq9a

.byte 177,4;.byte 77,4
L80
.text;L80=addq10

/ =/ ;  I must be odd on integers
cr73=.+2; 0
.byte 20,0;.byte 20,1
L81
.data
L81:<movC>;.byte 301;<',I\n>
<sxt>;.byte 311;<-\n>
<divP>;.byte 302;<,I-\n>
<movC>;.byte 311;<-,A\n>
<\0>
.text
.byte 20,0;.byte 77,0
L82
.data
L82:<KC>
<movC>;.byte 301;<',I\n>
<sxt>;.byte 311;<-\n>
<div>;.byte 250;<sp)+,I-\n>
<movC>;.byte 311;<-,A\n>
<\0>
.text
.byte 124,0;.byte 77,0
L83
.data
L83:<KC>
<GJ>
<movC>;.byte 243;<(J),I\n>
<sxt>;.byte 311;<-\n>
<div>;.byte 250;<sp)+,I-\n>
<movC>;.byte 311;<-,#(J)\n>
<\0>
.text
.byte 177,0;.byte 77,0
L84
.data
L84:<GD>
<KC>
<movC>;.byte 252;<2(sp),I\n>
<sxt>;.byte 311;<-\n>
<div>;.byte 250;<sp)+,I-\n>
<movC>;.byte 311;<-,*(sp)+\n>
<\0>
.text
.byte 20,5;.byte 20,5
L85
.text;L85=addq1a

.byte 20,5;.byte 24,4
L86
.text;L86=addq4a

.byte 20,5;.byte 77,4
L87
.text;L87=addq5

.byte 20,4;.byte 77,4
L88
.text;L88=addq6

.byte 177,5;.byte 77,4
L89
.text;L89=addq9a

.byte 177,4;.byte 77,4
L90
.text;L90=addq10

/ =mod; I must be odd on integers
cr74=.+2; 0
.byte 20,0;.byte 20,1
L91
.data
L91:<movC>;.byte 301;<',I\n>
<sxt>;.byte 311;<-\n>
<div>;.byte 302;<,I-\n>
<movC>;.byte 311;<,A\n>
<\0>
.text
.byte 20,0;.byte 77,0
L92
.data
L92:<KC>
<movC>;.byte 301;<',I\n>
<sxt>;.byte 311;<-\n>
<div>;.byte 250;<sp)+,I-\n>
<movC>;.byte 311;<,A\n>
<\0>
.text
.byte 124,0;.byte 77,0
L93
.data
L93:<KC>
<GJ>
<movC>;.byte 243;<(J),I\n>
<sxt>;.byte 311;<-\n>
<div>;.byte 250;<sp)+,I-\n>
<movC>;.byte 311;<,#(J)\n>
<\0>
.text
.byte 177,0;.byte 77,0
L94
.data
L94:<GD>
<KC>
<movC>;.byte 252;<2(sp),I\n>
<sxt>;.byte 311;<-\n>
<div>;.byte 250;<sp)+,I-\n>
<mov>;.byte 311;<,*(sp)+\n>
<\0>
.text
/ =^ -- =xor
cr79=.+2; 0
.byte 20,1;.byte 77,0
L95
.text;L95=addq3

.byte 20,3;.byte 77,0
L96
.data
L96:<KC>
<movb>;.byte 301;<',I\n>
<xor>;.byte 311;<,(sp)\n>
<mov>;.byte 250;<sp)+,I\n>
<movb>;.byte 311;<,A\n>
<\0>
.text
.byte 177,0;.byte 77,0
L97
.data
L97:<GD>
<movC>;.byte 252;<(sp),-(sp)\n>
<KA>
<xor>;.byte 311;<,(sp)\n>
<movC>;.byte 250;<sp)+,I\n>
<movC>;.byte 311;<,*(sp)+\n>
<\0>
.text
/ =>> (all complicated cases done by =<< -)
cr75=.+2; 0
.byte 20,0;.byte 5,0
L98
.data
L98:<asrC>;.byte 301;<'\n>
<movC>;.byte 301;<,I\n>
<\0>
.text
.byte 177,0;.byte 5,0
L99
.data
L99:<GB>
<asrC>;.byte 243;<(I)\n>
<movC>;.byte 243;<(I),I\n>
<\0>
.text
/ << for longs
cr91=.+2; 0
.byte 77,10;.byte 20,1
L100
.text;L100=add1

.byte 77,10;.byte 124,1
L101
.text;L101=add2

.byte 77,10;.byte 24,0
L102
.text;L102=add3

.byte 77,10;.byte 177,1
L103
.text;L103=add4

.byte 77,10;.byte 77,0
L104
.text;L104=add5

/ int -> float
cr51=.+2; 0
.byte 20,1;.byte 77,0
L105
.data
L105:<movif>;.byte 301;<,I\n>
<\0>
.text
.byte 177,1;.byte 77,0
L106
.data
L106:<GB>
<movif>;.byte 243;<(I),I\n>
<\0>
.text
.byte 77,0;.byte 77,0
L107
.data
L107:<GA>
<movif>;.byte 311;<,I\n>
<\0>
.text
/ float, double -> int
cr52=.+2; 0
.byte 77,4;.byte 77,0
L108
.data
L108:<GA>
<movfi>;.byte 311;<,I\n>
<\0>
.text
/ double (float) to long
cr56=.+2; 0
.byte 77,4;.byte 77,0
L109
.data
L109:<GA>
<setl\n>
<movfi>;.byte 311;<,-(sp)\n>
<mov>;.byte 250;<sp)+,I\n>
<mov>;.byte 250;<sp)+,I+\n>
<seti\n>
<\0>
.text
/ long to double
cr57=.+2; 0
.byte 20,10;.byte 77,0
L110
.data
L110:<setl\n>
<movif>;.byte 301;<,I\n>
<seti\n>
<\0>
.text
.byte 177,10;.byte 77,0
L111
.data
L111:<GB>
<setl\n>
<movif>;.byte 243;<(I),I\n>
<seti\n>
<\0>
.text
.byte 77,10;.byte 77,0
L112
.data
L112:<GC>
<setl\n>
<movif>;.byte 250;<sp)+,I\n>
<seti\n>
<\0>
.text
/ integer to long
cr58=.+2; 0
.byte 77,0;.byte 77,0
L113
.data
L113:<GI!>
<sxt>;.byte 311;<\n>
<\0>
.text
/ long to integer
cr59=.+2; 0
.byte 20,10;.byte 77,0
L114
.data
L114:<mov>;.byte 301;<+,I\n>
<\0>
.text
.byte 177,10;.byte 77,0
L115
.data
L115:<GB>
<mov>;.byte 243;<+2(I),I\n>
<\0>
.text
.byte 77,10;.byte 77,0
L116
.data
L116:<GA>
<mov>;.byte 311;<+,I\n>
<\0>
.text
/ *, /, remainder for longs.
cr82=.+2; 0
.byte 77,10;.byte 77,10
L117
.data
L117:<KC>
<GC>
<jsr>;.byte 360;<c,M\n>
<add>;.byte 244;<10,sp\n>
<\0>
.text
/ =*, =/, =rem for longs
/ Operands of the form &x op y, so stack space is known.
cr86=.+2; 0
.byte 77,0;.byte 77,10
L118
.data
L118:<KC>
<GC>
<jsr>;.byte 360;<c,M\n>
<add>;.byte 244;<6,sp\n>
<\0>
.text
/
/ c code tables -- compile for side effects.
/ Olso set condition codes properly (except for ++, --)
/

.globl	_efftab

.data
_efftab=.
	30.;	ci70
	31.;	ci70
	32.;	ci70
	33.;	ci70
	80.;	ci80
	70.;	ci70
	71.;	ci70	/ - like +
	78.;	ci78
	85.;	ci78
	75.;	ci75
	76.;	ci76
	92.;	ci92
	0
.text

/ =
ci80=.+2; 0
.data;move1:;.text;.byte 20,0;.byte 4,0
L119
.text;.byte 20,5;.byte 4,4
L119
.data
L119:<M'C>;.byte 301;<\n>
<\0>
.text
.data;move2:;.text;.byte 177,0;.byte 4,0
L120
.text;.byte 177,5;.byte 4,4
L120
.data
L120:<GB>
<M'C>;.byte 243;<(I)\n>
<\0>
.text
.data;move3:;.text;.byte 20,0;.byte 20,1
L121
.text;.byte 20,3;.byte 20,0
L121
.data
L121:<ML>;.byte 302;<,A\n>
<\0>
.text
.data;move4:;.text;.byte 20,3;.byte 177,0
L122
.text;.byte 20,0;.byte 177,1
L122
.data
L122:<KB>
<ML>;.byte 242;<(I),A\n>
<\0>
.text
.data;move5:;.text;.byte 20,0;.byte 77,0
L123
.data
L123:<KA>
<MC>;.byte 311;<,A\n>
<\0>
.text
.data;move6:;.text;.byte 177,0;.byte 20,1
L124
.text;.byte 177,3;.byte 20,0
L124
.data
L124:<GB>
<ML>;.byte 302;<,#(I)\n>
<\0>
.text
.data;move7:;.text;.byte 177,0;.byte 124,1
L125
.text;.byte 177,3;.byte 124,0
L125
.data
L125:<GB>
<KJ>
<ML>;.byte 242;<(J),#(I)\n>
<\0>
.text
.data;move8:;.text;.byte 177,0;.byte 24,0
L126
.data
L126:<GB>
<KI>
<MC>;.byte 312;<,#(I)\n>
<\0>
.text
.data;move9:;.text;.byte 124,0;.byte 177,1
L127
.text;.byte 124,3;.byte 177,0
L127
.data
L127:<KB>
<GJ>
<ML>;.byte 242;<(I),#(J)\n>
<\0>
.text
.data;move10:;.text;.byte 124,0;.byte 77,0
L128
.data
L128:<KA>
<GJ>
<MC>;.byte 311;<,#(J)\n>
<\0>
.text
.data;move11:;.text;.byte 177,0;.byte 177,1
L129
.text;.byte 177,3;.byte 177,0
L129
.data
L129:<GD>
<KB>
<ML>;.byte 242;<(I),*(sp)+\n>
<\0>
.text
.data;move12:;.text;.byte 177,0;.byte 77,0
L130
.data
L130:<GD>
<KA>
<MC>;.byte 311;<,*(sp)+\n>
<\0>
.text
.byte 20,10;.byte 4,0
L131
.data
L131:<clr>;.byte 301;<\n>
<clr>;.byte 301;<+\n>
<\0>
.text
.byte 20,10;.byte 20,1
L132
.data
L132:<mov>;.byte 302;<,A+\n>
<sxt>;.byte 301;<\n>
<\0>
.text
.byte 20,10;.byte 177,1
L133
.data
L133:<mov>;.byte 242;<(I),A+\n>
<sxt>;.byte 301;<\n>
<\0>
.text
.byte 20,10;.byte 77,0
L134
.data
L134:<KA>
<mov>;.byte 311;<,A+\n>
<sxt>;.byte 301;<\n>
<\0>
.text
.byte 20,10;.byte 77,4
L135
.data
L135:<KA>
<setl\n>
<movfi>;.byte 311;<,A\n>
<seti\n>
<\0>
.text
.byte 124,10;.byte 77,4
L136
.data
L136:<KA>
<GJ>
<setl\n>
<movfi>;.byte 311;<,#(J)\n>
<seti\n>
<\0>
.text
.data;move13a:;.text;.byte 20,10;.byte 10,0
L137
.data
L137:<M>;.byte 302;<,A+\n>
<V>;.byte 301;<\n>
<\0>
.text
.data;move13:;.text;.byte 20,10;.byte 20,10
L138
.data
L138:<M>;.byte 302;<,A\n>
<M>;.byte 302;<+,A+\n>
<V>;.byte 301;<\n>
<\0>
.text
.data;move14:;.text;.byte 20,10;.byte 177,10
L139
.data
L139:<KB>
<M>;.byte 242;<(I),A\n>
<M>;.byte 242;<+2(I),A+\n>
<V>;.byte 301;<\n>
<\0>
.text
.data;move14a:;.text;.byte 177,10;.byte 10,0
L140
.data
L140:<GB>
<M>;.byte 302;<,2+#(I)\n>
<V>;.byte 243;<(I)\n>
<\0>
.text
.data;move15:;.text;.byte 20,10;.byte 77,10
L141
.data
L141:<KA>
<M>;.byte 311;<,A\n>
<M>;.byte 311;<+,A+\n>
<V>;.byte 301;<\n>
<\0>
.text
.byte 177,10;.byte 20,1
L142
.data
L142:<GB>
<mov>;.byte 302;<,#+2(I)\n>
<sxt>;.byte 243;<(I)\n>
<\0>
.text
.data;move16:;.text;.byte 124,10;.byte 77,10
L143
.data
L143:<KA>
<GJ>
<M>;.byte 311;<+,#+2(J)\n>
<V>;.byte 243;<(J)\n>
<M>;.byte 311;<,#(J)\n>
<\0>
.text
.byte 177,10;.byte 77,0
L144
.data
L144:<KC>
<GB>
<mov>;.byte 250;<sp)+,#+2(I)\n>
<sxt>;.byte 243;<(I)\n>
<\0>
.text
.data;move17:;.text;.byte 177,10;.byte 77,10
L145
.data
L145:<KC>
<GB>
<M>;.byte 250;<sp)+,#(I)\n>
<M>;.byte 250;<sp)+,#+2(I)\n>
<V>;.byte 243;<(I)\n>
<\0>
.text
/ =| and =& ~
ci78=.+2; 0
.byte 20,0;.byte 20,0
L146
.text;L146=move3

.byte 20,0;.byte 77,0
L147
.text;L147=move5

.byte 177,0;.byte 20,0
L148
.text;L148=move6

.byte 177,0;.byte 124,0
L149
.text;L149=move7

.byte 177,0;.byte 24,0
L150
.text;L150=move8

.byte 124,0;.byte 177,0
L151
.text;L151=move9

.byte 124,0;.byte 77,0
L152
.text;L152=move10

.byte 177,0;.byte 177,0
L153
.text;L153=move11

.byte 177,0;.byte 77,0
L154
.text;L154=move12

.byte 20,10;.byte 10,0
L155
.text;L155=move13a

.byte 20,10;.byte 20,10
L156
.text;L156=move13

.byte 20,10;.byte 177,10
L157
.text;L157=move14

.byte 20,10;.byte 77,10
L158
.text;L158=move15

.byte 177,10;.byte 10,0
L159
.text;L159=move14a

.byte 124,10;.byte 77,10
L160
.text;L160=move16

.byte 177,10;.byte 77,10
L161
.text;L161=move17

/ =+
ci70=.+2; 0
.byte 177,0;.byte 4,0
L162
.text;.byte 20,0;.byte 4,0
L162
.data
L162:<\0>
.text
.byte 20,0;.byte 5,0
L163
.data
L163:<M'C>;.byte 301;<\n>
<\0>
.text
.byte 20,1;.byte 20,1
L164
.text;L164=move3

.byte 20,1;.byte 177,1
L165
.text;L165=move4

.byte 20,1;.byte 77,0
L166
.text;L166=move5

.byte 177,0;.byte 5,0
L167
.text;L167=move2

.byte 124,1;.byte 177,1
L168
.text;L168=move9

.byte 20,0;.byte 177,1
L169
.data
L169:<KB>
<movC>;.byte 301;<',J\n>
<M>;.byte 242;<(I),J\n>
<movC>;.byte 312;<,A\n>
<\0>
.text
.byte 20,0;.byte 77,0
L170
.data
L170:<KA>
<movC>;.byte 301;<',J\n>
<M>;.byte 311;<,J\n>
<movC>;.byte 312;<,A\n>
<\0>
.text
.byte 124,1;.byte 77,0
L171
.text;L171=move10

.byte 177,1;.byte 77,0
L172
.text;L172=move12

.byte 177,0;.byte 77,0
L173
.data
L173:<KC>
<GB>
<movC>;.byte 243;<(I),J\n>
<M>;.byte 250;<sp)+,J\n>
<movC>;.byte 312;<,#(I)\n>
<\0>
.text
.byte 20,10;.byte 10,0
L174
.text;L174=move13a

.byte 20,10;.byte 20,10
L175
.text;L175=move13

.byte 20,10;.byte 177,10
L176
.text;L176=move14

.byte 20,10;.byte 77,10
L177
.text;L177=move15

.byte 177,10;.byte 10,0
L178
.text;L178=move14a

.byte 124,10;.byte 77,10
L179
.text;L179=move16

.byte 177,10;.byte 77,10
L180
.text;L180=move17

/ =>> (all harder cases handled by =<< -)
ci75=.+2; 0
.byte 20,0;.byte 5,0
L181
.data
L181:<asrC>;.byte 301;<\n>
<\0>
.text
.byte 177,0;.byte 5,0
L182
.data
L182:<GB>
<asrC>;.byte 243;<(I)\n>
<\0>
.text
/ =<<
ci76=.+2; 0
.byte 20,0;.byte 5,0
L183
.data
L183:<aslC>;.byte 301;<\n>
<\0>
.text
.byte 177,0;.byte 5,0
L184
.data
L184:<GB>
<aslC>;.byte 243;<(I)\n>
<\0>
.text
.byte 11,0;.byte 20,1
L185
.data
L185:<ash>;.byte 302;<,A\n>
<\0>
.text
.byte 11,0;.byte 177,1
L186
.data
L186:<KB>
<ash>;.byte 242;<(I),A\n>
<\0>
.text
.byte 11,0;.byte 77,0
L187
.data
L187:<KA>
<ash>;.byte 311;<,A\n>
<\0>
.text
/ =<< for longs
ci92=.+2; 0
.byte 20,10;.byte 20,1
L188
.data
L188:<GA>
<ashc>;.byte 302;<,I\n>
<mov>;.byte 311;<,A\n>
<mov>;.byte 311;<+,A+\n>
<\0>
.text
.byte 20,10;.byte 77,0
L189
.data
L189:<KC>
<GA>
<ashc>;.byte 250;<sp)+,I\n>
<mov>;.byte 311;<,A\n>
<mov>;.byte 311;<+,A+\n>
<\0>
.text
.byte 177,10;.byte 77,0
L190
.data
L190:<GD>
<KC>
<mov>;.byte 262;<(sp),I\n>
<mov>;.byte 262;<(I),I+\n>
<mov>;.byte 250;<I),I\n>
<ashc>;.byte 250;<sp)+,I\n>
<mov>;.byte 311;<,*(sp)\n>
<mov>;.byte 250;<sp)+,I\n>
<mov>;.byte 311;<+,2(I)\n>
<\0>
.text
/
/ c code tables-- set condition codes
/

.globl	_cctab

.data
_cctab=.
	106.;	cc60
	28.;	rest
	55.;	rest
	34.;	rest
	35.;	rest
	36.;	rest
	37.;	rest
	40.;	rest
	41.;	rest
	42.;	rest
	43.;	rest
	45.;	rest
	46.;	rest
	81.;	cc81	/ & as in "if ((a&b)==0)"
	48.;	rest
	60.;	cc60
	61.;	cc60
	62.;	cc60
	63.;	cc60
	64.;	cc60
	65.;	cc60
	66.;	cc60
	67.;	cc60
	68.;	cc60
	69.;	cc60
	72.;	rest
	73.;	rest
	79.;	rest
	0
.text

/ relationals
cc60=.+2; 0
.byte 20,0;.byte 4,0
L191
.text;.byte 20,5;.byte 4,4
L191
.text;L191=move1

.byte 20,4;.byte 4,0
L192
.data
L192:<movof>;.byte 301;<,I\n>
<\0>
.text
.byte 177,0;.byte 4,0
L193
.text;.byte 177,5;.byte 4,4
L193
.text;L193=move2

.byte 177,4;.byte 4,0
L194
.data
L194:<GB>
<movof>;.byte 243;<(I),I\n>
<\0>
.text
.byte 77,0;.byte 4,0
L195
.text;.byte 77,4;.byte 4,4
L195
.data
L195:<GE>
<\0>
.text
.byte 20,1;.byte 20,1
L196
.text;.byte 20,3;.byte 20,3
L196
.text;L196=move3

.byte 177,1;.byte 20,1
L197
.text;.byte 177,3;.byte 20,3
L197
.text;L197=move6

.byte 77,0;.byte 20,1
L198
.text;.byte 77,4;.byte 20,5
L198
.text;L198=add1

.byte 177,1;.byte 124,1
L199
.text;.byte 177,3;.byte 124,3
L199
.text;L199=move7

.byte 177,1;.byte 24,0
L200
.text;L200=move8

.byte 77,0;.byte 124,1
L201
.text;.byte 77,4;.byte 124,5
L201
.text;L201=add2

.byte 77,0;.byte 24,0
L202
.text;.byte 77,4;.byte 24,4
L202
.text;L202=add3

.byte 177,1;.byte 177,1
L203
.text;.byte 177,3;.byte 177,3
L203
.text;L203=move11

.byte 177,1;.byte 77,0
L204
.text;L204=move12

.byte 77,0;.byte 77,0
L205
.text;.byte 77,4;.byte 77,4
L205
.text;L205=add5

/ & as in "if ((a&b) ==0)"
cc81=.+2; 0
.byte 20,0;.byte 20,0
L206
.text;L206=move3

.byte 177,0;.byte 20,0
L207
.text;L207=move6

.byte 77,0;.byte 20,0
L208
.text;L208=add1

.byte 77,0;.byte 24,0
L209
.text;L209=add3

.byte 77,0;.byte 77,0
L210
.text;L210=add5

/ set codes right
rest=.+2; 0
.byte 77,0;.byte 77,0
L211
.text;.byte 77,4;.byte 77,4
L211
.data
L211:<HA>
<\0>
.text
/
/ c code tables-- expression to -(sp)
/

.globl	_sptab

.data
_sptab=.
	106.;	cs106
	40.;	cs40
	41.;	cs40
	55.;	cs40
	48.;	cs40
	58.;	cs58
	56.;	cs56
	0
.text


/ name
cs106=.+2; 0
.byte 4,0;.byte 77,0
L212
.text;.byte 4,4;.byte 77,0
L212
.data
L212:<clrC>;.byte 255;<(sp)\n>
<\0>
.text
.byte 20,1;.byte 77,0
L213
.data
L213:<mov>;.byte 301;<,-(sp)\n>
<\0>
.text
.byte 177,1;.byte 77,0
L214
.data
L214:<GB>
<mov>;.byte 243;<(I),-(sp)\n>
<\0>
.text
.byte 20,10;.byte 77,0
L215
.data
L215:<mov>;.byte 301;<+,-(sp)\n>
<mov>;.byte 301;<,-(sp)\n>
<\0>
.text
/ +, -, |, &~
cs40=.+2; 0
.byte 77,0;.byte 5,0
L216
.data
L216:<GC>
<M'>;.byte 250;<sp)\n>
<\0>
.text
.byte 77,0;.byte 20,1
L217
.data
L217:<GC>
<M>;.byte 302;<,(sp)\n>
<\0>
.text
.byte 77,0;.byte 177,1
L218
.data
L218:<GC>
<KB>
<M>;.byte 242;<(I),(sp)\n>
<\0>
.text
.byte 77,0;.byte 77,0
L219
.data
L219:<GC>
<KA>
<M>;.byte 311;<,(sp)\n>
<\0>
.text
/ integer to long
cs58=.+2; 0
.byte 77,0;.byte 77,0
L220
.data
L220:<GC>
<sxt>;.byte 255;<(sp)\n>
<\0>
.text
/ float to long
cs56=.+2; 0
.byte 77,4;.byte 77,0
L221
.data
L221:<GA>
<setl\n>
<movfi>;.byte 311;<,-(sp)\n>
<seti\n>
<\0>
.text
.text; 0
