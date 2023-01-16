#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <openssl/sha.h>


#define SHA_LENGTH 32 // in bytes
#define MAX_BUFFER_SIZE (262144*SHA_LENGTH) // 1 TiB / 4 MiB
#define BLOCK_SIZE (4*1024*1024)

int compute_dropbox_hash(char const * filename, unsigned char out[SHA_LENGTH])
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0) { return 1; }

    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if (err < 0) { return 1; }

    size_t file_size = statbuf.st_size;
    size_t block_count = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    unsigned char * begin = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (begin == MAP_FAILED) { return 1; }

    if (close(fd)) { return 1; }

    unsigned char * end = begin + file_size;
    unsigned char * iter = begin;
    size_t read_size = 0;

    unsigned char * buf = malloc(SHA_LENGTH * block_count);
    if (buf == NULL) { return 1; }

    for (size_t i = 0 ; i < block_count ; ++i) {
        unsigned char * block_begin = begin + (BLOCK_SIZE * i);
        unsigned char * buf_begin = buf + (SHA_LENGTH * i);
        size_t block_size = (end - block_begin) < BLOCK_SIZE ? (end - block_begin) : BLOCK_SIZE;
        if (SHA256(block_begin, block_size, buf_begin) == NULL) { return 1; }
    }
    if (SHA256(buf, SHA_LENGTH * block_count, out) == NULL) { return 1; }

    free(buf);

    err = munmap(begin, statbuf.st_size);

    if (err != 0) { return 1; }
    return 0;
}

int main(int argc, char ** argv)
{
    unsigned char out[SHA_LENGTH];

    if (argc <= 1) {
        printf("error\n");
        exit(1);
    }

    const char * filename = argv[1];
    int r = compute_dropbox_hash(filename, out);
    if (r) {
        fprintf(stderr, "Error!\n");
        exit(1);
    }
    for (size_t i = 0 ; i < SHA_LENGTH ; ++i) {
        printf("%02x", out[i]);
    }
    printf("\n");
}
