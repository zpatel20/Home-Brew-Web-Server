//citations: http://stackoverflow.com/questions/409087/creating-a-web-server-in-pure-c
// http://blog.abhijeetr.com/2010/04/very-simple-http-server-writen-in-c.html
//https://dzone.com/articles/web-server-c
//https://www.youtube.com/watch?v=Q1bHO4VbUck


//include statements 
#include <fnmatch.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>

#define BACKLOG (10)

void serve_request(int);


//the original string of request and index
char * request_str = "HTTP/1.0 200 OK\r\n"
        "Content-type: text/html; charset=UTF-8\r\n\r\n";

char * index_hdr = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"><html>"
        "<title>Directory listing for %s</title>"
"<body>"
"<h2>Directory listing for %s</h2><hr><ul>";

char * index_body = "<li><a href=\"%s\">%s</a>";

char * index_ftr = "</ul><hr></body></html>";

char* parseRequest(char* request) 
{
  //assume file paths are no more than 256 bytes + 1 for null.
  char *buffer = malloc(sizeof(char)*257);
  memset(buffer, 0, 257);

  if(fnmatch("GET * HTTP/1.*",  request, 0)) return 0;

  sscanf(request, "GET %s HTTP/1.", buffer);
  return buffer;
}

//checks what kind of demand was made in the browser 
//citation: http://www3.nd.edu/~dthain/courses/cse30341/spring2009/project4/source/request.c
//citation: http://sanjeewamalalgoda.blogspot.com/2010/05/java-micro-edition-code-for-logarithams.html
char * request_handle (char * filename)
{
  char store[256];
  char * hold;
  char type[256];

  strncpy(store, filename, 256);

  hold = strtok(store, ".");

  strncpy(type, hold, 256);

  while(hold != NULL)
    {
      strncpy(type,hold, 256);
      hold = strtok(NULL, ".");
    }

  if(strcmp (type, "html") == 0)
    return "text/html";
  else if (strcmp (type, "pdf") == 0)
    return "application/html";
  else if (strcmp (type, "gif") == 0)
    return "image/gif";
   else if (strcmp (type, "png") == 0)
    return "image/png";
  else if (strcmp (type, "jpg") == 0 || strcmp(type, "jpeg") == 0)
    return "image/jpeg";
  else if (strcmp (type, "ico") == 0)
    return "image/x-icon";
  else
    return "text/html";
}

//***** pages for error checking ******
//202 when everything is ok
//error checking code citation: http://www.tenouk.com/visualcplusmfc/visualcplusmfc32e.html
char * good202 = "HTTP/1.0 200 OK\r\n"
  "Content-type: %s; charset = UTF-8\r\n\r\n";
//404 when page is not found
char * error404 = "HTTP/1.0 404 Not Found\r\n"
  "Content-Type: text/html; charset=UTF-8\r\n\r\n %s";
//error message image
//picture of cat comes up with 404 error
//citation: google image from this website: https://c2.staticflickr.com/8/7022/6540669737_7527a5de13_b.jpg
char * error404jpg = "<!DOCTYPE html>\n"
  "<html>\n"
  "<body>\n"
  "\n"
  "<h1 style=\"color:red\">ERROR ERROR ERROR ERROR ERROR\n: 404 Requested File Not Found</h1>\n"
  "<img src=\"https://c2.staticflickr.com/8/7022/6540669737_7527a5de13_b.jpg\" alt=\"Mountain View\" style=\"width:640px;height:428px;\">\n"
  "\n"
  "</body>\n"
  "</html>";

char * get_good202 (char * file)
{
  char * save = malloc(512);
  sprintf(save, good202, request_handle(file));
  return save;
}

char * get_error404 (char * file)
{
  char * save = malloc(512);
  sprintf(save, error404, error404jpg);
  return save;
}

