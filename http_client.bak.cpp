#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <limits.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string>
#include <iostream>
using namespace std;
int main(int argc, char *argv[])
{
  int sockfd;
  char buffer[1024];
  struct sockaddr_in server_addr;
  struct hostent *host;
  int portnumber,nbytes;
  char host_addr[256];
  string host_file;
  char local_file[256];
  FILE * fp;
  char request[1024];
  int send, totalsend;
  int i;
  char * pt;

  if(argc!=2)
  {
      fprintf(stderr,"Usage:%s web-address\a\n",argv[0]);
      exit(1);
  }
  portnumber=80;
  strcpy(host_addr,argv[1]);
  string host_name;
  string http_prefix = "http://";
  string https_prefix = "https://";
  host_name.assign(argv[1]);
  if(host_name.compare(0, http_prefix.length(), http_prefix) == 0)
  {
    host_name = host_name.substr(http_prefix.length(), string::npos);
  }
  else if(host_name.compare(0, https_prefix.length(), https_prefix) == 0)
  {
    host_name = host_name.substr(https_prefix.length(), string::npos);
  }
  else
  {
    fprintf(stderr, "url (%s) format error.\n", host_name.c_str());
    return 0;
  }
  int pos = host_name.find('/', 0);
  if(pos != string::npos)
  {
    host_file = host_name.substr(pos, string::npos);
    host_name = host_name.substr(0, pos);
  }
  else
  {
    host_file = "";
  }
  cout << "http host name: " << host_name << endl; 
  cout << "http host file: " << host_file << endl;
  if((host=gethostbyname(host_name.c_str()))==NULL)/*取得主机IP地址*/
  {
      fprintf(stderr,"Gethostname error, %s\n", strerror(errno));
      exit(1);
  }
  if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)/*建立SOCKET连接*/
  {
      fprintf(stderr,"Socket Error:%s\a\n",strerror(errno));
      exit(1);
  }
  //设置超时时间
  struct timeval timeo = {3,0};
  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));

  /* 客户程序填充服务端的资料 */
  bzero(&server_addr,sizeof(server_addr));
  server_addr.sin_family=AF_INET;
  server_addr.sin_port=htons(portnumber);
  server_addr.sin_addr=*((struct in_addr *)host->h_addr);
  string str_ip = inet_ntoa(server_addr.sin_addr);
  cout << "server addr: " << str_ip << endl;
  /* 客户程序发起连接请求 */
  if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)/*连接网站*/
  {
      fprintf(stderr,"Connect Error:%s\a\n",strerror(errno));
      exit(1);
  }
  #if 1
  sprintf(request, "GET /%s HTTP/1.1\r\nAccept: */*\r\nAccept-Language: zh-cn\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\nHost: %s:%d\r\nConnection: Close\r\n\r\n", host_file.c_str(), str_ip.c_str(), portnumber);
  #endif

  printf("%s", request);/*准备request，将要发送给主机*/

  /*取得真实的文件名*/
  strcpy(local_file, "index.html");
  /*发送http请求request*/
  send = 0;totalsend = 0;
  nbytes=strlen(request);
  while(totalsend < nbytes) 
  {
      send = write(sockfd, request + totalsend, nbytes - totalsend);
      if(send==-1) {printf("send error!%s\n", strerror(errno));exit(0);}
      totalsend+=send;
      printf("%d bytes send OK!\n", totalsend);
  }
  /* 连接成功了，接收http响应，response */
  string str_response;
  str_response.clear();
  while((nbytes=read(sockfd,buffer,1024)) > 0)
  {
      str_response.append(buffer, nbytes);
  }
  cout << "response: " << str_response << endl;
  //fclose(fp);
  /* 结束通讯 */
  close(sockfd);
  exit(0);
}