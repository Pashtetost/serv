#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <algorithm>
#include <fcntl.h>
#include <set>

using namespace std;
char* processing(char* data, int len);
char* itoa(int value, char* result);

int main(int argc, char *argv[])
{
      int listenfd, udpfd;
      int bytes_ready;
      char mesg[1024];
       fd_set rset;
      socklen_t len;
      const int on = 1;
      struct sockaddr_in addr;

      listenfd = socket(AF_INET, SOCK_STREAM, 0);
      if(listenfd < 0)
        {
            perror("socket");
            exit(1);
        }
      fcntl(listenfd, F_SETFL, O_NONBLOCK);
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(INADDR_ANY);
      addr.sin_port = htons(3164);
      setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
     if( bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)))
      {
              perror("bind");
              exit(2);
      }
     listen(listenfd, 2);

     set <int> clients;
     clients.clear();

      /* создание сокета UDP */
      udpfd = socket(AF_INET, SOCK_DGRAM, 0);
     if (bind(udpfd, (struct sockaddr *)&addr, sizeof(addr)))
      {
              perror("bind");
              exit(2);
      }

     while(1)
      {
        FD_ZERO(&rset);
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);
        for(set<int>::iterator it = clients.begin(); it != clients.end(); it++)
                    FD_SET(*it, &rset);

        int mx=max(udpfd,max(listenfd,*max_element(clients.begin(),clients.end())));
        if ((select(mx+1, &rset, NULL, NULL, NULL)) <= 0)
        {
                perror("select error");
                exit(3);

        }
        if (FD_ISSET(listenfd, &rset))
        {
             //cout<<"Добавили клиента"<<endl;
            int sock = accept(listenfd, NULL, NULL);
                       if(sock < 0)
                       {
                           perror("accept");
                           exit(3);
                       }
                       fcntl(sock, F_SETFL, O_NONBLOCK);
                       clients.insert(sock);
         }
        for(set<int>::iterator it = clients.begin(); it != clients.end(); it++)
              {
                  if(FD_ISSET(*it, &rset))
                  {
                      //cout<<"Поступили данные от клиента, читаем их"<<endl;
                      bytes_ready = recv(*it, mesg, 1024, 0);

                      if(bytes_ready <= 0)
                      {
                          // Соединение разорвано, удаляем сокет из множества
                          close(*it);
                          clients.erase(*it);
                          continue;
                      }
                      // Отправляем данные обратно клиенту
                      char *b=processing(mesg,strlen(mesg));
                      cout<<b<<endl;
                      send(*it, b, strlen(b), 0);
                      memset(mesg, 0, sizeof(mesg));
                  }
              }
         if (FD_ISSET(udpfd, &rset))
         {
            len = sizeof(addr);
            bytes_ready = recvfrom(udpfd, mesg, 1024, 0, (struct sockaddr *)&addr, &len);
            char *b=processing(mesg,strlen(mesg));
            sendto(udpfd, b,  strlen(b), 0, (struct sockaddr *)&addr, sizeof(addr));
            memset(mesg, 0, sizeof(mesg));
         }
       }
    return 0;
}

char* processing (char *data,int len)
{
    cout<<"Обработка данных"<<endl;

    int summ=0;
    char *buf=new char [len];
    memset(buf, 0, len);
    set <int> *mass=new set<int>;
    mass->clear();

    int k=0;
    cout<<data<<endl;
    for(int i=0;i<len;i++)
    {
                if(data[i]>='0' && data[i]<='9')
                {
                   buf[k]=data[i];
                   k++;
                   if(data[i+1]>='0' && data[i+1]<='9')
                   {
                       continue;
                   }else
                   {
                       //cout<<buf<<"в массив"<<endl;
                       mass->insert(atoi(buf));
                        memset(buf, 0, len);
                       k=0;
                   }
                }

    }
    char dat[1024];
     memset(dat, 0, 1024);
    for(set<int>::iterator it = mass->begin(); it != mass->end(); it++)
    {
       summ+=*it;
       itoa(*it,dat);
       strcat(buf,dat);
       strcat(buf," ");
    }
    strcat(buf,"\n");
    itoa(summ,dat);
    strcat(buf,dat);
    cout<<buf<<endl;
    return  buf;
}
char* itoa(int value, char* result) {


        char* ptr = result, *ptr1 = result, tmp_char;
        int tmp_value;

        do {
            tmp_value = value;
            value /= 10;
            *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * 10)];
        } while ( value );

        // Apply negative sign
        if (tmp_value < 0) *ptr++ = '-';
        *ptr-- = '\0';
        while(ptr1 < ptr) {
            tmp_char = *ptr;
            *ptr--= *ptr1;
            *ptr1++ = tmp_char;
        }
        return result;
    }
