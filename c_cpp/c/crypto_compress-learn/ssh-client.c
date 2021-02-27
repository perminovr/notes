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




int main ()
{
    // from https://rtfm.co.ua/c-libssh-primer-ssh-klienta/

    ssh_session session;
    int port = 22;
    int verbosity = SSH_LOG_WARNING;
    int connection;

    session = ssh_new();
    if (session == NULL) 
        exit(-1);

    ssh_options_set(session, SSH_OPTIONS_HOST, "10.10.0.100");
    ssh_options_set(session, SSH_OPTIONS_USER, "root");
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(session, SSH_OPTIONS_PORT, &port);

    connection = ssh_connect(session);

    if (connection == SSH_OK) {
        int state = ssh_is_server_known(session);

        switch (state) {
            case SSH_SERVER_KNOWN_OK: {
                char *pswd = "PASSWORD";
                if (ssh_userauth_password(session, "root", pswd) == SSH_AUTH_SUCCESS) {
                    ssh_channel channel = ssh_channel_new(session);

                    if ( ssh_channel_open_session(channel) == SSH_OK ) {                        

                        char buffer[1024];
                        int nbytes;
                        if (ssh_channel_request_exec(channel, "ps aux | grep ssh") == SSH_OK) {
                            nbytes = ssh_channel_read(channel, buffer, 1024, 0);
                            printf("%s\n", buffer);
                        }

                    }
                    ssh_channel_send_eof(channel);
                    ssh_channel_close(channel);
                    ssh_channel_free(channel);
                }
            } break;
            default: {

            } break;
        }
    }

    ssh_disconnect(session);
    ssh_free(session);

    return 0;
}