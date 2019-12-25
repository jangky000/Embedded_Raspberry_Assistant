#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include <errno.h>

#include <string.h>

#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

//#include <locale.h>

void Schedule_Delete(char *day, char *msg);
void Schedule_Add(char *day, char *msg);
void Schedule_List(char *yyyymmdd);
int Decode(char *result, char *msg);
int Run_Socket(char *sb_name);
void history(char *msg);
void *ThreadMain(void *argument);

bdaddr_t bdaddr_any = {0, 0, 0, 0, 0, 0};

bdaddr_t bdaddr_local = {0, 0, 0, 0xff, 0xff, 0xff};



int _str2uuid( const char *uuid_str, uuid_t *uuid ) {



    /* This is from the pybluez stack */



    uint32_t uuid_int[4];

    char *endptr;



    if( strlen( uuid_str ) == 36 ) {

        char buf[9] = { 0 };



        if( uuid_str[8] != '-' && uuid_str[13] != '-' &&

        uuid_str[18] != '-' && uuid_str[23] != '-' ) {

        return 0;

    }



    // first 8-bytes

    strncpy(buf, uuid_str, 8);

    uuid_int[0] = htonl( strtoul( buf, &endptr, 16 ) );

    if( endptr != buf + 8 ) return 0;

        // second 8-bytes

        strncpy(buf, uuid_str+9, 4);

        strncpy(buf+4, uuid_str+14, 4);

        uuid_int[1] = htonl( strtoul( buf, &endptr, 16 ) );

        if( endptr != buf + 8 ) return 0;



        // third 8-bytes

        strncpy(buf, uuid_str+19, 4);

        strncpy(buf+4, uuid_str+24, 4);

        uuid_int[2] = htonl( strtoul( buf, &endptr, 16 ) );

        if( endptr != buf + 8 ) return 0;



        // fourth 8-bytes

        strncpy(buf, uuid_str+28, 8);

        uuid_int[3] = htonl( strtoul( buf, &endptr, 16 ) );

        if( endptr != buf + 8 ) return 0;



        if( uuid != NULL ) sdp_uuid128_create( uuid, uuid_int );

    } else if ( strlen( uuid_str ) == 8 ) {

        // 32-bit reserved UUID

        uint32_t i = strtoul( uuid_str, &endptr, 16 );

        if( endptr != uuid_str + 8 ) return 0;

        if( uuid != NULL ) sdp_uuid32_create( uuid, i );

    } else if( strlen( uuid_str ) == 4 ) {

        // 16-bit reserved UUID

        int i = strtol( uuid_str, &endptr, 16 );

        if( endptr != uuid_str + 4 ) return 0;

        if( uuid != NULL ) sdp_uuid16_create( uuid, i );

    } else {

        return 0;

    }

    return 1;



}





