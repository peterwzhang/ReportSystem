#include <assert.h>
#include <bits/pthreadtypes.h>
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

condlock lock_data;

void sig_handler(int signo) {
    // sem_post required to be async-signal-safe by POSIX standards.
    sem_post(&lock_data.cond);
}

void print_rr_buf(const report_request_buf *buffer){
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

void *wait_for_signal(void *args){
    signal(SIGINT, sig_handler);
    while (!lock_data.cond_value){ // to make it work for multiple ctrl+c presses
        sem_wait(&lock_data.cond);
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
        //print_rr_buf(rbuf);
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


int main(int argc, char**argv)
{
    //TODO: add checks
    sem_init(&lock_data.mutex, 0, 1);
    sem_init(&lock_data.cond, 0, 0);
    lock_data.cond_value = 0;

    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    report_request_buf *rbufArr = malloc(sizeof *rbufArr);
    assert(rbufArr != NULL);

    updateQueueId(QUEUE_NUMBER, &key, &msqid, msgflg);
    getMessage(msqid, rbufArr);

    //print_rr_buf(&rbuf);
    int totalCount = rbufArr->report_count;
    rbufArr = realloc(rbufArr, totalCount * (sizeof *rbufArr));
    assert(rbufArr != NULL);
    for (int i = 1; i < totalCount; ++i){ // get the rest of the messages
        getMessage(msqid, &rbufArr[i]);
    }

    for (int i = 0; i < totalCount; ++i){
        print_rr_buf(&rbufArr[i]);
    }

    // set up shared array on heap
    int *sentRecCount = calloc(totalCount, sizeof *sentRecCount);
    assert(sentRecCount != NULL);

    // create our printing thread
    pthread_t printThread;
    int *totalRecCount = malloc(sizeof *totalRecCount);
    assert(totalRecCount!= NULL);
    *totalRecCount = 0;
    parameters *p = malloc(sizeof *p);
    assert(p != NULL);
    p->recordCounts = sentRecCount;
    p->size = totalCount;
    p->totalRecCount = totalRecCount;
    pthread_create(&printThread, NULL, (void*) wait_for_signal, (void*) p);
    // hook sigint
    //signal(SIGINT, sig_handler);

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
                    updateQueueId(i + 1, &key, &msqid, msgflg);
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
    sem_post(&lock_data.cond);
    sem_post(&lock_data.mutex);
    pthread_join(printThread, NULL); // wait for final print to finish
    free(rbufArr);
    free(sentRecCount);
    free(totalRecCount);
    free(p);
    free(emptyString);
    exit(0); // return from main thread will kill all child threads
}