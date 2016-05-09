#include "libwebsockets.h"
#include "ss_api.h"

float VOLUME_MASTER = 30.0;

float VOLUME_S[10] = { 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3 };

float VOLUME_S0 = 0.3;
float VOLUME_S1 = 0.3;
float VOLUME_S2 = 0.3;
float VOLUME_S3 = 0.3;
float VOLUME_S4 = 0.3;
float VOLUME_S5 = 0.3;
float VOLUME_S6 = 0.3;
float VOLUME_S7 = 0.3;
float VOLUME_S8 = 0.3;
float VOLUME_S9 = 0.3;

float AMP_BUFFER[IO_BUFFER_SIZE] = { 0.0 };
float IN_AMP_BUFFER[IO_BUFFER_SIZE] = { 0.0 };

short AMP_INDEX = 0x00;
short AMP_READY = 0x00;

short IN_AMP_INDEX = 0x00;
short IN_AMP_READY = 0x00;

short PSD_BUFFER[10] = { 0.0 };

int callback_fft_plot(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    struct per_session_data__lws_mirror *pss = (struct per_session_data__lws_mirror *)user;
    const char delim[2] = ":";
    char l_amp_buffer[UI_BUFFER_SIZE];
    char temp[50];
    char *payloadPtr = in;
    char *token;
    int volume;
    int n, index;

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf("Callback connection established\n");
            break;
        case LWS_CALLBACK_PROTOCOL_DESTROY:
            printf("Destroy\n");
            lwsl_notice("%s: fft protocol cleaning up\n", __func__);
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
            printf("Send plot points\n");
            if(AMP_READY == 1 && IN_AMP_READY == 1) {
                for(index = 0; index < UI_BUFFER_SIZE; index++) {
                    l_amp_buffer[index] = 0;
                }
                printf("Buffer ready to be served\n");
                for(index = 0; index < IO_BUFFER_SIZE; index++) {
                    //sprintf(l_amp_buffer, "%f", AMP_BUFFER[index]);
                    //printf("IN_AMP: %f\n", IN_AMP_BUFFER[index]);
                    //puts(l_amp_buffer);
                    sprintf(temp, "%f : ", IN_AMP_BUFFER[index]);
                    strcat(l_amp_buffer, temp);
                    IN_AMP_BUFFER[index] = 0;
                }
                strcat(l_amp_buffer, "!! ");
                for(index = 0; index < IO_BUFFER_SIZE; index++) {
                    //sprintf(l_amp_buffer, "%f", AMP_BUFFER[index]);
                    //puts(l_amp_buffer);
                    sprintf(temp, "%f : ", AMP_BUFFER[index]);
                    strcat(l_amp_buffer, temp);
                    AMP_BUFFER[index] = 0;
                }
                strcat(l_amp_buffer, "\0");
                //puts(l_amp_buffer);
                AMP_INDEX = 0;
                AMP_READY = 0;
                IN_AMP_INDEX = 0;
                IN_AMP_READY = 0;

                lws_write(wsi, (unsigned char *) &l_amp_buffer, sizeof(char) * UI_BUFFER_SIZE, LWS_WRITE_TEXT);
            }
            break;
        case LWS_CALLBACK_RECEIVE:
                printf("Volume change received!\n");
                printf("%s\n", (const char *)in);

                token = strtok(payloadPtr, delim);
                printf(" %s\n", token);

                if(strcmp(token, "smaster") == 0) {
                    token = strtok(NULL, delim);
                    VOLUME_MASTER = atoi(token);
                    printf("volume: %f\n", VOLUME_MASTER);
                }
                else if(strcmp(token, "s0") == 0) {
                    token = strtok(NULL, delim);
                    VOLUME_S[0] = atoi(token) / 100.0;
                    printf("volume: %f\n", VOLUME_S[0]);
                }
                else if(strcmp(token, "s1") == 0) {
                    token = strtok(NULL, delim);
                    VOLUME_S[1] = atoi(token) / 100.0;
                    printf("volume: %f\n", VOLUME_S[1]);
                }
                else if(strcmp(token, "s2") == 0) {
                    token = strtok(NULL, delim);
                    VOLUME_S[2] = atoi(token) / 100.0;
                    printf("volume: %f\n", VOLUME_S[2]);
                }
                else if(strcmp(token, "s3") == 0) {
                    token = strtok(NULL, delim);
                    VOLUME_S[3] = atoi(token) / 100.0;
                    printf("volume: %f\n", VOLUME_S[3]);
                }
                else if(strcmp(token, "s4") == 0) {
                    token = strtok(NULL, delim);
                    VOLUME_S[4] = atoi(token) / 100.0;
                    printf("volume: %f\n", VOLUME_S[4]);
                }
                else if(strcmp(token, "s5") == 0) {
                    token = strtok(NULL, delim);
                    VOLUME_S[5] = atoi(token) / 100.0;
                    printf("volume: %f\n", VOLUME_S[5]);
                }
                else if(strcmp(token, "s6") == 0) {
                    token = strtok(NULL, delim);
                    VOLUME_S[6] = atoi(token) / 100.0;
                    printf("volume: %f\n", VOLUME_S[6]);
                }
                else if(strcmp(token, "s7") == 0) {
                    token = strtok(NULL, delim);
                    VOLUME_S[7] = atoi(token) / 100.0;
                    printf("volume: %f\n", VOLUME_S[7]);
                }
                else if(strcmp(token, "s8") == 0) {
                    token = strtok(NULL, delim);
                    VOLUME_S[8] = atoi(token) / 100.0;
                    printf("volume: %f\n", VOLUME_S[8]);
                }
                else if(strcmp(token, "s9") == 0) {
                    token = strtok(NULL, delim);
                    VOLUME_S[9] = atoi(token) / 100.0;
                    printf("volume: %f\n", VOLUME_S[9]);
                }
                else {

                }
                goto done; 
        choke:
            lwsl_debug("LWS_CALLBACK_RECEIVE: throttling %p\n", wsi);
            lws_rx_flow_control(wsi, 0);
        done:
            lws_callback_on_writable_all_protocol(lws_get_context(wsi), lws_get_protocol(wsi));
            break;
            /*
             * this just demonstrates how to use the protocol filter. If you won't
             * study and reject connections based on header content, you don't need
             * to handle this callback
             */
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
            /* SS_Server_DumpHandShake(wsi); */
            /* you could return non-zero here and kill the connection */
            break;
        default:
            break;
    }
    return 0;
}