sdp_session_t *register_service(uint8_t rfcomm_channel) {



    /* A 128-bit number used to identify this service. The words are ordered from most to least



    * significant, but within each word, the octets are ordered from least to most significant.



    * For example, the UUID represneted by this array is 00001101-0000-1000-8000-00805F9B34FB. (The



    * hyphenation is a convention specified by the Service Discovery Protocol of the Bluetooth Core



    * Specification, but is not particularly important for this program.)



    *



    * This UUID is the Bluetooth Base UUID and is commonly used for simple Bluetooth applications.



    * Regardless of the UUID used, it must match the one that the Armatus Android app is searching



    * for.



    */



    const char *service_name = "Armatus Bluetooth server";



    const char *svc_dsc = "A HERMIT server that interfaces with the Armatus Android app";



    const char *service_prov = "Armatus";



    uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid,

           svc_class_uuid;

    sdp_list_t *l2cap_list = 0,

                *rfcomm_list = 0,

                *root_list = 0,

                *proto_list = 0,

                *access_proto_list = 0,

                *svc_class_list = 0,

                *profile_list = 0;



    sdp_data_t *channel = 0;

    sdp_profile_desc_t profile;

    sdp_record_t record = { 0 };

    sdp_session_t *session = 0;



    // set the general service ID

    //sdp_uuid128_create(&svc_uuid, &svc_uuid_int);

    _str2uuid("00001101-0000-1000-8000-00805F9B34FB",&svc_uuid);

    sdp_set_service_id(&record, svc_uuid);



    char str[256] = "";

    sdp_uuid2strn(&svc_uuid, str, 256);

    printf("Registering UUID %s\n", str);



    // set the service class

    sdp_uuid16_create(&svc_class_uuid, SERIAL_PORT_SVCLASS_ID);

    svc_class_list = sdp_list_append(0, &svc_class_uuid);

    sdp_set_service_classes(&record, svc_class_list);



    // set the Bluetooth profile information

    sdp_uuid16_create(&profile.uuid, SERIAL_PORT_PROFILE_ID);

    profile.version = 0x0100;

    profile_list = sdp_list_append(0, &profile);

    sdp_set_profile_descs(&record, profile_list);



    // make the service record publicly browsable

    sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);

    root_list = sdp_list_append(0, &root_uuid);

    sdp_set_browse_groups(&record, root_list);



    // set l2cap information

    sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);

    l2cap_list = sdp_list_append(0, &l2cap_uuid);

    proto_list = sdp_list_append(0, l2cap_list);



    // register the RFCOMM channel for RFCOMM sockets

    sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);

    channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);

    rfcomm_list = sdp_list_append(0, &rfcomm_uuid);

    sdp_list_append(rfcomm_list, channel);

    sdp_list_append(proto_list, rfcomm_list);



    access_proto_list = sdp_list_append(0, proto_list);

    sdp_set_access_protos(&record, access_proto_list);



    // set the name, provider, and description

    sdp_set_info_attr(&record, service_name, service_prov, svc_dsc);



    // connect to the local SDP server, register the service record,

    // and disconnect

    session = sdp_connect(&bdaddr_any, &bdaddr_local, SDP_RETRY_IF_BUSY);

    sdp_record_register(session, &record, 0);



    // cleanup

    sdp_data_free(channel);

    sdp_list_free(l2cap_list, 0);

    sdp_list_free(rfcomm_list, 0);

    sdp_list_free(root_list, 0);

    sdp_list_free(access_proto_list, 0);

    sdp_list_free(svc_class_list, 0);

    sdp_list_free(profile_list, 0);

    return session;

}

char input[1024] = { 0 };

char *read_server(int client) {
    // read data from the client
    int bytes_read;
    memset(input, 0, 1024);
    bytes_read = read(client, input, sizeof(input));
    if (bytes_read > 0) {
        printf("received [%s]\n", input);
        return input;

    } else {
        return NULL;
    }
}

void write_server(int client, char *message) {

    // send data to the client

    char messageArr[1024] = { 0 };

    int bytes_sent;
    
    memset(messageArr, 0, 1024);

    strcpy(messageArr, message);

    bytes_sent = write(client, messageArr, strlen(messageArr));

    if (bytes_sent > 0) {

        printf("sent [%s] %d\n", messageArr, bytes_sent);
    }
}

void File_Init()
{
    FILE *fp = fopen("history.txt", "w");
    fclose(fp);
    FILE *fp2 = fopen("schedule.txt", "w");
    fclose(fp2);
    FILE *fp3 = fopen("news.txt", "w");
    fclose(fp3);
    FILE *fp4 = fopen("weather.txt", "w");
    fclose(fp4);
}

