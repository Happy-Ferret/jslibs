/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef WIN32
	#include <io.h>
#else
	#include <unistd.h>
#endif

#ifndef O_BINARY
	#define O_BINARY 0
#endif

#ifndef O_SEQUENTIAL
	#define O_SEQUENTIAL 0
#endif

char hex[] = "0123456789ABCDEF";

int main(int argc, char* argv[]) {
	
	argc--;

//	for ( int i = 0; i < sizeof(str); i++ )
//		printf("%d:%d \n", i, str[i] );

//	int f = open(argv[1], O_WRONLY | O_BINARY );
//	for ( unsigned char c = 0; c < 255; c++ )
//		write(f, &c, 1);
//	close(f);

//	return EXIT_FAILURE;

	printf("Running %s with %d arguments\n", argv[0], argc);

	if ( argc < 2 )
		return EXIT_FAILURE;


	printf("Deleting the destination file %s\n", argv[2]);
	remove(argv[2]);

	printf("Opening source file %s.\n", argv[1]);
	int srcFd = open( argv[1], O_RDONLY | O_BINARY | O_SEQUENTIAL );
	if ( srcFd <= 0 ) {

		printf( "Unable to open %s.\n", argv[1]);
		return EXIT_FAILURE;
	}

	printf("Opening destination file %s.\n", argv[2]);
	FILE *dstFile = fopen(argv[2], "w");
	if ( !dstFile ) {

		printf( "Unable to create %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	unsigned char srcBuf[8192];
	int readCount;

	fputc( '"', dstFile );
	do {

		readCount = read(srcFd, srcBuf, sizeof(srcBuf));
		if ( readCount < 0 ) {

			printf( "Source file read error\n" );
			return EXIT_FAILURE;
		}

		for ( int i = 0; i < readCount; ++i ) {
			
			unsigned char c = srcBuf[i];

			switch (c) {
				case '\0':
					fputs( "\\000", dstFile );
					continue;
				case '\a':
					fputs( "\\a", dstFile );
					continue;
				case '\b':
					fputs( "\\b", dstFile );
					continue;
				case '\t':
					fputs( "\\t", dstFile );
					continue;
				case '\n':
					fputs( "\\n", dstFile );
					continue;
				case '\v':
					fputs( "\\v", dstFile );
					continue;
				case '\f':
					fputs( "\\f", dstFile );
					continue;
				case '\r':
					fputs( "\\r", dstFile );
					continue;
				case '"':
					fputs( "\\\"", dstFile );
					continue;
				case '\\':
					fputs( "\\\\", dstFile );
					continue;
			}

			if ( c < 32 || c >= 127 ) {
				
				fputc( '\\', dstFile );
				fputc( hex[ (c>>6) & 0x7 ], dstFile );
				fputc( hex[ (c>>3) & 0x7 ], dstFile );
				fputc( hex[ (c>>0) & 0x7 ], dstFile );
				continue;
			}

			fputc( c, dstFile );
		}

	} while ( readCount == sizeof(srcBuf) );

	fputc( '"', dstFile );
	fclose(dstFile);
	close(srcFd);

	printf("Done.\n");
	return EXIT_SUCCESS;
}
