#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

void usage(void)
{
	printf("usage:\n"
		   "  -u to write in usermode\n"
		   "  -k to write into a pipe so it's buffered in the kernel. This case doesn't exit, so pipe fd-s don't close.\n");
}

void progress(unsigned int part, unsigned int total)
{
	static unsigned int percentage = 0;
	int i = 0;

	unsigned int new_percentage = part/(total/100);
	if (new_percentage > percentage) {
		percentage = new_percentage;
		printf("\r%3u%% ", percentage);
		for (i = 1; i <= percentage; ++i) {
			printf("#");
		}
		for (; i < 100; ++i) {
			printf(" ");
		}
		printf("|");
		if (percentage == 100)
			printf("\n");
	}
}

int main(int argc, char ** argv)
{
	unsigned int *p = NULL, j;
	unsigned int malloc_size = (1<<30);
	unsigned int size_to_fill = (1<<30);
	unsigned int size_filled = 0;
	unsigned int alloc_print_count = 0;
	unsigned int max_alloc_print_count = 10;
	int pipefd[2];
	int ret;
	ssize_t write_ret;
	unsigned int written = 0;

	if (argc != 2 || !strcmp(argv[1], "-h")) {
		usage();
	}
	if (!strcmp(argv[1], "-u")) {
		while (size_filled < size_to_fill) {
			p = (unsigned int *)malloc(malloc_size);
			while (p == NULL) {
				malloc_size >>= 1;
				p = (unsigned int *)malloc(malloc_size);
			}
			if (alloc_print_count < max_alloc_print_count) {
				printf("Allocated %u\n", malloc_size);
				alloc_print_count++;
			}
			j = 0xffffffff;
			for (int i = 0; i < (malloc_size)/sizeof(unsigned int); i++) {
				p[i] = j--;
			}
			size_filled += malloc_size;
			malloc_size = (1<<30);
			// Leak the memory on purpose
			p = NULL;
		}
	} else if (!strcmp(argv[1], "-k")) {
		malloc_size = (1<<16);
		size_to_fill = (1<<16);
		ret = pipe(pipefd);
		while (ret != 0) {
			ret = pipe(pipefd);
		}
		while (size_filled < size_to_fill) {
			p = (unsigned int *)malloc(malloc_size);
			while (p == NULL) {
				malloc_size >>= 1;
				p = (unsigned int *)malloc(malloc_size);
			}
			if (alloc_print_count < max_alloc_print_count) {
				printf("Allocated %u\n", malloc_size);
				alloc_print_count++;
			}
			j = 0xcccccccc;
			for (int i = 0; i < (malloc_size)/sizeof(unsigned int); i++) {
				p[i] = j--;
			}
			write_ret = write(pipefd[1], p, malloc_size);
			if (write_ret <= 0) {
				printf("Error %d %s\n", errno, strerror(errno));
				break;
			}
			written = write_ret;
			while (written < malloc_size) {
				write_ret = write(pipefd[1], p+(written/sizeof(unsigned int)), malloc_size);
				progress(write_ret, malloc_size);
				if (write_ret <= 0)
					break;
				written += write_ret;
			}
			size_filled += written;
			// Zero-out the memory in user mode
			memset(p, 0, malloc_size);
			// Here we shouldn't leak the memor
			free(p);
			malloc_size = (1<<16);
		}
		printf("Looping forever. Now reboot to upload mode.\n");
		while (1) {};
	} else {
		usage();
	}
	return 0;
}