int main()
{
    File_Init();
    pthread_t thread_id;  
    signal( SIGPIPE, SIG_IGN );  

    int port = 3, result, sock, client, bytes_read, bytes_sent;
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buffer[1024] = { 0 };
    socklen_t opt = sizeof(rem_addr);

    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = bdaddr_any;
    loc_addr.rc_channel = (uint8_t) port;


    // register service
    sdp_session_t *session = register_service(port);

    // allocate socket
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    printf("socket() returned %d\n", sock);

    // bind socket to port 3 of the first available
    result = bind(sock, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
    printf("bind() on channel %d returned %d\n", port, result);

    // put socket into listening mode
    result = listen(sock, 1);
    printf("listen() returned %d\n", result);

    //sdpRegisterL2cap(port);
    while(1)
    {
        // accept one connection
        printf("calling accept()\n");
        client = accept(sock, (struct sockaddr *)&rem_addr, &opt);
        printf("accept() returned %d\n", client);

        ba2str(&rem_addr.rc_bdaddr, buffer);
        printf("accepted connection from %s\n", buffer);
        history("블루투스 연결\n");
        memset(buffer, 0, sizeof(buffer));
        pthread_create( &thread_id, NULL, ThreadMain, (void*)client);   
    }
}



int Run_Socket(char *msg)
{
    //int MAXLINE = 256;
    char line[256];
    char sb_name[256] = "./RA_client";
    sprintf(sb_name, "%s %s", sb_name, msg);

    FILE *fpin;

    if( (fpin = popen(sb_name, "r")) == NULL )
    {
        perror("파일을 찾을 수 없습니다\n");
        return -1;
    }
    /*
    fgets(line, 256, fpin);
    printf("1");
    printf(line);
    fgets(line, 256, fpin);
    printf("2");
    printf(line);
    fgets(line, 256, fpin);
    printf("3");
    printf(" <%d> ", (int)strcmp(line, "weather\n"));
    printf("[%s]",line);
    fgets(line, 256, fpin);
    printf("4");
    printf(line);
    */
    
    while(fgets(line, 256, fpin))
    {
        history(line);
        printf(line);
        if(!strcmp(line, "weather\n") || !strcmp(line, "news\n")) break;
    }
    if(!strcmp(line, "weather\n"))
    {
        printf("write weather.txt\n");
        FILE *fp = fopen("weather.txt", "w");
        while(fgets(line, 256, fpin))
        {
            if(!strcmp(line, "done\n"))
            {
                printf(line);
                break;
            }
            fputs(line, fp);
        }
        fclose(fp);
        while(fgets(line, 256, fpin))
        {
            printf(line);
        }
        history("날씨 정보 수집 완료\n");
    }
    else if(!strcmp(line, "news\n"))
    {
        printf("write news.txt\n");
        FILE *fp = fopen("news.txt", "w");
        while(fgets(line, 256, fpin))
        {
            if(!strcmp(line, "done"))
            {
                printf(line);
                break;
            }
            fputs(line,fp);
        }
        fclose(fp);
        while(fgets(line, 256, fpin))
        {
            printf(line);
        }
        history("뉴스 정보 수집 완료\n");
    }
    
    pclose(fpin);

}

void history(char *msg)
{
    FILE *fp = fopen("history.txt", "a"); // -> "a" and RA3 -> setText
    fprintf(fp, "%s", msg);
    fclose(fp);
}

int Decode(char *result, char *msg)
{
    FILE *fpin;
    char decode[256];
    //char result[MAXLINE];
    sprintf(decode, "python decode.py %s", msg);
    if( (fpin = popen(decode, "r")) == NULL )
    {
        perror("파일을 찾을 수 없습니다\n");
        return -1;
    }
    //while(fgets(result, 256, fpin))
    //{
    //    printf(result);
    //}
    fgets(result, 256, fpin);
    printf(result);
    pclose(fpin);
}


void Schedule_List(char *day)
{
    FILE *fp = fopen("schedule.txt", "w");
    char date[50];
    sprintf(date, "<h3>%c%c%c%c년도 %c%c월 %c%c일 일정</h3>\n", day[0], day[1], day[2], day[3], day[4], day[5], day[6], day[7]);
    fputs(date, fp);
    fputs("<ul>\n", fp);
    
    FILE *fp2 = fopen("calender.txt", "r");
    char line[256];
    while(fgets(line, 256, fp2))
    {
        if(!strncmp(line+1, day, 8))
        {
            fputs("<li>", fp);
            fputs(line+11,fp);
            fputs("</li>\n", fp);
        }
    }
    
    fclose(fp2);
    
    fputs("</ul>", fp);
    fclose(fp);
    printf("schedule done\n");
}

void Schedule_Add(char *day, char *msg)
{
    FILE *fp = fopen("calender.txt", "a");
    fputs("[", fp);
    fputs(day, fp);
    fputs("] ", fp);
    fputs(msg, fp);
    fputs("\n", fp);
    fclose(fp);
}

void Schedule_Delete(char *day, char *msg)
{
    FILE *fp1 = fopen("calender.txt", "r");
    FILE *fp2 = fopen("calender_tmp.txt", "w");
    
    char delete[256];
    sprintf(delete, "[%s] %s\n", day, msg);
    printf("[delete: %s]\n", delete);
    
    char buf[256]={0,};
    char date[20]={0,};
    char schedule[256]={0,};
    char line[256]={0,};
    int flag = 0;
    while(!feof(fp1))
    {
        fscanf(fp1, "%s", buf);
        if(buf[0]=='[' && buf[9]==']')
        {
            if(flag == 0)
            {
                strcpy(date, buf);
                memset(buf, 0, sizeof(buf));
                flag = 1;
            }
            else{
                sprintf(line, "%s%s\n", date, schedule);
                if(strcmp(line, delete))
                {
                    fprintf(fp2, "%s", line);
                }
                memset(schedule, 0, sizeof(schedule));
                strcpy(date, buf);
                memset(buf, 0, sizeof(buf));
                memset(line, 0, sizeof(line));
            }
        }
        else{
            strcat(schedule, " ");
            strcat(schedule, buf);
            memset(buf, 0, sizeof(buf));
        }
    }
    schedule[strlen(schedule)-1]='\0';
    sprintf(line, "%s%s\n", date, schedule);
    if(strcmp(line, delete))
    {
        fprintf(fp2, "%s", line);
    }
    memset(buf, 0, sizeof(buf));
    memset(date, 0, sizeof(date));
    memset(schedule, 0, sizeof(schedule));
    flag=0;
    
    fclose(fp2);
    fclose(fp1);
    
    //delete
    int nResult = remove( "calender.txt" );
	if( nResult == 0 )
	{
		printf( "파일 삭제 성공\n" );
	}
	else if( nResult == -1 )
	{
		perror( "파일 삭제 실패\n" );
	}
    //rename
    nResult = rename( "calender_tmp.txt", "calender.txt" );

	if( nResult == 0 )
	{
		printf( "이름 변경 성공\n" );
	}
	else if( nResult == -1 )
	{
		perror( "이름 변경 실패\n" );
	}
}

char flag[10];
char yyyymmdd[10];

void *ThreadMain(void *argument)  
{  
    char buf[1024];  
    pthread_detach(pthread_self());  
    int client = (int)argument;  

    while(1)
    {  
        char *recv_message = read_server(client);

        if(recv_message == NULL ){
            printf("client disconnected\n");
            break;
        } 

        printf("%s\n", recv_message);
        //저장
        char history_msg[256];
        sprintf(history_msg, "음성인식: [%s]\n", recv_message);
        history(history_msg);
        
        if(!strcmp("취소", recv_message))
        {
            //printf(flag);
            if(strlen(flag)!=0)
            {
                memset(flag, 0, sizeof(flag));
                memset(yyyymmdd, 0, sizeof(yyyymmdd));
                history("취소 완료\n");
                printf("취소 완료\n");
            }
            continue;
        }
        
        if(!strcmp(flag, "121"))
        {
            Schedule_Add(yyyymmdd, recv_message);
            history("일정 추가 완료\n");
            printf("일정 추가 완료\n");
            Schedule_List(yyyymmdd);
            history("수정된 일정 조회\n");
            printf("수정된 일정 조회\n");
            memset(flag, 0, sizeof(flag));
            memset(yyyymmdd, 0, sizeof(yyyymmdd));
            continue;
        }
        else if(!strcmp(flag, "131"))
        {
            Schedule_Delete(yyyymmdd, recv_message);
            history("일정 삭제 완료\n");
            printf("일정 삭제 완료\n");
            Schedule_List(yyyymmdd);
            history("수정된 일정 조회\n");
            printf("수정된 일정 조회\n");
            memset(flag, 0, sizeof(flag));
            memset(yyyymmdd, 0, sizeof(yyyymmdd));
            continue;
        }
        
        //자연어 처리
        history("자연어 처리\n");
        printf("자연어 처리\n");
        char result[256];
        if( Decode(result, recv_message) == -1 )
        {
            printf("decode popen error\n");
        }
        //저장
        history(result);
        
        // 내부외부 처리
        if(result[0] == '2'){
            //소켓 연결
            history("PC 연결\n");
            printf("PC 연결\n");
            if(Run_Socket(recv_message) == -1){
                printf("socket error\n");
                history("소켓 생성 오류\n");
            }
            history("PC 연결 해제\n");
            printf("PC 연결 해제\n");
        }
        
        // 일정
        else if(result[0] == '1')
        {
            //il
            if(result[1] == '1')
            {
                //jo
                //printf("jo\n");
                if(result[2] == '0')
                {
                    history("날짜 입력 대기\n");
                    printf("날짜 입력 대기\n");
                    //flag naljja
                    sprintf(flag, "110");
                }
                else if(result[2] == '1')
                {
                    history("일정 조회\n");
                    printf("일정 조회\n");
                    //show list
                    memset(yyyymmdd, 0, sizeof(yyyymmdd));
                    strncpy(yyyymmdd, result+4, 8);
                    yyyymmdd[8]='\0';
                    Schedule_List(yyyymmdd);
                    memset(flag, 0, sizeof(flag));
                    memset(yyyymmdd, 0, sizeof(yyyymmdd));
                }
            }
            else if(result[1] == '2')
            {
                //chu
                //printf("chu\n");
                if(result[2] == '0')
                {
                    history("날짜 입력 대기\n");
                    printf("날짜 입력 대기\n");
                    //flag naljja
                    sprintf(flag, "120");
                }
                else if(result[2] == '1')
                {
                    history("추가 내용 대기\n");
                    printf("추가 내용 대기\n");
                    memset(yyyymmdd, 0, sizeof(yyyymmdd));
                    strncpy(yyyymmdd, result+4, 8);
                    yyyymmdd[8]='\0';
                    //flag chu msg
                    sprintf(flag, "121");
                }
            }
            else if(result[1] == '3')
            {
                //sak
                //printf("sak\n");
                if(result[2] == '0')
                {
                    history("날짜 입력 대기\n");
                    printf("날짜 입력 대기\n");
                    //flag naljja
                    sprintf(flag, "130");
                }
                else if(result[2] == '1')
                {
                    history("삭제 일정 조회\n");
                    printf("삭제 일정 조회\n");
                    //show list
                    memset(yyyymmdd, 0, sizeof(yyyymmdd));
                    strncpy(yyyymmdd, result+4, 8);
                    yyyymmdd[8]='\0';
                    Schedule_List(yyyymmdd);
                    history("삭제 내용 대기\n");
                    printf("삭제 내용 대기\n");
                    //flag sak msg
                    sprintf(flag, "131");
                }
            }
            else if(result[1] == '0')
            {
                //history("날짜 입력\n");
                //printf("nal\n");
                if(!strcmp(flag, "110"))
                {
                    history("일정 조회\n");
                    printf("일정 조회\n");
                    //show list
                    memset(yyyymmdd, 0, sizeof(yyyymmdd));
                    strncpy(yyyymmdd, result+4, 8);
                    yyyymmdd[8]='\0';
                    Schedule_List(yyyymmdd);
                    memset(flag, 0, sizeof(flag));
                    memset(yyyymmdd, 0, sizeof(yyyymmdd));
                }
                else if(!strcmp(flag, "120"))
                {
                    history("추가 내용 대기\n");
                    printf("추가 내용 대기\n");
                    memset(yyyymmdd, 0, sizeof(yyyymmdd));
                    strncpy(yyyymmdd, result+4, 8);
                    yyyymmdd[8]='\0';
                    //flag chu msg
                    sprintf(flag, "121");
                }
                else if(!strcmp(flag, "130"))
                {
                    history("삭제일정 조회\n");
                    printf("삭제일정 조회\n");
                    //show list
                    memset(yyyymmdd, 0, sizeof(yyyymmdd));
                    strncpy(yyyymmdd, result+4, 8);
                    yyyymmdd[8]='\0';
                    Schedule_List(yyyymmdd);
                    history("삭제 내용 대기\n");
                    printf("삭제 내용 대기\n");
                    //flag sak msg
                    sprintf(flag, "131");
                }
            }
            else
            {
                //flag
            }
        }

        //write_server(client, recv_message);

    }  

    printf("disconnected\n" );  

    close(client);  

    return 0;    

}  


