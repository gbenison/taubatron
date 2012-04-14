/*
 * GCB 6apr12
 *
 * C version of CGI-trampoline code.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

static const char* socket_name  = "/tmp/test.sock";
static const char* request_path = "/taubatron";

static int
open_socket_fd(const char *path)
{
  int result = socket(AF_UNIX, SOCK_STREAM, 0);
  if (result < 1) { perror("socket"); exit(1); }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family=AF_UNIX;
  strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

  connect(result, (const struct sockaddr*)(&addr), sizeof(addr));

  return result;
}

static FILE*
open_socket_file(const char *path)
{
  int socket_fd = open_socket_fd(path);
  
  if (errno != 0)
    {
      perror(path);
      exit(1);
    }

  FILE* result = fdopen(socket_fd, "w");
  if (result == NULL) { perror(path); exit(1); }

  return result;
}

int
main()
{
  int content_length = 0;
  if (getenv("CONTENT_LENGTH"))
    content_length = atoi(getenv("CONTENT_LENGTH"));

  /* Read content from stdin, if any */
  char buf[content_length];
  int actual_content_length = fread(buf, 1, content_length, stdin);

  FILE *upstream_socket = open_socket_file(socket_name);
  setvbuf(upstream_socket, NULL, _IONBF, 0);

  /* Construct request and send to upstream. */
  fprintf(upstream_socket, "%s %s %s\r\n",
	  getenv("REQUEST_METHOD"),
	  request_path,
	  getenv("SERVER_PROTOCOL"));
  if (content_length > 0)
    fprintf(upstream_socket, "Content-length: %d\r\n", actual_content_length);
  if (getenv("CONTENT_TYPE"))
    fprintf(upstream_socket, "Content-type: %s\r\n",
	    getenv("CONTENT_TYPE"));

  /* End headers; output body */
  fprintf(upstream_socket, "\r\n");
  fwrite(buf, 1, actual_content_length, upstream_socket);
  fflush(upstream_socket);

  /* Read response */
  #define BUF_SIZE 512
  char response_buf[BUF_SIZE];

  int f_status_line = 0;
  while (!feof(upstream_socket))
    {
      int n_read = fread(response_buf, 1, BUF_SIZE, upstream_socket);
      int offset = 0;
      if (!f_status_line)
	{
	  for (; offset < n_read; ++offset)
	    if (response_buf[offset] == '\n')
	      {
		f_status_line = 1;
		break;
	      }
	}
      fwrite(&response_buf[offset], 1, n_read - offset, stdout);
    }

  return 0;
}
