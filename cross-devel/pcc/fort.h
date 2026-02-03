/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */

/*	machine dependent file  */

label( n ){
	printf( "L%d:\n", n );
	}

tlabel(){
	lccopy( 2 );
	printf( ":\n" );
	}
