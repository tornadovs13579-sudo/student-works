#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 8192

void ftp_send_cmd(int sock, const char *cmd) {
    char full[BUF_SIZE];
    snprintf(full, sizeof(full), "%s\r\n", cmd);
    send(sock, full, strlen(full), 0);
}

int ftp_read_resp(int sock) {
    char buf[BUF_SIZE];
    int total = 0;
    while (1) {
        int n = recv(sock, buf + total, sizeof(buf) - total - 1, 0);
        if (n <= 0) return -1;
        total += n;
        if (total >= 2 && buf[total-2] == '\r' && buf[total-1] == '\n') {
            buf[total] = '\0';
            printf("%s", buf);
            int code;
            sscanf(buf, "%d", &code);
            return code;
        }
    }
}

void setup_active(int ctrl_sock, char *local_ip, int *listen_sock) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock, 1);

    socklen_t len = sizeof(addr);
    getsockname(sock, (struct sockaddr*)&addr, &len);
    int port = ntohs(addr.sin_port);

    struct in_addr ip;
    inet_pton(AF_INET, local_ip, &ip);
    unsigned char *b = (unsigned char*)&ip.s_addr;

    char cmd[BUF_SIZE];
    snprintf(cmd, sizeof(cmd), "PORT %d,%d,%d,%d,%d,%d", b[0], b[1], b[2], b[3], port/256, port%256);
    ftp_send_cmd(ctrl_sock, cmd);
    ftp_read_resp(ctrl_sock);
    *listen_sock = sock;
}

void ftp_list(int ctrl_sock, char *local_ip) {
    int lsock, ds, n;
    char buf[BUF_SIZE];
    setup_active(ctrl_sock, local_ip, &lsock);
    ftp_send_cmd(ctrl_sock, "LIST");
    ftp_read_resp(ctrl_sock);
    ds = accept(lsock, NULL, NULL);
    close(lsock);
    printf("Содержимое каталога:\n");
    while ((n = recv(ds, buf, sizeof(buf)-1, 0)) > 0) { buf[n]='\0'; printf("%s", buf); }
    close(ds);
    ftp_read_resp(ctrl_sock);
}

void ftp_get(int ctrl_sock, char *local_ip, char *file) {
    int lsock, ds, n;
    char buf[BUF_SIZE], cmd[BUF_SIZE];
    FILE *fp;
    setup_active(ctrl_sock, local_ip, &lsock);
    snprintf(cmd, sizeof(cmd), "RETR %s", file);
    ftp_send_cmd(ctrl_sock, cmd);
    ftp_read_resp(ctrl_sock);
    ds = accept(lsock, NULL, NULL);
    close(lsock);
    fp = fopen(file, "wb");
    while ((n = recv(ds, buf, sizeof(buf), 0)) > 0) fwrite(buf, 1, n, fp);
    fclose(fp); 
    close(ds);
    ftp_read_resp(ctrl_sock);
    printf("Скачан: %s\n", file);
}

void ftp_put(int ctrl_sock, char *local_ip, char *file) {
    int lsock, ds, n;
    char buf[BUF_SIZE], cmd[BUF_SIZE];
    FILE *fp = fopen(file, "rb");
    if (!fp) { printf("Файл не найден: %s\n", file); return; }
    setup_active(ctrl_sock, local_ip, &lsock);
    snprintf(cmd, sizeof(cmd), "STOR %s", file);
    ftp_send_cmd(ctrl_sock, cmd);
    ftp_read_resp(ctrl_sock);
    ds = accept(lsock, NULL, NULL);
    close(lsock);
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) send(ds, buf, n, 0);
    fclose(fp); 
    close(ds);
    ftp_read_resp(ctrl_sock);
    printf("Загружен: %s\n", file);
}

void ftp_cd(int ctrl_sock, char *dir) {
    char cmd[BUF_SIZE];
    snprintf(cmd, sizeof(cmd), "CWD %s", dir);
    ftp_send_cmd(ctrl_sock, cmd);
    ftp_read_resp(ctrl_sock);
}

int main(int argc, char *argv[]) {

    int ctrl_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in srv;
    struct hostent *he = gethostbyname(argv[1]);
    srv.sin_family = AF_INET;
    srv.sin_port = htons(21);
    srv.sin_addr = *((struct in_addr*)he->h_addr_list[0]);
    
    char local_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &srv.sin_addr, local_ip, INET_ADDRSTRLEN);
    connect(ctrl_sock, (struct sockaddr*)&srv, sizeof(srv));

    ftp_read_resp(ctrl_sock);

    char cmd[BUF_SIZE];
    snprintf(cmd, sizeof(cmd), "USER %s", argv[2]); ftp_send_cmd(ctrl_sock, cmd); ftp_read_resp(ctrl_sock);
    snprintf(cmd, sizeof(cmd), "PASS %s", argv[3]); ftp_send_cmd(ctrl_sock, cmd); ftp_read_resp(ctrl_sock);

    printf("Команды: ls, cd <dir>, get <file>, put <file>, quit\nftp> ");
    while (fgets(cmd, sizeof(cmd), stdin)) {
        cmd[strcspn(cmd, "\r\n")] = 0;
        if (strcmp(cmd, "ls") == 0) ftp_list(ctrl_sock, local_ip);
        else if (strncmp(cmd, "cd ", 3) == 0) ftp_cd(ctrl_sock, cmd+3);
        else if (strncmp(cmd, "get ", 4) == 0) ftp_get(ctrl_sock, local_ip, cmd+4);
        else if (strncmp(cmd, "put ", 4) == 0) ftp_put(ctrl_sock, local_ip, cmd+4);
        else if (strcmp(cmd, "quit") == 0) break;
        else printf("? %s\n", cmd);
        printf("ftp> ");
    }
    ftp_send_cmd(ctrl_sock, "QUIT");
    close(ctrl_sock);
    return 0;
}