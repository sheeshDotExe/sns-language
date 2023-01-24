#ifndef HEADER_STRUCTS_H
#define HEADER_STRUCTS_H

struct SSLOptions{
	int forceSSL;
	int useSSL;
	int hasCertificate;
	char* sslCertificate;
};

struct TcpOptions{
	int localHost;
	char* hostAddress;
	unsigned int port;
	int releaseMode;
	unsigned long int connectionQueue;
	float connectionTimeoutWait;

	struct SSLOptions sslOptions;
};

struct resourcePath{
	char*format;
	char*path;
};

struct HeaderOptions{
	long unsigned int headerStart;
	long unsigned int headerEnd;
	char* rawHeaderData;

	struct TcpOptions tcpOptions;
	struct resourcePath staticFiles;
	struct resourcePath* staticFormattedFiles;
	unsigned int formattedFiles;
};

#endif