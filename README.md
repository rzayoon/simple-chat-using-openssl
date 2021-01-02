# simple-chat-using-openssl
simple chat server and client using openssl

1. prerequisited
libssl1.0-dev

sudo apt install -y make libssl1.0-dev
sudo apt autoremove

c compilder
sudo apt install gcc

make 
sudo apt install make


2. Create Key

rootCA, root.pem

/usr/bin/openssl req -newkey rsa:1024 -sha1 -keyout rootkey.pem -out rootreq.pem -config root.cnf
/usr/bin/openssl x509 -req -in reetreq.pem -sha1 -extfile root.cnf -extensions certificate_extenstions \
  -signkey rootkey.pem -out rootcert.pem
/bin/cat rootcert.pem rootkey.pem > root.pem


serverCA, sign it with rootCA
/usr/bin/openssl req -newkey rsa:1024 -sha1 -keyout serverCAkey.pem -out serverCAreq.pem -config serverCA.cnf
/usr/bin/openssl x509 -req -in serverCAreq.pem -sha1 -extfile serverCA.cnf -extenstions -CA root.pem -CAkey root.pem \
  -CAcreateserial -out serverCAcert.pem
/bin/cat serverCAcert.pem serverCAkey.pem rootcert.pem > serverCA.pem

server's certi and sign it with serverCA
/usr/bin/openssl req -newkey rsa:1024 -sha1 -keyout serverkey.pem -out serverreq.pem -config server.cnf -reqexts req_extenstions
/usr/bin/openssl x509 -req -in serverreq.pem -sha1 -extfile server.cnf -extensions certificate_extensions -CA serverCA.pem \
  -CAkey serverCA.pem -CAcreateserial -out servercert.pem
/bin/cat servercert.pem serverkey.pem serverCAcert.pem rootcert.pem > server.pem

client's certi and sign it with rootCA
/usr/bin/openssl req -newkey rsa:1024 -sha1 -keyout client.pem -out clientreq.pem -config client.cnf -reqexts req_extensions
/usr/bin/openssl x509 -req -in clientreq.pem -sha1 -extfile client.cnf -extensions certificate_extensions -CA root.pem -CAkey root.pem \
  -CAcreateserial -out clientcert.pem
/bin/cat clientcert.pem clientkey.pem rootcert.pem > client.pem

now you've got root.cnt, serverCA.cnf, server.cnf, client.cnf and KEY
 
