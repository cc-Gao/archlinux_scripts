#include "WeatherGet.h"
#include "cJSON.h"


//struct addrinfo
//{
//    int ai_flags;			/* Input flags.  */
//    int ai_family;		/* Protocol family for socket.  */
//    int ai_socktype;		/* Socket type.  */
//    int ai_protocol;		/* Protocol for socket.  */
//    socklen_t ai_addrlen;		/* Length of socket address.  */
//    struct sockaddr* ai_addr;	/* Socket address for socket.  */
//    char* ai_canonname;		/* Canonical name for service location.  */
//    struct addrinfo* ai_next;	/* Pointer to next in list.  */
//};

int sock_init(const char* server, unsigned short port)
{
    int fd = -1;

    struct addrinfo hints;
    struct addrinfo* result, * rp;
    char server_port[32];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    snprintf(server_port, sizeof(server_port), "%u", port);
    int ret = getaddrinfo(server, server_port, &hints, &result);
    if (ret) {
        printf("getaddrinfo: %s\n", gai_strerror(ret));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd == -1)
            continue;
        if (connect(fd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;
        close(fd);
    }
    if (rp == NULL) {
        printf("Could not connect\n");
        return -1;
    };
    freeaddrinfo(result);
    printf("connect success\n");
    return fd;
}

int get_line_http(int fd, char* buf, ssize_t size)
{
    int i = 0;
    char c = '\0';
    int n;
    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(fd, &c, 1, 0);
        if (n > 0)
        {
            if (c == '\r')
            {
                n = recv(fd, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n'))
                {
                    recv(fd, &c, 1, 0);
                    char ch = '\0';
                    recv(fd, &ch, 1, MSG_PEEK);
                    if (ch == '\r')
                    {
                        recv(fd, &ch, 1, 0);
                        recv(fd, &ch, 1, 0);
                        buf[i] = '\n';
                        return 2;
                    }
                }
                else
                {
                    c = '\n';
                }
            }
            buf[i] = c;
            i++;
        }
        else
        {
            c = '\n';
        }
    }

    if (n == -1) return -1;
    if (n == 0)  return 0;
    return 1;
}

int main(int argc,char * argv[])
{
    unsigned short port = 80;
    const char* server = "api.seniverse.com";
    int fd = sock_init(server, port);

    
    char str1[4096];

    memset(str1, 0, 4096);
    strcat(str1, "GET /v3/weather/now.json?key=SVR-4vr6BWucOlHnf&location=wuhang&language=zh-Hans&unit=c HTTP/1.1\r\n");
    strcat(str1, "HOST: api.seniverse.com\r\n");
    strcat(str1, "\r\n");

    ssize_t wret = write(fd, str1, strlen(str1));
    if (wret < 0)
    {
        perror("write error!");
        exit(1);
    }
    printf("write success\n");

    char line[1024] = { 0 };

    char http_head[32][128];
    int numofhead = 0;

    ssize_t rret = 0;
    while ((rret = get_line_http(fd, line, sizeof(line))) != 2)
    {
        if (rret == -1)
        {
            perror("read error");
            exit(1);
        }
        else if (rret == 0)
        {
            printf("connect is closed");
        }

        strcpy(http_head[numofhead], line);
        numofhead++;
    }


    read(fd, line, sizeof(line));

    cJSON* json;
    char* out;
    json = cJSON_Parse(line);
    if (json == NULL)
    {
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        return 1;
    }
    cJSON* arrayItem = cJSON_GetObjectItem(json, "results");
    cJSON* object = cJSON_GetArrayItem(arrayItem, 0);

    cJSON* location;
    cJSON* now;
    cJSON* last_update;
    location = cJSON_GetObjectItem(object, "location");
    now = cJSON_GetObjectItem(object, "now");
    last_update = cJSON_GetObjectItem(object, "last_update");



    FILE* fp = fopen("/home/gck/Scripts/WeatherGet.txt","w");




    cJSON* id = cJSON_GetObjectItem(location, "id");
    fprintf(fp,"id:%s\n", id->valuestring);
    id = cJSON_GetObjectItem(location, "name");
    fprintf(fp,"name:%s\n", id->valuestring);
    id = cJSON_GetObjectItem(location, "country");
    fprintf(fp,"country:%s\n", id->valuestring);
    id = cJSON_GetObjectItem(location, "path");
    fprintf(fp,"path:%s\n", id->valuestring);
    id = cJSON_GetObjectItem(location, "timezone");
    fprintf(fp,"timezone:%s\n", id->valuestring);
    id = cJSON_GetObjectItem(location, "timezone_offset");
    fprintf(fp,"timezone_offset:%s\n", id->valuestring);
    id = cJSON_GetObjectItem(now, "text");
    fprintf(fp,"text:%s\n", id->valuestring);
    id = cJSON_GetObjectItem(now, "code");
    fprintf(fp,"code:%s\n", id->valuestring);
    id = cJSON_GetObjectItem(now, "temperature");
    fprintf(fp,"temperature:%s\n", id->valuestring);

    fprintf(fp,"last_update_time:%s\n", last_update->valuestring);
    
    close(fd);

}


