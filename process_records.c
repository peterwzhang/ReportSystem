//#define _GNU_SOURCE // remove this, its included in compile command
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "report_record_formats.h"
#include "queue_ids.h"

typedef struct thread_param{
    int *recordCounts;
    int size;
    int *totalRecCount;

} parameters;

typedef struct condLock{
    sem_t mutex;
    sem_t cond;
    int cond_value;
} condlock;

int cmp(const void* a, const void* b);
void hookSignal();
void sigHandler(int signo);
void printRRBuf(const report_request_buf *buffer);
void printReport(void *args);
void *waitForSignal(void *args);
void updateQueueId(const int queueNum, key_t *key, int *msqid, const int msgflg);
void getMessage(const int msqid, report_request_buf *rbuf);
void sendMessage(const int msqid, report_record_buf *sbuf, const char *string);

condlock lock_data;

int main(int argc, char**argv)
{
    // make sure my semaphores are initialized properly
    assert(sem_init(&lock_data.mutex, 0, 1) == 0);
    assert(sem_init(&lock_data.cond, 0, 0) == 0);
    lock_data.cond_value = 0;

    // variables for receiving and sending messages
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    report_request_buf *rbufArr = malloc(sizeof *rbufArr);
    assert(rbufArr != NULL);

    // get the first record request
    updateQueueId(QUEUE_NUMBER, &key, &msqid, msgflg);
    getMessage(msqid, rbufArr);
    //printRRBuf(&rbuf); debug print

    //resize rbufArr to hold all record requests, then scan in the rest of the requests
    int totalCount = rbufArr->report_count;
    rbufArr = realloc(rbufArr, totalCount * (sizeof *rbufArr));
    assert(rbufArr != NULL);
    for (int i = 1; i < totalCount; ++i){
        getMessage(msqid, &rbufArr[i]);
    }
    // debug print
    // for (int i = 0; i < totalCount; ++i){
    //     printRRBuf(&rbufArr[i]);
    // }

    //sort our array based on thread ids
    qsort(rbufArr, totalCount, sizeof(*rbufArr), cmp);

    // set up shared array on heap, for the number of records to be sent per queue
    int *sentRecCount = calloc(totalCount, sizeof *sentRecCount);
    assert(sentRecCount != NULL);

    // create our printing thread
    pthread_t printThread;
    int *totalRecCount = calloc(1, sizeof *totalRecCount); // shared variable on heap for total # records
    assert(totalRecCount!= NULL);
    // pass pointers of shared heap variables for printing to our status function -> less global variables
    parameters *p = malloc(sizeof *p);
    assert(p != NULL);
    p->recordCounts = sentRecCount;
    p->size = totalCount;
    p->totalRecCount = totalRecCount;
    pthread_create(&printThread, NULL, (void*) waitForSignal, (void*) p);

    // scan stdin for requested words
    char line[RECORD_MAX_LENGTH];
    report_record_buf sbuf; // buffer to store our messages to send
    //int sentRecCount[totalCount];
    while (fgets(line, RECORD_MAX_LENGTH, stdin)){
            for (int i = 0; i < totalCount; ++i){
                if (strstr(line, rbufArr[i].search_string) != NULL){
                    //fprintf(stderr, "%s", line);
                    //send line if match
                    sem_wait(&lock_data.mutex);
                    ++sentRecCount[i];
                    sem_post(&lock_data.mutex);
                    updateQueueId(rbufArr[i].report_idx, &key, &msqid, msgflg);
                    sendMessage(msqid, &sbuf, line);
                }
        }
        sem_wait(&lock_data.mutex);
        ++(*totalRecCount);
        //sem_post(&lock_data.mutex);
        if (*totalRecCount == 10){
            // SIGINT will wake up the main thread, if this happens sleep returns the remaining time
            // so by storing this we can continue sleeping for the remaining time.
            // int remainingTime = sleep(5);
            // while (remainingTime != 0) remainingTime = sleep(remainingTime);
            sleep(5);
        }
        sem_post(&lock_data.mutex);

    }
    char *emptyString = malloc(sizeof *emptyString);
    emptyString[0] = 0;
    for (int i = 0; i < totalCount; ++i){
        updateQueueId(i + 1, &key, &msqid, msgflg);
        sendMessage(msqid, &sbuf, emptyString);
    }
    sem_wait(&lock_data.mutex);
    lock_data.cond_value = 1;
    printReport((void*) p);
    sem_post(&lock_data.cond);
    sem_post(&lock_data.mutex);
    pthread_join(printThread, NULL); // wait for final print to finish
    free(rbufArr);
    free(sentRecCount);
    free(totalRecCount);
    free(p);
    free(emptyString);
    exit(0);
}

