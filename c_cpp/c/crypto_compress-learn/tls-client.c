#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/if_packet.h>
#include <libssh/libssh.h>
#include <openssl/ssl.h>
#include <openssl/err.h>



#define FAIL    -1



int OpenConnection(const char *ip, int port)
{
    int fd;
    struct sockaddr_in addr = {0};

    fd = socket(PF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if ( connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0 ) {
        ;
    }
    return fd;
}



SSL_CTX* InitCTX(void)
{
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = TLSv1_2_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL ) {
        return 0;
    }
    return ctx;
}



void ShowCerts(SSL* ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL ) {
        printf("Server certificates:\n");
        line = X509_NAME_oneline( X509_get_subject_name(cert), 0, 0 );
        printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline( X509_get_issuer_name(cert), 0, 0 );
        printf("Issuer: %s\n", line);
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    } else {
        printf("Info: No client certificates configured.\n");
    }
}



int main()
{
    SSL_CTX *ctx;
    int server;
    SSL *ssl;
    char buf[1024] = "REQUEST";
    int bytes;
    
    SSL_library_init();
    ctx = InitCTX();

    server = OpenConnection("10.10.0.100", 10020);
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */

    if ( SSL_connect(ssl) < 0 ) {  /* perform the connection */
        ;
    }

    ShowCerts(ssl);        /* get any certs */
    SSL_write(ssl, buf, strlen(buf));   /* encrypt & send message */
    bytes = SSL_read(ssl, buf, strlen(buf)); /* get reply & decrypt */
    buf[bytes] = 0;

    printf("%s\n", buf);

    SSL_free(ssl);        /* release connection state */
    close(server);         /* close socket */
    SSL_CTX_free(ctx);        /* release context */
    return 0;
}