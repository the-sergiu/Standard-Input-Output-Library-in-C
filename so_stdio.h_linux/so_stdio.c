#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "so_stdio.h"

#define BUFSIZE 4096

typedef struct _so_file {
	int fd;
	int flags[3];
	char readBuffer[BUFSIZE];
	char writeBuffer[BUFSIZE];
	int readInBuffer;
	int writtenSize;
	int readBytesSize;
	int err;
	int eof;
	long writeCursor;
	long readCursor;
	int pid;
} SO_FILE;

/* Rezolva testul 19 - rand write */
ssize_t xwrite(int fd, const void *buf, size_t count)
{
	size_t bytes_written = 0;

	while (bytes_written < count) {
		ssize_t bytes_written_now = write(fd,
						buf + bytes_written,
						count - bytes_written);

		if (bytes_written_now <= 0) /* I/O error */
			return -1;

		bytes_written += bytes_written_now;
	}

	return bytes_written;
}
/* Restul testelor se comporta normal pt xwrite/write */

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *file = malloc(sizeof(SO_FILE));
	int fd = 0;

	file->readCursor = SEEK_SET;
	file->writeCursor = SEEK_SET;
	if (strncmp(mode, "r", 2) == 0) {
		file->flags[0] = O_RDONLY;
		fd = open(pathname, file->flags[0]);
		if (fd < 0) {
			free(file);
			return NULL;
		}
	} else if (strncmp(mode, "r+", 2) == 0) {
		file->flags[0] = O_RDWR;
		fd = open(pathname, file->flags[0]);
		if (fd < 0) {
			free(file);
			return NULL;
		}
	} else if (strncmp(mode, "w", 2) == 0) {
		file->flags[0] = O_WRONLY;
		file->flags[1] = O_CREAT;
		file->flags[2] = O_TRUNC;
		fd = open(pathname, file->flags[1]
			| file->flags[2]
			| file->flags[0], 0644);
		if (fd < 0) {
			free(file);
			return NULL;
		}
	} else if (strncmp(mode, "w+", 2) == 0) {
		file->flags[0] = O_RDWR;
		file->flags[1] = O_CREAT;
		file->flags[2] = O_TRUNC;
		fd = open(pathname, file->flags[0]
			| file->flags[1]
			| file->flags[2], 0644);
		if (fd < 0) {
			free(file);
			return NULL;
		}
	} else if (strncmp(mode, "a", 2) == 0) {
		file->flags[0] = O_APPEND;
		file->flags[1] = O_CREAT;
		file->flags[2] = O_WRONLY;
		fd = open(pathname, file->flags[1]
			| file->flags[2]
			| file->flags[0], 0644);
		if (fd < 0) {
			free(file);
			return NULL;
		}
	} else if (strncmp(mode, "a+", 2) == 0) {
		file->flags[0] = O_APPEND;
		file->flags[1] = O_RDWR;
		file->flags[2] = O_CREAT;
		fd = open(pathname, file->flags[0]
			| file->flags[1]
			| file->flags[2], 0644);
		if (fd < 0) {
			free(file);
			return NULL;
		}
	} else {
		free(file);
		return NULL;
	}
	file->fd = fd;
	file->err = 0;
	file->eof = 0;
	file->writtenSize = 0;
	file->readInBuffer = 0;
	file->readBytesSize = 0;
	file->pid = 0;

	return file;
}

