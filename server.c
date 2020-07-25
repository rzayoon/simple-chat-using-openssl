#include "common.h"

#define CERTFILE "server.pem"

#define MAX_CLIENTS 4
#define MAX_BUF 512
#define ID_SIZE 5


int uid = 0;


SSL *clients[MAX_CLIENTS];
int cli_cnt = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


SSL_CTX *setup_server_ctx(void){
	SSL_CTX *ctx;

	ctx = SSL_CTX_new(SSLv23_method( ));
	if(SSL_CTX_use_certificate_chain_file(ctx, CERTFILE) != 1)
		int_error("Error loading certificate from file");
	if (SSL_CTX_use_PrivateKey_file(ctx, CERTFILE, SSL_FILETYPE_PEM) != 1)
		int_error("Error loading private key from file");
	return ctx;
}

void send_msg(SSL* ssl, char* msg, int len) {
	int i;
	int err;
	pthread_mutex_lock(&mutex);
	for (i = 0; i < cli_cnt; i++) {
		err = SSL_write(clients[i], msg, len);
		if (err < 0){
			printf("Mesage sending error.");
			break;
		}
	}
	pthread_mutex_unlock(&mutex);
}

int do_server_loop(SSL *ssl){
	int err;
	char buf[MAX_BUF];
	do{
		
		err = SSL_read(ssl, buf, sizeof(buf));
		send_msg(ssl,buf, err);
	}
	while(err>0);
	return (SSL_get_shutdown(ssl) & SSL_RECEIVED_SHUTDOWN) ? 1 : 0;
}

void THREAD_CC menu_thread(void *arg){
	pthread_detach(pthread_self( ));
	char buf[MAX_BUF];
	fprintf(stdout, "----------------------menu---------------------\n1.!unum: checking number of user\n-----------------------------------------------\n");


	while(1){
		if(!fgets(buf, sizeof(buf), stdin))
			break;
		if(!strcmp(buf, "!unum\n")){
			pthread_mutex_lock(&mutex);
			fprintf(stdout, "the number of client connected is %d\n", cli_cnt);
			pthread_mutex_unlock(&mutex);
		}
	

	}


}

void THREAD_CC server_thread(void *arg){
	SSL *ssl = (SSL *)arg;
	pthread_detach(pthread_self( ));
	int i;
	char busymsg[] = "This server is full. Try later.\n";
	
	if(SSL_accept(ssl) <= 0)
	int_error("Error accepting SSL connection");
			
	pthread_mutex_lock(&mutex);
	int user = uid;

	if(cli_cnt + 1 > MAX_CLIENTS){
		pthread_mutex_unlock(&mutex);

		SSL_write(ssl, busymsg, sizeof(busymsg));
		SSL_clear(ssl);		
		SSL_free(ssl);
		ERR_remove_state(0);
		return;
	}

	clients[cli_cnt++] = ssl;
	uid++;
	pthread_mutex_unlock(&mutex);		



	fprintf(stderr, "SSL Connection opened with client%d\n",user);

	if (do_server_loop(ssl))
		SSL_shutdown(ssl);
	else
		SSL_clear(ssl);


	pthread_mutex_lock(&mutex);
	for (i=0; i<cli_cnt; i++){
		if(ssl == clients[i]){
			for(; i < cli_cnt - 1; i++){
				clients[i] = clients[i+1];
			}
			clients[cli_cnt-1] = NULL;
			break;
		}
	}
	cli_cnt--;
	pthread_mutex_unlock(&mutex);	

	fprintf(stderr, "SSL Connection closed with client%d\n",user);
	SSL_free(ssl);

	ERR_remove_state(0);

}

int main(int argc, char *argv[]){
	
	if (argc != 2){
		printf(" Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	BIO *acc, *client;
	SSL *ssl;
	SSL_CTX *ctx;
	THREAD_TYPE tid;

	init_OpenSSL( );
	seed_prng( );

	ctx = setup_server_ctx( );

	acc = BIO_new_accept(argv[1]);
	if (!acc)
		int_error("Error creating server socket");
	if (BIO_do_accept(acc) <= 0)
		int_error("Error binding server socket");

	system("clear");
	fprintf(stderr, "chat server port %s\n", argv[1]);
	
	THREAD_CREATE(tid, (void *)menu_thread, NULL);
	for(;;){
		if(BIO_do_accept(acc) <= 0)
			int_error("Error accepting connection");

		client = BIO_pop(acc);
		if(!(ssl = SSL_new(ctx)))
			int_error("Error creating SSL context");		

		SSL_set_bio(ssl, client, client);
		
		THREAD_CREATE(tid, (void *)server_thread, ssl);
	}

	SSL_CTX_free(ctx);
	BIO_free(acc);
	return 0;
}
