/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include <stdio.h>          /* stderr, stdout */
#include <stdlib.h>         /* mkfifo */
#include <netdb.h>          /* hostent struct, gethostbyname() */
#include <arpa/inet.h>      /* inet_ntoa() to format IP address */
#include <netinet/in.h>     /* in_addr structure */
#include <unistd.h>         /* ::close */
#include <sys/stat.h>       /* I_WUSR */
#include <string.h>         /* strcpy... */
#include <sys/socket.h>
#include "../radio_codes.h"
#include <fcntl.h>
#include <time.h>

#define BUFFSIZE 8192

static void die(RadioCodes::Status s, char *mess) {
        fprintf (stderr,"%d %s\n",(int)s,mess); 
        exit(1); 
}

void splitUrl(const char* url, char* host, int & port, char* path)
{
    char urlcopy[1024];
    strcpy(urlcopy,url);
    const char* hostcursor = strchr(url,'/')+2;
    if (hostcursor)
    {
        const char* pathcursor = strchr(hostcursor,'/');
        if (pathcursor)
        {
            strcpy(path,pathcursor);
            urlcopy[pathcursor-url] = '\0';
        }
        else
            strcpy(path,"/");
        const char* portcursor = strchr(hostcursor,':');
        if (portcursor)
        {
            port = atoi(urlcopy+(portcursor-url+1));
            urlcopy[portcursor-url] = '\0';
        }
        else
            port = 80;
        
        strcpy(host,urlcopy+(hostcursor-url));
    }
}


struct RadioDemux
{
    char curHeader [4096];
    char curMetadata [1024];
    char contentType [256];
    char fileTemplate [128];
    int port,sock;
    char* buffer;
    bool starting;
    void sendString (const char* str)
    {
        int len = strlen(str);
        if (::send(sock, str, len, 0) != len) {
            die(RadioCodes::SendMismatch,"Mismatch in number of sent bytes");
        }
    }
    int byteCount;
    int metadataInterval,bitrate;
    int metadataLen;
    FILE* file;
    FILE* dbgFile;

    enum State
    {
        State_Headers,
        State_Metadata,
        State_Audio
    } state;
    
    int httpStatus;
    void start(const char* url);
    void processMetadata(char* data, int len)
    {
        const char* title = "StreamTitle='";
        char* titlePtr = 0;
        for (int i=0; i < len; ++i,++data)
        {
            if (titlePtr)
            {
                if (strstr(data,"';") == data)
                {
                    *data = 0;
                    fprintf(stderr,"%d %s\n",RadioCodes::StreamTitleChanged,titlePtr);
                    break;                    
                }
            }
            if (strstr(data,title) == data)
            {
                data+=strlen(title);
                i+=strlen(title);
                titlePtr = data;
            }
        }
    }
    void processHeader(char* data)
    {
        fprintf(stderr,"%d %s\n",RadioCodes::Header,data);
        if (httpStatus == 0)
        {
            char str[256];
            char dummy[64];
            sscanf(data,"%s %d %s",dummy, &httpStatus,str);
            if (httpStatus >= 400)
            {
                die ((RadioCodes::Status)httpStatus,str);
            }
        }
        else
        {
            char key[32];
            char val[1024];
            strcpy(key,strtok(data,": "));
            strcpy(val,strtok(NULL,": "));
            if (httpStatus >= 300 && httpStatus  < 400)
            {
                if (!strcmp(key,"Location"))
                {
                    ::close(sock);
                    start(val);
                    exit (0);
                }
            }
            else if (!strcmp(key,"icy-metaint"))
            {
                metadataInterval = atoi(val);
                if (metadataInterval > BUFFSIZE)
                    buffer = (char*)malloc(metadataInterval);
            }
            else if (!strcmp(key,"icy-br"))
            {
                bitrate = atoi(val)*1024;
            }
            else if (!strcmp(key,"content-type"))
            {
                strcpy(contentType,val);
            }
        }
    }
    
