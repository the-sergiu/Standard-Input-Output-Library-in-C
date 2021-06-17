#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Windows.h>
#define DLL_EXPORTS ".c"
#include "so_stdio.h"

#define BUFSIZE 4096

typedef struct _so_file {
	HANDLE hFile;
	char readBuffer[BUFSIZE];
	char writeBuffer[BUFSIZE];
	int readInBuffer;
	int writtenSize;
	int readBytesSize;
	int err;
	int eof;
	long writeCursor;
	long readCursor;
} SO_FILE;

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *file = malloc(sizeof(SO_FILE));
	HANDLE hFile;
	DWORD dwPos;

	file->readCursor = 0;
	file->writeCursor = 0;
	if (strncmp(mode, "r", 2) == 0) {
		hFile = CreateFile(
			pathname,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		free(file);
		return NULL;
	}

	} else if (strncmp(mode, "r+", 2) == 0) {
		hFile = CreateFile(
			pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (hFile == INVALID_HANDLE_VALUE) {
			free(file);
			return NULL;
		}
	} else if (strncmp(mode, "w", 2) == 0) {
		hFile = CreateFile(
			pathname,
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			free(file);
			return NULL;
		}
	} else if (strncmp(mode, "w+", 2) == 0) {
		hFile = CreateFile(
			pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (hFile == INVALID_HANDLE_VALUE) {
			free(file);
			return NULL;
		}

	} else if (strncmp(mode, "a", 2) == 0) {
		hFile = CreateFile(
			pathname,
			FILE_APPEND_DATA,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (hFile == INVALID_HANDLE_VALUE) {
			free(file);
			printf("CreateFile error - a\n");
			return NULL;
		}
	} else if (strncmp(mode, "a+", 2) == 0) {
		hFile = CreateFile(
			pathname,
			GENERIC_READ | FILE_APPEND_DATA,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (hFile == INVALID_HANDLE_VALUE) {
			free(file);
			printf("CreateFile error - a+\n");
			return NULL;
		}
		/* lseek */
		dwPos = SetFilePointer(
			hFile,
			0,
			NULL,
			FILE_END
		);
	} else {
		free(file);
		return NULL;
	}

	memset(file->writeBuffer, 0, BUFSIZE);
	memset(file->readBuffer, 0, BUFSIZE);
	file->hFile = hFile;
	file->err = 0;
	file->eof = 0;
	file->writtenSize = 0;
	file->readInBuffer = 0;
	file->readBytesSize = 0;

	return file;
}

int so_fclose(SO_FILE *stream)
{
	BOOL bRet;
	int flushReturn;

	if (stream->writtenSize % BUFSIZE > 0) {
		flushReturn = so_fflush(stream);
		if (flushReturn == -1) {
			free(stream);
			return -1;
		}
	}

	bRet = CloseHandle(stream->hFile);
	if (bRet == FALSE) {
		free(stream);
		return -1;
	}
	if (stream != NULL)
		free(stream);
	return 0;
}

int so_fgetc(SO_FILE *stream)
{
	int bytesDif;
	unsigned char returnedChar;
	DWORD dwRet;

	/* Daca numarul de caractere de citit e 0*/
	if (stream->readInBuffer == 0) {
		/* Mai facem un apel de sistem cand s-a umplut buffer-ul*/
		dwRet = ReadFile(
				stream->hFile,
				stream->readBuffer,
				BUFSIZE,
				&stream->readBytesSize,
				NULL
			);
		/* Verificam daca mai are ce sa citeasca */
		if (stream->readBytesSize <= 0) {
			stream->eof = 1;
			stream->err = 1;
			return -1;
		}
		/* Actualizam numarul de caractere citit */
		stream->readInBuffer = stream->readBytesSize;
	}
	/* Calculam diferenta intre cat citim si cat mai e de citit */
	bytesDif = stream->readBytesSize - stream->readInBuffer;
	returnedChar = stream->readBuffer[bytesDif];
	/* Actualizam numarul de caractere citite */
	stream->readInBuffer = stream->readInBuffer - sizeof(unsigned char);
	/* Incrementam cursorul */
	stream->readCursor = stream->readCursor + sizeof(unsigned char);
	/* Returnam caracterul */
	return returnedChar;
}

HANDLE so_fileno(SO_FILE *stream)
{
	return stream->hFile;
}

int so_feof(SO_FILE *stream)
{
	if (stream->eof == 1)
		return -1;
	else
		return 0;
}

int so_ferror(SO_FILE *stream)
{
	if (stream->err == 1)
		return 1;
	else
		return 0;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	unsigned char charRead = 0;
	int fgetcReturn = 0;
	/* nmemb elemente * dimensiune size */
	int sizeToRead = size * nmemb;
	int bytesReadCounter = 0;

	while (bytesReadCounter < sizeToRead) {
		/* Citim caracterul */
		fgetcReturn = so_fgetc(stream);
		if (fgetcReturn == -1) {
			stream->eof = 1;
			stream->err = 1;
			return bytesReadCounter/size;
		}
		charRead = (unsigned char)fgetcReturn;
		/* Scriem la adresa, actualizam dimensiunea citita*/
		memcpy((char *)ptr + bytesReadCounter,
				 &charRead, sizeof(charRead));
		bytesReadCounter = bytesReadCounter + sizeof(charRead);
	}
	return (bytesReadCounter / size);
}

int so_fputc(int c, SO_FILE *stream)
{
	DWORD writeReturn, bytes_written_now;

	/* Daca nu s-a umplut buffer-ul */
	if (stream->writtenSize != 0
		 && stream->writtenSize % BUFSIZE == 0) {
		/* Deschidem fisierul si scriem in el */
		writeReturn = WriteFile(
				stream->hFile,
				stream->writeBuffer,
				BUFSIZE,
				&bytes_written_now,
				NULL
			);
		memset(stream->writeBuffer, 0, BUFSIZE);
		/* verificare eroare */
		if (writeReturn == 0) {
			stream->err = 1;
			stream->eof = 1;
			return -1;
		}
	}
	/* Scriem caracterul pe o pozitie ce se incrementeaza */
	stream->writeBuffer[stream->writtenSize % BUFSIZE] = c;
	/* Incrementam numarul de caractere scris - practic pozitia */
	stream->writtenSize = stream->writtenSize + sizeof(unsigned char);
	/* Incrementam cursorul */
	stream->writeCursor = stream->writeCursor + sizeof(unsigned char);
	return stream->writeBuffer
			[(stream->writtenSize - sizeof(unsigned char))
			% BUFSIZE];
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	unsigned char charWritten = 0;
	unsigned char tempChar;
	/* nmemb elemente * dimensiune size */
	int sizeToRead = size * nmemb;
	int bytesWrittenCounter = 0;

	while (bytesWrittenCounter < sizeToRead) {
		memcpy(&tempChar, (char *)ptr + bytesWrittenCounter,
				sizeof(unsigned char));
		charWritten = so_fputc(tempChar, stream);
		if (charWritten == 0) {
			stream->err = 1;
			stream->eof = 1;
		}
		bytesWrittenCounter = bytesWrittenCounter + sizeof(charWritten);
	}
	return bytesWrittenCounter/size;
}

long so_ftell(SO_FILE *stream)
{
	if (stream->err == 1 || stream->eof == 1)
		return -1;
	if (stream->readCursor != 0)
		return stream->readCursor;
	else
		return stream->writeCursor;
}

int so_fflush(SO_FILE *stream)
{
	int writeReturn;
	DWORD bytes_written_now = 0;

	writeReturn = WriteFile(
				stream->hFile,
				stream->writeBuffer,
				stream->writtenSize % BUFSIZE,
				&bytes_written_now,
				NULL
			);
	memset(stream->writeBuffer, 0, BUFSIZE);
	stream->writtenSize = 0;
	stream->writeCursor = 0;
	stream->err = 0;
	stream->eof = 0;

	if (bytes_written_now == -1) {
		stream->err = 1;
		return -1;
	} else if (bytes_written_now == 0) {
		stream->eof = 1;
		return -1;
	}
	return 0;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
	long returnSeek;

	if (stream->writtenSize != 0)
		so_fflush(stream);
	returnSeek = SetFilePointer(stream->hFile, offset, NULL, whence);
	if (returnSeek == -1) {
		stream->err = 1;
		return -1;
	}
	stream->readCursor = returnSeek;
	stream->writeCursor = returnSeek;
	return 0;

}

SO_FILE *so_popen(const char *command, const char *type)
{
	UNREFERENCED_PARAMETER(command);
	UNREFERENCED_PARAMETER(type);
	return NULL;
}

int so_pclose(SO_FILE *stream) 
{
	UNREFERENCED_PARAMETER(stream);
	return 0;
}