int hasIndexHtml(char *chkFile)
{
  struct stat sb;
  int x = stat(chkFile, &sb);
  if(x == 0)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

//checks to see if directory exists
int directoryExists (char * chkFile)
{
  struct stat sb;
  stat(chkFile, &sb);
  return S_ISDIR(sb.st_mode);
}

//checks to see if files exists
int fileExists (char *chkFile)
{
  struct stat sb;
  stat(chkFile, &sb);
  return S_ISREG(sb.st_mode);
}

char * findDirectory(char * file)
{
  char * save = malloc(512 * 128);

  sprintf(save, request_str);
  sprintf(save + strlen(save), index_hdr, file, file);

  DIR * pdir;
  struct dirent *epdir;
  pdir = opendir(file);
  char filename[512];

  if(pdir != NULL)
    {
      while((epdir = readdir(pdir)) != NULL)
        {
          sprintf(filename, "%s%s", filename, epdir->d_name);

          if(fileExists(filename))
          	            {
              sprintf(save + strlen(save), index_body, filename+1, epdir->d_name);
            }
          else if(directoryExists(filename))
            {
              sprintf(save + strlen(save), index_body, filename+1, epdir->d_name);
            }
        }
    }

  (void) closedir (pdir);
  sprintf(save + strlen(save), index_ftr);
  return save;
}


//citation: https://tia.mat.br/posts/2014/10/06/life_of_a_http_request.html
void serve_request(int client_fd){
  int read_fd;
  int bytes_read;
  int file_offset = 0;
  char client_buf[4096];
  char send_buf[4096];
  char filename[4096];
  char * requested_file;
  char * ind = "index.html";
  memset(client_buf,0,4096);
  memset(filename,0,4096);
  
  while(1)
  {
    file_offset += recv(client_fd,&client_buf[file_offset],4096,0);
    if(strstr(client_buf,"\r\n\r\n"))
      break;
  }
  requested_file = parseRequest(client_buf);
  
  // take requested_file, add a . to beginning, open that file
  filename[0] = '.';
  strncpy(&filename[1],requested_file,4095);

  if((read_fd=open(filename,0,0)) != -1)
  {
    if(fileExists(filename))
       {
        send(client_fd, get_good202(filename), strlen(get_good202(filename)), 0);

        while(1)
          {
            bytes_read = read(read_fd, send_buf, 4096);

            if(bytes_read == 0)
              {
                break;
              }

            send(client_fd, send_buf, bytes_read, 0);
          }
      }
    else if(directoryExists(filename))
      {
        char store[1024];

        sprintf(store, "%s%s", filename, ind);

        if(hasIndexHtml(store + 2))
          {
            read_fd = open(store+2, 0, 0);
            send(client_fd, get_good202(store + 2), strlen(get_good202(store + 2)), 0);
            while(1)
              {
                bytes_read = read(read_fd, send_buf, 4096);
                if(bytes_read == 0)
                  {
                    break;
                  }
                send(client_fd, send_buf, bytes_read, 0);
              }
          }
           else
          {
            send(client_fd, findDirectory(filename), strlen(findDirectory(filename)), 0);
          }
      }
  }
  else
    {
      send(client_fd, get_error404(filename), strlen(get_error404(filename)), 0);
    }

  close(read_fd);
  close(client_fd);
  return;
}

//copied from piazza:https://piazza.com/class/irnt3005sy32t5?cid=275
//concurrency function
void * thread(void *arg)
{
        int sock =(*((int*)arg));
        free(arg);
        pthread_detach(pthread_self());
        serve_request(sock);
        //close(*(int*)vargp);
        return NULL;
}

int main(int argc, char** argv) 
{

    int retval;

    int port = atoi(argv[1]);

    /* Create a socket to which clients will connect. */
    int server_sock = socket(AF_INET6, SOCK_STREAM, 0);
    if(server_sock < 0) 
    {
        perror("Creating socket failed");
        exit(1);
    }

    int reuse_true = 1;
    retval = setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_true,
                        sizeof(reuse_true));
    if (retval < 0)
     {
        perror("Setting socket option failed");
        exit(1);
     }


    struct sockaddr_in6 addr;   
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port); 
    addr.sin6_addr = in6addr_any; 

    retval = bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
    if(retval < 0) 
    {
        perror("Error binding to port");
        exit(1);
    }

    retval = listen(server_sock, BACKLOG);
    if(retval < 0) 
    {
        perror("Error listening for connections");
        exit(1);
    }

    while(1) 
    {

        int* sock = (int*) malloc(sizeof(int));
        
        struct sockaddr_in remote_addr;
        unsigned int socklen = sizeof(remote_addr);

       (*sock) = accept(server_sock, (struct sockaddr*) &remote_addr, &socklen);
        if(sock < 0) 
        {
            perror("Error accepting connection");
            exit(1);
        }

        		//copied from piazza: https://piazza.com/class/irnt3005sy32t5?cid=275
        		//concurrency function
                pthread_t tid;
                pthread_create(&tid,NULL,thread,(void*)sock);

    }

    close(server_sock);
}


