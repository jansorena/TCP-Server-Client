#include "tcp.h"
#include "md5.h"
#include "util.h"
#include "decrypt.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define SIZE 1024
#define MD5_LEN 32

int main(void) {
    struct tcp_server_t server;
    tcp_server_create(&server, 10000);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int sock = tcp_server_accept(&server, &client_addr, &client_addr_len);

        printf("Cliente conectado con direccion IP: %s\n\n",inet_ntoa(client_addr.sin_addr));

        tcp_send(sock,"Conectado correctamente con el servidor\n",SIZE);

        char buffer_filename[SIZE];
        tcp_recv(sock, &buffer_filename, SIZE);

        int buffer_filesize;
        tcp_recv(sock, &buffer_filesize, SIZE);
        buffer_filesize = ntohl(buffer_filesize);

        char md5[MD5_LEN+1];
        tcp_recv(sock, &md5, MD5_LEN+1);

        printf("Archivo: %s    [%.2f Kb]\n",buffer_filename,(float)buffer_filesize/1024);
        printf("MD5 sum: %s\n",md5);
        
        FILE *fp;
        if ((fp = fopen(buffer_filename, "wb")) == NULL) {
            perror("No se pudo crear el archivo");
            exit(1);
        }

        printf("\n");
        tcp_recv_files(sock,fp,buffer_filesize);
        fclose(fp);

        char path_key[SIZE]; 
        printf("\nIngrese la direccion de la llave: ");
        scanf("%s",path_key);
        while(!check_file(path_key)){
            printf("Llave no encontrada. Intente nuevamente: ");
            scanf("%s",path_key);
        }

        printf("\nDesencriptando el archivo ...\n");
        f_decrypt(path_key,buffer_filename);
        
        printf("Comparando MD5 sum ...\n");
        char md5_cal[MD5_LEN+1];
        char *file_decrypt = remove_enc(buffer_filename);
        if (!CalcFileMD5(file_decrypt, md5_cal)){
            printf("No se pudo calcular MD5 sum del archivo");
            exit(1);
        }
        if (!strcmp(md5, md5_cal)) printf("Comparacion exitosa: %s\n\n",md5_cal);
        else printf("CUIDADO! MD5 sum es distinto\n\n");


        free(file_decrypt);
        tcp_close(sock);
        printf("\nEscuchando conexiones entrantes ...\n");
    }

    return 0;
}
