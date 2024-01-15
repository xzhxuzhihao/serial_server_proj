#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <syslog.h>
#include <osip2/osip.h>
#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
#include <eXosip2/eX_message.h>
#include <osipparser2/osip_port.h>

static volatile int keepRuning = 1;
char localip[16];
struct eXosip_t *context_t;
struct client_invite{
	const char *src;
	const char *dst;
	const char *sipstr;
};
static void intHandler(int dummy){
	keepRuning = 0;
}
void my_trace_func(const char *fi, int li, osip_trace_level_t level, const char *chfr, va_list ap)
{
    vprintf(chfr, ap);
}

int ClientInvite(void *arg){
    int i;
    char tmpcallId[64];
    char tmp[4096];
    osip_message_t *invite = NULL;
	struct client_invite *client_t = (struct client_invite *)arg;
    //memset(tmpcallId, '\0', 64);
  
    //to, from, route, subject
    i = eXosip_call_build_initial_invite (context_t, &invite, client_t->dst, client_t->src, NULL, client_t->sipstr);
    if (i != 0)
    {
        printf ("Initial INVITE failed!\n");
        return -1;
    }
  
	sprintf (tmp, 
			  "v=0\n"
			  "o=1002 2590 505 IN IP4 %s\n"
			  "s=Talk\n"
			  "c=IN IP4 %s\n"
			  "t=0 0\n"
			  "a=rtcp-xr:rcvr-rtt=all:10000 stat-summary=loss,dup,jitt,TTL voip-metrics\n"
			  "a=record:off\n"
			  "m=audio 57200 RTP/AVP 96 97 98 0 8 99 100 101\n"
			  "a=rtpmap:96 opus/48000/2\n"
			  "a=fmtp:96 useinbandfec=1\n"
			  "a=rtpmap:97 speex/16000\n"
			  "a=fmtp:97 vbr=on\n"
			  "a=rtpmap:98 speex/8000\n"
			  "a=fmtp:98 vbr=on\n"
			  "a=rtpmap:99 telephone-event/48000\n"
			  "a=rtpmap:100 telephone-event/16000\n"
			  "a=rtpmap:101 telephone-event/8000\n"
			  "a=rtcp:37411\n"
			  "a=rtcp-fb:* trr-int 1000\n"
			  "a=rtcp-fb:* ccm tmmbr\n", localip, localip);  

    osip_message_set_body (invite, tmp, strlen(tmp));
    osip_message_set_content_type (invite, "application/sdp");
    osip_message_set_header (invite, "Allow","Invite,ACK,UPDATE,INFO,CANCEL,BYE,OPTIONS,REFER,SUBSCRIBE,N      OTIFY,MESSAGE");
    osip_message_set_header (invite, "p-hint", "z_looup");
  
    eXosip_lock (context_t);
    //printf("eXosip_call_send_initial_invite\n");
    i=eXosip_call_send_initial_invite (context_t, invite);
    eXosip_unlock (context_t);
  
    return 0;
}
int GetHostIP(){
    struct ifaddrs *ifaddr, *ifa;
    int  s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we can free list later */
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if((strcmp(ifa->ifa_name,"lo")!=0)&&(ifa->ifa_addr->sa_family==AF_INET)) {
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
			strcpy(localip, host);
			break;
        }
    }

    freeifaddrs(ifaddr);
	return 0;
}

