#ifndef FIND_HEADER_H
#define FIND_HEADER_H

#include "../fileReader/fileReader.h"
#include "../filePatterns/patternReader.h"

struct SSLOptions{
	int forceSSL;
	int useSSL;
	int hasCertificate;
	char* sslCertificate;
};

struct TcpOptions{
	int localHost;
	char* hostAdress;
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
};


// get the options of the file between *** ***
int readHeaderData(struct File file, unsigned long int start, unsigned long int end);
struct HeaderOptions getHeaderOptions(struct File file);

#endif