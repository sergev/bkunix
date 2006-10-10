/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */

/*	machine dependent file  */

label( n ){
	printf( "L%d:\n", n );
	}

tlabel(){
	lccopy( 2 );
	printf( ":\n" );
	}
