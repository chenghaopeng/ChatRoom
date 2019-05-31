#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define SERV_PORT 45000

#define SIZE 1000

const int LOGIN = 1, EXIT = 2, GET = 3, SEND = 4, NOTHING = 5, ERROR = 6;

int sockfd;
int logined = 0;
char rec[SIZE];

int
prefixEquals (char *a, char *b)
{
  if (strlen (b) > strlen (a))
    return 0;
  int i;
  for (i = 0; i < strlen (b); ++i)
    {
      if (a[i] != b[i])
	{
	  return 0;
	}
    }
  return 1;
}

int
getOperation (char *opera)
{
  if (strlen (opera) == 0)
    return GET;
  if (prefixEquals (opera, ":login"))
    return LOGIN;
  if (prefixEquals (opera, ":exit"))
    return EXIT;
  if (prefixEquals (opera, ":get"))
    return GET;
  if (prefixEquals (opera, ":nothing"))
    return NOTHING;
  if (prefixEquals (opera, ":error"))
    return ERROR;
  return SEND;
}

void
login (char *op)
{
  if (logined)
    {
      printf ("You have already loged in.\n");
      return;
    }
  write (sockfd, op, SIZE);
  char rec[SIZE];
  int n = read (sockfd, rec, SIZE);
  rec[n] = '\0';
  if (prefixEquals (rec, ":nothing"))
    {
      logined = 1;
      printf ("Log in successfully.\n");
    }
  else
    {
      printf ("Fail to log in. %s\n", rec);
    }
}

void
getMessage ()
{
  if (!logined)
    {
      printf ("You have not loged in.\n");
      return;
    }
  write (sockfd, ":get", SIZE);
  char rec[SIZE * 10];
  int n = read (sockfd, rec, SIZE * 10);
  rec[n] = '\0';
  if (!prefixEquals (rec, ":nothing"))
    {
      printf ("\n\n\n\n%s", rec);
    }
}

void
sendMessage (char *op)
{
  if (!logined)
    {
      printf ("You have not loged in.\n");
      return;
    }
#ifdef DEBUG
  printf ("send message: %s\n", op);
#endif
  write (sockfd, op, SIZE);
  getMessage ();
}

void
readIn (char *s)
{
  memset (s, 0, sizeof (s));
  scanf ("%[^\n]", s);
  getchar ();
}

void
Main ()
{
  char opera[SIZE];
  int op;

  while (1)
    {
      printf ("\n\nInput Command or Chat\n");
      readIn (opera);
      op = getOperation (opera);
#ifdef DEBUG
      printf ("text: %s   opera: %d\n", opera, op);
#endif
      if (op == EXIT)
	break;
      if (op == LOGIN)
	{
	  login (opera);
	}
      if (op == SEND)
	{
	  sendMessage (opera);
	}
      if (op == GET)
	{
	  getMessage ();
	}
    }
}

int
main ()
{
  struct sockaddr_in serv, cli;

  sockfd = socket (PF_INET, SOCK_STREAM, 0);

  bzero (&serv, sizeof (serv));

  serv.sin_family = PF_INET;
  serv.sin_port = htons (SERV_PORT);
  serv.sin_addr.s_addr = inet_addr ("127.0.0.1");

  connect (sockfd, (struct sockaddr *) &serv, sizeof (struct sockaddr));

  Main ();

  close (sockfd);
  return 0;
}
