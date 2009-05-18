#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef WIN32
	#include <io.h>
#endif

#ifndef O_BINARY
	#define O_BINARY 0
#endif

#ifndef O_SEQUENTIAL
	#define O_SEQUENTIAL 0
#endif

char hex[] = "0123456789abcdef";

int main(int argc, char* argv[]) {

	if ( argc < 2 )
		return EXIT_FAILURE;

	int srcFd = open( argv[1], O_RDONLY | O_BINARY | O_SEQUENTIAL );
	if ( srcFd <= 0 ) {
		
		printf( "file not found %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	int dstFd = open( argv[2], O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_SEQUENTIAL, 00700 );
	if ( dstFd <= 0 ) {
		
		printf( "unable to create %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	char srcBuf[8192];
	char dstBuf[sizeof(srcBuf)*5]; // 0xff,

	int readCount, writtenCount;
	do {
	
		readCount = read(srcFd, srcBuf, sizeof(srcBuf));
		if ( readCount < 0 )
			return EXIT_FAILURE;

		for ( int i = 0; i < readCount; ++i ) {

			dstBuf[i*5+0] = '0';
			dstBuf[i*5+1] = 'x';
			dstBuf[i*5+2] = hex[srcBuf[i]>>4];
			dstBuf[i*5+3] = hex[srcBuf[i]&15];
			dstBuf[i*5+4] = ',';
		}
		
		writtenCount = write(dstFd, dstBuf, readCount*5);
		if ( writtenCount < 0 )
			return EXIT_FAILURE;

	} while ( readCount == sizeof(srcBuf) );

	close(dstFd);
	close(srcFd);
	return EXIT_SUCCESS;
}
