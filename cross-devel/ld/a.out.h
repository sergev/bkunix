/*
 * Structure of a.out file.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef _AOUT_H_
#define _AOUT_H_ 1

struct exec {
	short		a_magic;	/* magic number */
	unsigned short	a_text; 	/* size of text segment */
	unsigned short	a_data; 	/* size of initialized data */
	unsigned short	a_bss;  	/* size of unitialized data */
	unsigned short	a_syms; 	/* size of symbol table */
	unsigned short	a_entry; 	/* entry point */
	unsigned short	a_unused;	/* not used */
	unsigned short	a_flag; 	/* relocation info stripped */
};

#define	A_FMAGIC	0407		/* normal */
#define	A_NMAGIC	0410		/* read-only text */
#define	A_IMAGIC	0411		/* separated I&D */
#define	N_BADMAG(x)	((x).a_magic != A_FMAGIC && \
			(x).a_magic != A_NMAGIC && \
			(x).a_magic != A_IMAGIC)

#define A_NRELFLG	01		/* non-relocatable flag */

#define	N_TXTOFF(x)	(sizeof(struct exec))
#define	N_RELOFF(x)	(N_TXTOFF(x) + (x).a_text + (x).a_data)
#define	N_SYMOFF(x)	(N_RELOFF(x) + (((x).a_flag & A_NRELFLG) ? 0 : \
			((x).a_text + (x).a_data)))

/*
 * relocation types
 */
#define A_RPCREL	001		/* PC-relative flag */
#define A_RMASK		016		/* bit mask */
#define A_RABS		000		/* absolute */
#define A_RTEXT		002		/* reference to text */
#define A_RDATA		004		/* reference to data */
#define A_RBSS		006		/* reference to bss */
#define	A_REXT		010		/* external symbol */
#define A_RINDEX(r)	((r) >> 4)	/* external symbol index */
#define A_RPUTINDEX(r)	((r) << 4)

/*
 * symbol table entry
 */
struct nlist {
	char    	n_name[8];	/* symbol name */
	short     	n_type;    	/* type flag */
	unsigned short	n_value;	/* value */
};

/*
 * values for type flag
 */
#define	N_UNDF		000		/* undefined */
#define	N_ABS		001		/* absolute */
#define	N_TEXT		002		/* text symbol */
#define	N_DATA		003		/* data symbol */
#define	N_BSS		004		/* bss symbol */
#define	N_COMM		005		/* for ld internal use only */
#define	N_TYPE		037
#define	N_REG		024		/* register name */
#define	N_FN		037		/* file name symbol */
#define	N_EXT		040		/* external bit, or'ed in */

#endif /* _AOUT_H_ */