int so_fclose(SO_FILE *stream)
{
	int writeReturn = 0, closeReturn = 0;
	int count = 0;

	if (stream->writtenSize > 0) {
		writeReturn = xwrite(stream->fd,
					stream->writeBuffer,
					stream->writtenSize % BUFSIZE);
		if (writeReturn < 0)
			count = 1;
		/* Eroare in write */
		if (count == 1) {
			stream->err = 1;
			if (stream != NULL)
				free(stream);
			return -1;
		}
	}
	count = 0;
	closeReturn = close(stream->fd);
	/* Eroare in close */
	if (closeReturn < 0) {
		stream->err = 1;
		if (stream != NULL)
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

	/* Daca numarul de caractere de citit e 0*/
	if (stream->readInBuffer == 0) {

		/* Mai facem un apel de sistem cand s-a umplut buffer-ul*/
		stream->readBytesSize =
						read(stream->fd,
							stream->readBuffer,
							BUFSIZE);
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

int so_fileno(SO_FILE *stream)
{
	return stream->fd;
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
		charRead = (unsigned char *)fgetcReturn;
		/* Scriem la adresa, actualizam dimensiunea citita*/
		memcpy(ptr + bytesReadCounter, &charRead, sizeof(charRead));
		bytesReadCounter = bytesReadCounter + sizeof(charRead);
	}
	return (bytesReadCounter / size);
}

int so_fputc(int c, SO_FILE *stream)
{
	int writeReturn = 0;

	/* Daca nu s-a umplut buffer-ul */
	if (stream->writtenSize != 0
		&& stream->writtenSize % BUFSIZE == 0) {
		/* Deschidem fisierul si scriem in el */
		writeReturn = xwrite(stream->fd, stream->writeBuffer, BUFSIZE);
		/* verificare eroare */
		if (writeReturn == -1) {
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
		memcpy(&tempChar, ptr + bytesWrittenCounter,
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
	else if (stream->flags[0] == O_RDONLY)
		return stream->readCursor;
	else
		return stream->writeCursor;
}

int so_fflush(SO_FILE *stream)
{
	int writeReturn;

	writeReturn = xwrite(stream->fd, stream->writeBuffer,
						stream->writtenSize % BUFSIZE);
	if (writeReturn == -1) {
		stream->err = 1;
		stream->eof = 1;
		return -1;
	}
	memset(stream->writeBuffer, 0, BUFSIZE);
	stream->writtenSize = 0;
	stream->readCursor = 0;
	stream->writeCursor = 0;

	return 0;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
	long returnSeek;

	if (stream->writtenSize != 0) {
		write(stream->fd, stream->writeBuffer,
				stream->writtenSize % BUFSIZE);
		stream->writtenSize = 0;
		so_fflush(stream);
	}
	returnSeek = lseek(stream->fd, offset, whence);
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
	SO_FILE *file = malloc(sizeof(SO_FILE));
	int pipeFileDescriptors[2];

	/* rezolvare memcheck */
	file->eof = 0;
	file->err = 0;
	file->readCursor = 0;
	file->readInBuffer = 0;
	file->writeCursor = 0;
	file->readCursor = 0;
	file->writtenSize = 0;
	file->readBytesSize = 0;
	file->pid = 0;
	/* Transmiterea fd prin pipe */
	pipe(pipeFileDescriptors);
	file->pid = fork();
	if (file->pid == -1) {
		/* perror("Error fork!\n"); */
		free(file);
		return NULL;
	} else if (file->pid == 0) {
		/* Child node */
		if (strncmp(type, "r", 2) == 0) {
			close(pipeFileDescriptors[0]);
			dup2(pipeFileDescriptors[1], STDOUT_FILENO);
			execl("/bin/sh", "sh", "-c", command, NULL);
		} else if (strncmp(type, "w", 2) == 0) {
			close(pipeFileDescriptors[1]);
			dup2(pipeFileDescriptors[0], STDIN_FILENO);
			execl("/bin/sh", "sh", "-c", command, NULL);
		}
	} else {
		/* Parent node */
		if (strncmp(type, "r", 2) == 0) {
			close(pipeFileDescriptors[1]);
			file->fd = pipeFileDescriptors[0];
			file->flags[0] = O_RDONLY;
		} else if (strncmp(type, "w", 2) == 0) {
			file->flags[1] = O_CREAT;
			file->flags[0] = O_WRONLY;
			file->flags[2] = O_TRUNC;
			file->fd = pipeFileDescriptors[1];
			close(pipeFileDescriptors[0]);
		}
		return file;
	}
	if (file != NULL)
		free(file);
	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	int status = 0;
	int waitReturn = 0;

	/* Pentru popen-write */
	if (stream->flags[0] == O_WRONLY) {
		write(stream->fd, stream->writeBuffer,
				stream->writtenSize % BUFSIZE);
	}

	close(stream->fd);
	if (stream->pid > 0) {
		waitReturn = waitpid(stream->pid, &status, 0);
		if (waitReturn == -1) {
			free(stream);
			return -1;
		}
	}
	if (stream != NULL)
		free(stream);
	return status;
}
