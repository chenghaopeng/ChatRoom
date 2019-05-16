#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

long long getCurrentTime () {
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000) / 1000;
}

void getTime (char *t) {
	time_t ct;
	time (&ct);
	struct tm *p;
	p = localtime (&ct);
	sprintf (t, "%02d:%02d:%02d", p->tm_hour, p->tm_min, p->tm_sec);
}

#define SERV_PORT 45000

#define SIZE 1000

const int LOGIN = 1, EXIT = 2, GET = 3, SEND = 4, NOTHING = 5;

int prefixEquals (char *a, char *b) {
	if (strlen (b) > strlen (a))
		return 0;
	int i;
	for (i = 0; i < strlen (b); ++i) {
		if (a[i] != b[i]) {
			return 0;
		}
	}
	return 1;
}

int getOperation (char *opera) {
	if (prefixEquals (opera, ":login"))
		return LOGIN;
	if (prefixEquals (opera, ":exit"))
		return EXIT;
	if (prefixEquals (opera, ":get"))
		return GET;
	if (prefixEquals (opera, ":nothing"))
		return NOTHING;
	return SEND;
}

typedef struct {
	long long loginTime;
	long long recTime;
	char userName[SIZE];
	char info[SIZE];
} User;

User user;

void preWork () {
	system ("mkdir ./tmp");
	system ("rm ./tmp/*");
}

void printAction (char *s) {
	char t[9];
	getTime (t);
	printf("%s %s %s\n", t, user.info, s);
}

void getUserName (char *username, char *s) {
	int i = 6, j = 0;
	while (s[i] == ' ')
		i++;
	while (s[i] != '\0') {
		username[j] = s[i];
		j++;
		i++;
	}
	username[j] = '\0';
}

void sendMessage (char *s) {
	char t[9];
	getTime (t);
	char cmd[SIZE];
	sprintf (cmd, "echo \"%s %s: %s\" >> ./tmp/%lld", t, user.userName, s, getCurrentTime ());
	printAction("Say");
	#ifdef DEBUG
	printf ("%s\n", cmd);
	#endif
	system (cmd);
}

void getMessage (int connfd) {
	char buf [SIZE*10];
	memset(buf, '\0', sizeof(buf));
	char cmd [SIZE];
	sprintf (cmd, "ls ./tmp/ | sort | awk '{if($1>=%lld) print $1;}' | while read file_name; do cat ./tmp/$file_name; done", user.recTime);
	FILE *stream = popen(cmd, "r");
	fread(buf, sizeof(char), sizeof(buf), stream);
	pclose(stream);
	#ifdef DEBUG
	printf("%s", buf);
	printf("%d\n", strlen(buf));
	#endif
	if (strlen(buf) == 0) write (connfd, ":nothing", 9);
	else write (connfd, buf, strlen(buf)+1);
	user.recTime = getCurrentTime ();
}

int main (void) {
	preWork ();

	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
	int listenfd, connfd;
	char buf[SIZE];
	char str[INET_ADDRSTRLEN];
	int i, n;

	listenfd = socket (AF_INET, SOCK_STREAM, 0);

	bzero (&servaddr, sizeof (servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
	servaddr.sin_port = htons (SERV_PORT);

	bind (listenfd, (struct sockaddr *) &servaddr, sizeof (servaddr));

	listen (listenfd, 20);

	printf ("Accepting connections ...\n");
	while (1) {
		cliaddr_len = sizeof (cliaddr);
		connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &cliaddr_len);
		n = fork ();
		if (n == -1) {
			printf ("Fork error\n!");
			exit (1);
		} else if (n == 0) {
			close (listenfd);
			while (1) {
				n = read (connfd, buf, SIZE);
				if (n == 0) {
					printAction("Exit");
					break;
				}
				/*printf("PID %d received from %s at PORT %d\n", getpid(),
				   inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
				   ntohs(cliaddr.sin_port));

				   for (i = 0; i < n; i++)
				   buf[i] = toupper(buf[i]);
				   write(connfd, buf, n); */
				int op = getOperation (buf);
				if (op == LOGIN) {
					user.loginTime = user.recTime = getCurrentTime ();
					getUserName (user.userName, buf);
					sprintf (user.info, "PID %d IP %s PORT %d USERNAME %s", getpid (), inet_ntop (AF_INET, &cliaddr.sin_addr, str, sizeof (str)), ntohs (cliaddr.sin_port), user.userName);
					printAction("Log in");
				}
				if (op == SEND) {
					sendMessage (buf);
				}
				if (op == GET) {
					getMessage (connfd);
				}
			}
			close (connfd);
			exit (0);
		} else
			close (connfd);
	}
}