    void process (char* data, int len)
    {
        switch (state)
        {
            
            case State_Headers:
            {
                data[len] = 0;
                strcat(curHeader,data);
                char* linebreak = strstr(curHeader,"\r\n");
                if (curHeader == linebreak)
                {
                    if (metadataInterval == 0)
                        die(RadioCodes::NoMetadataInterval,"No meta-data interval");
                    state = State_Audio;
                }
                else if (linebreak > curHeader)
                {
                    *linebreak = 0;
                    processHeader(curHeader);
                    curHeader[0] = 0;
                }
            }
            break;
            case State_Audio:
            {
                byteCount += len;
                if (!starting)
                {
                    starting = true;
                    char ext[5];
                    if (!strcmp(contentType,"audio/aac") || !strcmp(contentType,"audio/aacp"))
                        strcpy(ext,".aac");
                    else
                        strcpy(ext,".mp3");
                    
                    char filename [32] = "/var/tmp/radio-stream";
                    
//                    mkstemp(filename);
//                    strcpy(filename,fileTemplate);
//                    int f = mkstemp(filename);
                    strcat(filename,ext);
                    
//                    remove(filename);
                    remove (filename);
                    fprintf(stderr,"%d Creating file %s\n",RadioCodes::CreatingFile,filename);
                    if (mkfifo (filename,S_IREAD|S_IWRITE|S_IRUSR|S_IRGRP|S_IROTH) < 0)
                    die (RadioCodes::FailedToCreateNamedPipe,"Failed to create named pipe");
                    fprintf(stderr,"%d %s\n",RadioCodes::Ready,filename);
                    file = fopen (filename,"w");
                    if (file == NULL)
                        die (RadioCodes::FailedToOpenFile,"Failed to open file");
                    
//                    dbgFile = fopen("/webroot/dbgradio.mp3","w");
                }
                if (file)
                {
                    fprintf(stderr,"%d Writing %d bytes of audio\n",170,len);
                    fwrite(data,len,1,file);
//                    if (dbgFile)
//                    {
//                          fwrite(data,len,1,dbgFile);
                        fflush(dbgFile); 
//                    }   
                }            
                if (byteCount == metadataInterval)
                {
                    state = State_Metadata;
                    metadataLen = 0;
                    byteCount = 0;
                    curMetadata[0] = 0;
                }
            }
            break;
            case State_Metadata:
            {
                char nextChar = *data;
                if (byteCount == 0 && metadataLen == 0)
                {
                    metadataLen = 16L*nextChar;
                    if (metadataLen == 0)
                    {
                        fprintf(stderr,"%d No Metadata\n",RadioCodes::NoMetadata);
                        state = State_Audio;
                    }
                    else
                    {
                        curMetadata[metadataLen] = 0;
                    }
                }
                else 
                {
                    curMetadata[byteCount++] = nextChar;
                    if (byteCount == metadataLen)
                    {
                        processMetadata (curMetadata,metadataLen);
                        curMetadata[0] = 0;
                        metadataLen = 0;
                        byteCount = 0;
                        state = State_Audio;
                    }
                }
            }
            break;
        }   
    }
};
void RadioDemux::start(const char* u)
{
    starting = false;
    httpStatus = 0;
    curHeader[0] = 0;
    file = NULL;
    char hostname[1024] = "";
    char path[1024] = "";
    splitUrl(u,hostname,port,path);
//    fprintf(stderr,"%s %s %d %s\n",u,hostname,port,path);
    struct hostent *host;     /* host information */
    struct in_addr h_addr;    /* internet address */
    if ((host = gethostbyname(hostname)) == NULL) {
        die(RadioCodes::HostNotFound,"Host Not Found");
    }
    else
    {
        h_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
        fprintf(stderr,"%d Address found: %s\n", RadioCodes::AddressFound,inet_ntoa(h_addr));
        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
            die(RadioCodes::FailedToCreateSocket,"Failed to create socket");
        }
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));       /* Clear struct */
        server_addr.sin_family = AF_INET;                  /* Internet/IP */
        server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(h_addr));  /* IP address */
        server_addr.sin_port = htons((unsigned short)port);       /* server port */
        /* Establish connection */
        fprintf(stderr,"%d Connecting to %s:%d\n",RadioCodes::Connecting,inet_ntoa(h_addr),port);
        if (::connect(sock,
              (struct sockaddr *) &server_addr,
              sizeof(server_addr)) < 0) {
                  die(RadioCodes::FailedToConnectWithServer,"Failed to connect with server");
              }
              sendString("GET ");
              sendString(path);
              sendString(" HTTP/1.1\r\n");
              sendString("Accept: */*\r\n");
              sendString("User-Agent: Qtopia/4\r\n");
              sendString("x-audiocast-udpport: 4882\r\n");
              sendString("icy-metadata: 1\r\n");
              sendString("Host: ");
              sendString(hostname);
              sendString("\r\nConnection: Close\r\n\r\n");
              fprintf(stderr,"%d Request Sent\n",RadioCodes::RequestSent);
            
              char static_buffer [BUFFSIZE];
              buffer = static_buffer;
              
              state = State_Headers;
              byteCount = 0;
              metadataInterval = 0;
              time_t startTime;
              time (&startTime);
              int totalAudioBytes = 0;
              do
              {
                  int bytes = 0;
                  int len = 0;
                  switch (state)
                  {
                      case State_Headers:
                      case State_Metadata:
                          len  =1;
                          break;
                      case State_Audio:
                          len  = metadataInterval - byteCount;
                        break;
                  }
//                  if (len > BUFFSIZE)
//                      len = BUFFSIZE-1;
                  
//                  fprintf(stderr,"State:%d Len: %d Bytecount: %d mdi: %d\n",state,len,byteCount,metadataInterval);
                  for (int bc = 0; bc < len; bc+=bytes)
                  {
                    if ((bytes = ::recv(sock, buffer+bc, len-bc, 0)) < 1) {
                        die(RadioCodes::FailedToReceive,"Failed to receive bytes from server");
                    }
                  }
                  if (state == State_Audio)
                  {
                      /*
                      totalAudioBytes += len;
                      time_t curTime;
                      time(&curTime);
                      int diff = curTime - startTime;
                      fprintf(stderr,"Expected bitrate: %d; Actual Bitrate is %d\n",bitrate,abr);
                      if (diff > 0)
                      {
                        int abr = totalAudioBytes / diff;
                        fprintf(stderr,"Expected bitrate: %d; Actual Bitrate is %d\n",bitrate,abr);
                      }
                      */
                  }
  //                fprintf(stderr,"106 Data\n");z
                  process(buffer, len);
                    
              } while (true);
    }
}
int main (int argc, char** argv)
{
    if (argc < 2)
    {
        die(RadioCodes::BadRequest,"Usage: radio_demux url [fileTemplate]\n");
    }
    else
    {
        RadioDemux rd;
        if (argc > 2)
            strcpy(rd.fileTemplate,argv[2]);
        else
            strcpy(rd.fileTemplate,"/tmp/radio-stream");
        
        rd.start (argv[1]);        
    }
    
    
    
    
}

