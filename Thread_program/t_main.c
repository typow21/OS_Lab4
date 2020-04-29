// Tyler Powell
// Lab 4: Signaling with multi-process and multi-threaded programs
// MULTI-THREAD PROGRAM

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h> 
#include <time.h> 
#include <signal.h>


// protos 
void *signal_gen();
void signal_handler(int sig);
int randomGen(int min, int max);
void sleep_random_interval(double min, double max);
int randomSignal();
void block_sigusrs(int sig);

void signal_handler();
void *handle_sigusr1();
void *handle_sigusr2();
void *report();


struct timespec startTime, currTime;
int rep_sigusr1_counter = 0;
int rep_sigusr2_counter = 0;
int total;
long sigusr1Times[10]; // why 10?
long sigusr2Times[10];
double elapsedTime;
long secs;
long nanosecs;



int sigusr1_sent_counter;
int sigusr2_sent_counter;
int sigusr1_recieve_counter;
int sigusr2_recieve_counter;
pthread_mutex_t sigusr1_sent_mutex;
pthread_mutex_t sigusr1_recieve_mutex;
pthread_mutex_t sigusr2_sent_mutex;
pthread_mutex_t sigusr2_recieve_mutex;



int main(){
    // srand(1);
    // init variables
    // sigusr1_sent_counter = 0;
    // sigusr2_sent_counter = 0; 
    // sigusr1_recieve_counter = 0;
    // sigusr2_recieve_counter = 0;

    // init mutex
    // pthread_mutexattr_t(attr);
     // do I need multiple attr? ( try without and see what happens )
    pthread_mutex_init(&sigusr1_sent_mutex, NULL);
    pthread_mutex_init(&sigusr2_sent_mutex, NULL);
    pthread_mutex_init(&sigusr1_recieve_mutex, NULL);
    pthread_mutex_init(&sigusr2_recieve_mutex, NULL);
    
    int tid;
	pthread_t gen_threads[3];
    pthread_t handle_threads[4];
    pthread_t reporter_thread;
	for(int i = 0; i < 3; i++) {
		if(pthread_create(&gen_threads[i],NULL, signal_gen, NULL) == -1) {
			puts("Generation thread creation failed");
		} 
	}

    // handle thread 1
	for(int i = 0; i < 2; i++) {
		if(pthread_create(&handle_threads[i],NULL, handle_sigusr1, NULL) == -1) { 
			puts("Handler 1 fork failed");
		} 
    }
    for(int i = 0; i < 2; i++) {
		if(pthread_create(&handle_threads[i],NULL, handle_sigusr2, NULL) == -1) { 
			puts("Handler 2 fork failed");
		}
    }
	for(int i = 0; i < 2; i++) {
		if(pthread_create(&handle_threads[i],NULL, handle_sigusr1, NULL) == -1) { 
			puts("Handler fork failed");
		} 
    }
	if(pthread_create(&reporter_thread,NULL, report, NULL) == -1) {
		puts("Report fork failed");
	}
    // 
    // sleep(5);
    pthread_exit(NULL); 
    exit(0);    
}

// signal generator thread
void *signal_gen(){
    int count = 0;
    sleep(1);
    block_sigusrs(0);
    while(count < 10){
        // puts("Reached");
        sleep_random_interval(.01, .1); // THIS STILL NEEDS WORK
        // int signalInd = randomSignal();
        // int signalInd = rand()%2;
        int signalInd = randomGen(1,2);
        if(signalInd == 1){
            puts("Sending signal 1");
            block_sigusrs(1);
            kill(0, SIGUSR1);
            fflush(stdout);
            pthread_mutex_lock(&sigusr1_sent_mutex);
            sigusr1_sent_counter++; // need to init this in main
            // printf("SIGUSR1 signal sent count: %d\n ", sigusr1_sent_counter);
            pthread_mutex_unlock(&sigusr1_sent_mutex);
        }
        else if(signalInd == 0){
            puts("Sending signal 2");
            block_sigusrs(2);
            kill(0, SIGUSR2);
            fflush(stdout);
            // // puts("reached post sig 2");
            pthread_mutex_lock(&sigusr2_sent_mutex);
            sigusr2_sent_counter++; // need to init this in main
            printf("SIGUSR2 signal sent count: %d\n", sigusr2_sent_counter);
            pthread_mutex_unlock(&sigusr2_sent_mutex);
        }
        count++;
        // sleep(1);
        // kill(0, signal); // remember that putting 0 for the pid sends a signal to everyone within a process group as per slides
    }
    exit(0);
}

void *handle_sigusr1(){
    puts("Reached handle sigusr1");
    // block_sigusrs(2);
    signal_handler(SIGUSR1);
    while(1) {
         pause();
    }
}

void *handle_sigusr2(){
    puts("Reached handle sigusr2");
    // block_sigusrs(1);
    // signal(SIGUSR2, signal_handler);
    signal_handler(SIGUSR2);
    while(1){
        pause();
    }
}

void block_sigusrs(int sig) {
    //  0 is both
    // 1 is sigusr1
    // 2 is sigusr2
	sigset_t sigset;
	sigemptyset(&sigset);// initalize set to empty
    if(sig == 0){
        sigaddset(&sigset, SIGUSR1);
        sigaddset(&sigset, SIGUSR2);
    }else if(sig == 1){
        sigaddset(&sigset, SIGUSR1);
    }else if(sig == 2){
        sigaddset(&sigset, SIGUSR2);
    }else{
        puts("Incorrect input");
    }
	sigprocmask(SIG_BLOCK, &sigset, NULL);// modify mask
}

// TODO
void *report(){
    printf("Complete!!!\n");
    printf("SIGUSR1 SENT COUNTER: %d\n", sigusr1_sent_counter);
    printf("SIGUSR2 SENT COUNTER:  %d\n", sigusr2_sent_counter);
    printf("SIGUSR1 RECIEVE COUNTER: %d\n", sigusr2_recieve_counter);
    printf("SIGUSR2 RECIEVE COUNTER:  %d\n", sigusr2_recieve_counter);
    puts("Reporting");
    exit(0);
}


// Helper functions
void sleep_random_interval(double min, double max){
 
}

int randomGen(int min, int max){
    int random = rand() % (max + min);
    return random;
}

void signal_handler(int sig){
    puts("Reached signal handler");
    if(sig == SIGUSR1){
        puts("Recieving signal 1");
        pthread_mutex_lock(&sigusr1_recieve_mutex);
        sigusr1_recieve_counter++; // need to init this in main
        printf("SIGUSR1 signal recieved count: %d\n ", sigusr1_recieve_counter);
        pthread_mutex_unlock(&sigusr1_recieve_mutex);
    }else if(sig == SIGUSR2){
        puts("Recieving signal 2");
        pthread_mutex_lock(&sigusr2_recieve_mutex);
        sigusr2_recieve_counter++; // need to init this in main
        printf("SIGUSR1 signal recieved count: %d\n ", sigusr2_recieve_counter);
        pthread_mutex_unlock(&sigusr2_recieve_mutex);
    }else{
        puts("Error handling signal");
        exit(0);
    }
    while(1){
        pause();
    }
}