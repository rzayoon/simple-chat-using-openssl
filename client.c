#include "common.h"

#define CERTFILE "client.pem"

#define MAX_BUF 512
#define INFO_SIZE 25
#define NAME_SIZE 10

char name[NAME_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

SSL_CTX *setup_client_ctx(void){
	SSL_CTX *ctx;

	ctx = SSL_CTX_new(SSLv23_method( ));
	if (SSL_CTX_use_certificate_chain_file(ctx, CERTFILE) != 1)
		int_error("Error loading certificate from file");
	if (SSL_CTX_use_PrivateKey_file(ctx, CERTFILE, SSL_FILETYPE_PEM) != 1)
		int_error("Error loading private key from file");
	return ctx;
}



void THREAD_CC send_thread(void* arg)
{
	SSL* ssl = (SSL *)arg;
	char name_msg[MAX_BUF];
	char buf[MAX_BUF - NAME_SIZE];
	int err;

	while(1){
		if(!fgets(buf, sizeof(buf), stdin))
			break;
		if(!strcmp(buf, "!exit\n")){
			break;	
		}

		sprintf(name_msg, "[%s]: %s", name, buf);
		err = SSL_write(ssl, name_msg, sizeof(name_msg));
		if (err <= 0)
			break;
	}
}	

void THREAD_CC recv_thread(void* arg)
{
	SSL* ssl = (SSL *)arg;
	pthread_detach(pthread_self( ));
	int err;
	char buf[MAX_BUF];
	

	do {
		
		err = SSL_read(ssl, buf, sizeof(buf));
		fprintf(stdout, "%s", buf);
		
	} while (err > 0);
}

int main(int argc, char *argv[]){
	
	if (argc != 4){
		printf(" Usage : %s <ip> <port> <name>\n", argv[0]);
		exit(1);
	}
	if (sizeof(argv[3]) > NAME_SIZE){
		printf(" Too long name. Use another name.\n");
		exit(1);
	}

	THREAD_TYPE rcv, snd;
	void* thread_return;
	BIO *conn;
	SSL *ssl;
	SSL_CTX *ctx;
	char ser_info[INFO_SIZE];
	
	init_OpenSSL( );
	seed_prng( );

	ctx = setup_client_ctx( );
	
	sprintf(ser_info, "%s:%s", argv[1], argv[2]);
	sprintf(name, "%s", argv[3]);
	conn = BIO_new_connect(ser_info);

	if (!conn)
		int_error("Error creating connection BIO");

	if (BIO_do_connect(conn) <= 0)
		int_error("Error connecting to remote machine");

	if(!(ssl = SSL_new(ctx)))
		int_error("Error creating an SSL context");
	SSL_set_bio(ssl, conn, conn);
	if (SSL_connect(ssl) <= 0)
		int_error("Error connecting SSL object");

	system("clear");
	fprintf(stderr, "SSL Connection opened. your name is [%s]\n", argv[3]);
	fprintf(stderr, "When you want exit, enter \"!exit\"\n");

	THREAD_CREATE(rcv, (void*)recv_thread, ssl);
	THREAD_CREATE(snd, (void*)send_thread, ssl);
	pthread_join(snd, &thread_return);

	fprintf(stderr, "SSL Connection closed\n");
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	return 0;
}
