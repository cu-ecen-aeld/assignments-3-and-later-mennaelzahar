#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <getopt.h>

#define PORT 9000
#define BUFFER_SIZE 1024
#define DATA_FILE "/var/tmp/aesdsocketdata"

volatile sig_atomic_t stop_flag = 0;
int server_fd = -1;
int daemon_mode = 0;

void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        stop_flag = 1;
    }
}

void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
        exit(EXIT_FAILURE);
    }
}

int setup_server_socket() {
    int server_fd, opt = 1;
    struct sockaddr_in address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    if (!daemon_mode) {
        printf("Server listening on port 9000...\n");
    }
    
    return server_fd;
}

void daemonize() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        exit(EXIT_SUCCESS); // Parent exits
    }
    
    // Child becomes session leader
    if (setsid() < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }
    
    // Change working directory to root
    chdir("/");
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // Redirect stdio to /dev/null
    open("/dev/null", O_RDONLY); // stdin
    open("/dev/null", O_WRONLY); // stdout
    open("/dev/null", O_WRONLY); // stderr
}

void handle_client(int client_fd, struct sockaddr_in *client_addr) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    syslog(LOG_INFO, "Accepted connection from %s", client_ip);

    FILE *data_file = fopen(DATA_FILE, "a+");
    if (data_file == NULL) {
        perror("fopen");
        close(client_fd);
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    char *packet = NULL;
    size_t packet_size = 0;

    while ((bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        if (bytes_read == -1) {
            perror("recv");
            break;
        }

        buffer[bytes_read] = '\0';

        char *newline = strchr(buffer, '\n');
        if (newline) {
            size_t chunk_size = newline - buffer + 1;
            
            packet = realloc(packet, packet_size + chunk_size + 1);
            if (!packet) {
                perror("realloc");
                break;
            }
            memcpy(packet + packet_size, buffer, chunk_size);
            packet_size += chunk_size;
            packet[packet_size] = '\0';

            fwrite(packet, 1, packet_size, data_file);
            fflush(data_file);

            rewind(data_file);
            char file_buffer[BUFFER_SIZE];
            size_t file_bytes;
            while ((file_bytes = fread(file_buffer, 1, BUFFER_SIZE, data_file)) > 0) {
                if (send(client_fd, file_buffer, file_bytes, 0) == -1) {
                    perror("send");
                    break;
                }
            }

            free(packet);
            packet = NULL;
            packet_size = 0;

            if (chunk_size < (size_t)bytes_read) {
                size_t remaining = bytes_read - chunk_size;
                packet = malloc(remaining + 1);
                if (!packet) {
                    perror("malloc");
                    break;
                }
                memcpy(packet, buffer + chunk_size, remaining);
                packet_size = remaining;
                packet[packet_size] = '\0';
            }
        } else {
            packet = realloc(packet, packet_size + bytes_read + 1);
            if (!packet) {
                perror("realloc");
                break;
            }
            memcpy(packet + packet_size, buffer, bytes_read);
            packet_size += bytes_read;
            packet[packet_size] = '\0';
        }
    }

    if (packet) {
        free(packet);
    }

    fclose(data_file);
    close(client_fd);
    syslog(LOG_INFO, "Closed connection from %s", client_ip);
}

void cleanup() {
    if (server_fd != -1) {
        close(server_fd);
    }
    remove(DATA_FILE);
}

void print_usage(char *progname) {
    fprintf(stderr, "Usage: %s [-d]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -d  Run as daemon\n");
}

int main(int argc, char *argv[]) {
    int opt;
    
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                daemon_mode = 1;
                break;
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    openlog("aesdsocket", LOG_PID, LOG_USER);

    setup_signal_handlers();
    server_fd = setup_server_socket();
    if (server_fd == -1) {
        return EXIT_FAILURE;
    }

    if (daemon_mode) {
        daemonize();
    }

    atexit(cleanup);

    while (!stop_flag) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd;

        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            if (stop_flag) {
                break;
            }
            perror("accept");
            continue;
        }

        handle_client(client_fd, &client_addr);
    }

    cleanup();
    closelog();
    return EXIT_SUCCESS;
}