void *sip_main(void *arg){
    sdp_connection_t *connection;
    sdp_message_t *sdp;
    osip_message_t *ack = NULL;
	char tmp[4096] = {0};
	int wait_mtime = 100;

	for(; keepRuning;){
		eXosip_event_t *event;
		event = eXosip_event_wait(context_t, 0, wait_mtime);
		if(!event){
			eXosip_automatic_action(context_t);
			osip_usleep(10000);
			continue;
		}
		eXosip_lock(context_t);
		eXosip_automatic_action(context_t);
		switch (event->type) {
			case EXOSIP_REGISTRATION_SUCCESS:
				printf("registered successfully\n");
				break;
			case EXOSIP_REGISTRATION_FAILURE:
				printf("registered failed.\n");
				break;
			case EXOSIP_CALL_ANSWERED:
				printf("receive 200-OK!\n");
 
				printf("call_id is %d,dialog_id is %d \n", event->cid, event->did);
 
				eXosip_call_build_ack (context_t, event->did, &ack);
				//vias
				eXosip_call_send_ack (context_t, event->did, ack);
 
				osip_message_t *pResponse;
				osip_message_clone(event->response, &pResponse);
 
				sdp = eXosip_get_sdp_info(pResponse);
				if(!sdp)
				{
					printf("Get SDP failed from message \n");
					break;
				}
				connection = eXosip_get_video_connection(sdp);
				if(!connection)
				{
					printf("Get Connection failed from SDP \n ");
					break;
				}
				else
				{
					if (connection->c_addr != NULL)
					 printf("Connection ip:%s\n",connection->c_addr);
				}
				//sdp_media_t *video_sdp = eXosip_get_video_media(sdp);
				break;
			case EXOSIP_MESSAGE_NEW:{
				osip_message_t *answer;
				int i= eXosip_message_build_answer(context_t, event->tid, 405, &answer);
				if (i != 0){
					printf("failed to reject %s\n", event->request->sip_method);
					break;
				}
				i = eXosip_message_send_answer(context_t, event->tid, 405, answer);

				if (i != 0) {
					printf("failed to reject %s\n", event->request->sip_method);
					break;
				}

				printf("%s rejected with 405", event->request->sip_method);
				break;
			}
			case EXOSIP_CALL_INVITE: {
				osip_message_t *answer;
				int i;

				i = eXosip_call_build_answer(context_t, event->tid, 405, &answer);

				if (i != 0) {
				  printf("failed to reject %s\n", event->request->sip_method);
				  break;
				}

				sprintf (tmp, 
				"v=0\n"
				"o=1002 2590 505 IN IP4 %s\n"
				"s=Talk\n"
				"c=IN IP4 %s\n"
				"t=0 0\n"
				"a=rtcp-xr:rcvr-rtt=all:10000 stat-summary=loss,dup,jitt,TTL voip-metrics\n"
				"a=record:off\n"
				"m=audio 18000 RTP/AVP 96 97 98 0 8 99 100 101\n"
				"a=rtpmap:96 opus/48000/2\n"
				"a=fmtp:96 useinbandfec=1\n"
				"a=rtpmap:97 speex/16000\n"
				"a=fmtp:97 vbr=on\n"
				"a=rtpmap:98 speex/8000\n"
				"a=fmtp:98 vbr=on\n"
				"a=rtpmap:99 telephone-event/48000\n"
				"a=rtpmap:100 telephone-event/16000\n"
				"a=rtpmap:101 telephone-event/8000\n"
				"a=rtcp:18001\n"
				"a=rtcp-fb:* trr-int 1000\n"
				"a=rtcp-fb:* ccm tmmbr\n", localip, localip);
				osip_message_set_body (answer, tmp, strlen(tmp));
				osip_message_set_content_type (answer, "application/sdp");

				osip_free(answer->reason_phrase);
				answer->reason_phrase = osip_strdup("No Support for Incoming Calls");
				i = eXosip_call_send_answer(context_t, event->tid, 405, answer);


				if (i != 0) {
				  printf("failed to reject %s\n", event->request->sip_method);
				  break;
				}
			
				printf("%s rejected with 405", event->request->sip_method);
				break;
			}
			case EXOSIP_CALL_CLOSED:
		        osip_message_t *answer;
				int i;

			    i = eXosip_call_build_answer(context_t, event->tid, 200, &answer);
			    if (i != 0) {
			      printf("failed to bye");
			      break;
			    }

			    osip_free(answer->reason_phrase);
			    answer->reason_phrase = osip_strdup("Release the pbx Calls");
			    i = eXosip_call_send_answer(context_t, event->tid, 200, answer);

			    if (i != 0) {
			      printf("failed to reject bye");
			      break;
			    }

			    printf("Send 200 for bye");
			    break;
			case EXOSIP_CALL_RELEASED:
				break;
			default:
				printf("others message\n");
				break;
		}
		eXosip_unlock(context_t);
		eXosip_event_free(event);
	}

    eXosip_quit(context_t);
	osip_free(context_t);

    return 0;
}
int main(){
	int port = 15060;
	int optval;
	int automasquerade = 0;
	int expiry = 3600;
	const char *proxy = "sip:192.168.1.102";
	const char *from = "sip:1002@192.168.1.102";
	const char *contact = "<sip:1002@192.168.1.103:15060;transport=udp>;expires=3600";
	const char *username = "1002";
	const char *password = "1234";
	const char *prog_name = "eXosip2-5.3.0";

	GetHostIP();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, intHandler);
    context_t = eXosip_malloc();
    //osip_trace_initialize_func(OSIP_INFO4, &my_trace_func);
    if (eXosip_init(context_t) != 0) {
        printf("Couldn't initialize eXosip!\n");
        eXosip_quit(context_t);
		osip_free(context_t);
        return -1;
    }
    
    if (eXosip_listen_addr(context_t, IPPROTO_UDP, NULL, port, AF_INET, 0) != 0) {
        printf("Couldn't initialize transport layer!\n");
        eXosip_quit(context_t);
		osip_free(context_t);
        return -1;
    }

	optval = automasquerade;
	eXosip_lock(context_t);
	eXosip_set_option(context_t, EXOSIP_OPT_AUTO_MASQUERADE_CONTACT, &optval);
	eXosip_set_user_agent(context_t, prog_name);
	eXosip_add_authentication_info(context_t, username, username, password, NULL, NULL);
	eXosip_unlock(context_t);

    // Build the REGISTER request
	{
		eXosip_lock(context_t);
		osip_message_t *reg = NULL;
		int result_judge = eXosip_register_build_initial_register(context_t, from, proxy, contact, expiry*2, &reg);
		if(result_judge<1) printf("register build error.\n");
											   
		// Send the REGISTER request
		int result_judge_1 = eXosip_register_send_register(context_t, result_judge, reg);
		if(result_judge_1!=0) printf("register send error.\n");
		eXosip_unlock(context_t);
	}
	pthread_t response_prc_thr;
	struct client_invite client_t;
	client_t.dst = "sip:1001@192.168.1.102:5060";
	client_t.src = "sip:1002@192.168.1.102:15060";
	client_t.sipstr = NULL;
	pthread_create(&response_prc_thr, NULL, sip_main, NULL);	

    printf("\n\n");
    
    printf("\n=============================\n");
    printf("1     send invite\n\n");
    printf("q     exit\n\n");
    printf("\n=============================\n");
    while (1){
		char command;
		char c;
		printf ("please input the comand:\n");
 
        scanf("%c", &command);
		while((c = getchar()) != '\n')
			;

        switch(command){
            case '1'://register
                ClientInvite(&client_t);
               break;
            case 'q':
				eXosip_quit(context_t);
				osip_free(context_t);
                return 0;
            default:
               break;
        }
    }

	return 0;
}