int cmp(const void* a, const void* b){
    const report_request_buf *aP = (report_request_buf *)a;
    const report_request_buf *bP = (report_request_buf *)b;
    if (aP->report_idx < bP->report_idx) return -1;
    else if (aP->report_idx > bP->report_idx) return 1;
    return 0;
}

void hookSignal(){
    // man signal (2) says to avoid using signal() and to use sigaction() instead.
    struct sigaction sA;
    sA.sa_handler = sigHandler;
    sigemptyset(&sA.sa_mask);
    assert(sigaction(SIGINT, &sA, NULL) == 0);
}

void sigHandler(int signo) {
    // I chose to use sem_post here since it is required to be async-signal-safe by POSIX standards,
    // so it can be safely called inside a signal handler.
    // - from man signal(7)
    sem_post(&lock_data.cond);
}

void printRRBuf(const report_request_buf *buffer){
    fprintf(stderr, "TYPE: %ld, ID: %d, REPORTCOUNT: %d, STRING: %s\n", buffer->mtype, buffer->report_idx, buffer->report_count, buffer->search_string);
}

// this will access shared data, ONLY USE THIS WHEN ITS LOCKED
void printReport(void *args){
    parameters *p =  (parameters*) args;
    fprintf(stdout, "***Report***\n");
    fprintf(stdout, "%d records read for %d reports\n", *p->totalRecCount, p->size);
    for (int i = 0; i < p->size; ++i){
        fprintf(stdout, "Records sent for report index %d: %d\n", i + 1, p->recordCounts[i]);
    }
}

void *waitForSignal(void *args){
    hookSignal();
    while (1){ // to make it work for multiple ctrl+c presses
        sem_wait(&lock_data.cond);
        if (lock_data.cond_value) break; // the program is done so exit the loop
        sem_wait(&lock_data.mutex);
        printReport(args);
        sem_post(&lock_data.mutex);
    }
    return NULL;
}

void updateQueueId(const int queueNum, key_t *key, int *msqid, const int msgflg){
    *key = ftok(FILE_IN_HOME_DIR, queueNum);
    if (*key == 0xffffffff) {
        fprintf(stderr,"Key cannot be 0xffffffff..fix queue_ids.h to link to existing file\n");
        exit(1);
    }
    if ((*msqid = msgget(*key, msgflg)) < 0) {
        int errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("(msgget)");
        fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
    }
    else
        fprintf(stderr, "msgget: msgget succeeded: msgqid = %d\n", *msqid);
}

void getMessage(const int msqid, report_request_buf *rbuf){
    int ret;
    do {
        ret = msgrcv(msqid, rbuf, sizeof(*rbuf), 1, 0);//receive type 1 message
        int errnum = errno;
        if (ret < 0 && errno !=EINTR){
            fprintf(stderr, "Value of errno: %d\n", errno);
            perror("Error printed by perror");
            fprintf(stderr, "Error receiving msg: %s\n", strerror( errnum ));
        }
        } while ((ret < 0 ) && (errno == 4)); // go until it does not fail; 4 == EINTR == interrupted system call
        //printRRBuf(rbuf);
    fprintf(stderr,"process-msgrcv-request: msg type-%ld, Record %d of %d: %s ret/bytes rcv'd=%d\n", rbuf->mtype, rbuf->report_idx,rbuf->report_count,rbuf->search_string, ret);
    
}

void sendMessage(const int msqid, report_record_buf *sbuf, const char *string){
    sbuf->mtype = 2;
    strcpy(sbuf->record, string); 
    size_t buf_length = strlen(sbuf->record) + sizeof(int)+1;//struct size without
    // Send a message.
    if((msgsnd(msqid, sbuf, buf_length, IPC_NOWAIT)) < 0) {
        int errnum = errno;
        fprintf(stderr,"%d, %ld, %s %d\n", msqid, sbuf->mtype, sbuf->record, (int)buf_length);
        perror("(msgsnd)");
        fprintf(stderr, "Error sending msg: %s\n", strerror( errnum ));
        exit(1);
    }
    else
        fprintf(stderr,"msgsnd-report_record: record\"%s\" Sent (%d bytes)\n", sbuf->record,(int)buf_length);
}
