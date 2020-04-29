// Tyler Powell
// Lab 4: Signaling with multi-process and multi-threaded programs
// MULTI-PROCESS PROGRAM

#include <time.h>
#include <sys/times.h>
#include <stdint.h>	/* for uint64 definition */
#include <stdlib.h>	/* for exit() definition */
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

// protos 
void signal_gen(int pid);
void signal_handler(int sig);
int randomGen(int min, int max);
void sleep_random_interval(double min, double max);
int randomSignal();
void block_sigusrs(int sig);

void signal_handler();
void handle_sigusr1();
void handle_sigusr2();
void report();

// globals variables for reporter process
struct timespec startTime, currTime;
int rep_sigusr1_counter = 0;
int rep_sigusr2_counter = 0;
int total;
long sigusr1Times[10]; // why 10?
long sigusr2Times[10];
double elapsedTime;
long secs;
long nanosecs;


struct shared_mem {
    int sigusr1_sent_counter;
    int sigusr2_sent_counter;
    int sigusr1_recieve_counter;
    int sigusr2_recieve_counter;
    pthread_mutex_t sigusr1_sent_mutex;
    pthread_mutex_t sigusr1_recieve_mutex;
    pthread_mutex_t sigusr2_sent_mutex;
    pthread_mutex_t sigusr2_recieve_mutex;
}*shmem;


int main(){
    // srand(1);
    // create the shared memory region here
    int shm_id; // shared memory id
    shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_mem), IPC_CREAT | 0666); // created shared mem region
    assert(shm_id >= 0); // error check memory creation
    shmem = (struct shared_mem *) shmat(shm_id, NULL, 0); // attach memory
    assert(shmem != (struct shared_mem *) -1); // error check memory attachment
    // init variables
    shmem->sigusr1_sent_counter = 0;
    shmem->sigusr2_sent_counter = 0; 
    shmem->sigusr1_recieve_counter = 0;
    shmem->sigusr2_recieve_counter = 0;

    // init mutex
    // pthread_mutexattr_t(attr);
     // do I need multiple attr? ( try without and see what happens )
    pthread_mutexattr_t attr;
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(shmem->sigusr1_sent_mutex), &attr);
    pthread_mutex_init(&(shmem->sigusr2_sent_mutex), &attr);
    pthread_mutex_init(&(shmem->sigusr1_recieve_mutex), &attr);
    pthread_mutex_init(&(shmem->sigusr2_recieve_mutex), &attr);
    
    int pid;
	int* gen_pids[3];
    int* handle_pids[4];
    int* reporter_id;
	for(int i = 0; i < 3; i++) {
		pid = fork();
		if(pid == -1) {
			puts("Generation fork failed");
		} else if (pid == 0) {
			signal_gen(pid);
			exit(0);
		} else {
            block_sigusrs(0);
			gen_pids[i] = &pid;
            waitpid(pid, NULL, 0);
		}
	}
	for(int i = 0; i < 4; i++) {
		if(pid == -1) { 
			puts("Handler fork failed");
		} else if(pid == 0) { //child, handler
			if(i < 2) {
                // puts("Reached signal handle main 1");
                // signal(SIGUSR1, signal_handler);
                // signal_handler(SIGUSR1);
				handle_sigusr1(); 
			} else {
				handle_sigusr2();
			}
			exit(0);
		} else { 
            handle_pids[i] = &pid;
            waitpid(pid, NULL, 0);
		}
    }
	
	pid = fork();
	if(pid == -1) {
		puts("Report fork failed");
	} else if (pid == 0) { //child, handler
		report();
		exit(0);						
	} else { //parent, main process
		reporter_id = &pid;
        waitpid(pid, NULL, 0);
	}
    // 
    // sleep(5);
    shmdt(shmem);
    exit(0);    
}

void signal_gen(int pid){
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
            kill(pid, SIGUSR1);
            fflush(stdout);
            pthread_mutex_lock(&shmem->sigusr1_sent_mutex);
            shmem->sigusr1_sent_counter++; // need to init this in main
            // printf("SIGUSR1 signal sent count: %d\n ", shmem->sigusr1_sent_counter);
            pthread_mutex_unlock(&shmem->sigusr1_sent_mutex);
        }
        else if(signalInd == 0){
            puts("Sending signal 2");
            block_sigusrs(2);
            kill(pid, SIGUSR2);
            fflush(stdout);
            // // puts("reached post sig 2");
            pthread_mutex_lock(&shmem->sigusr2_sent_mutex);
            shmem->sigusr2_sent_counter++; // need to init this in main
            printf("SIGUSR2 signal sent count: %d\n", shmem->sigusr2_sent_counter);
            pthread_mutex_unlock(&shmem->sigusr2_sent_mutex);
        }
        count++;
        // sleep(1);
        // kill(0, signal); // remember that putting 0 for the pid sends a signal to everyone within a process group as per slides
    }
    exit(0);
}

int randomSignal(){
    // holder value for now
    return 0;
}

void signal_handler(int sig){
    puts("Reached signal handler");
    if(sig == SIGUSR1){
        puts("Recieving signal 1");
        pthread_mutex_lock(&shmem->sigusr1_recieve_mutex);
        shmem->sigusr1_recieve_counter++; // need to init this in main
        printf("SIGUSR1 signal recieved count: %d\n ", shmem->sigusr1_recieve_counter);
        pthread_mutex_unlock(&shmem->sigusr1_recieve_mutex);
    }else if(sig == SIGUSR2){
        puts("Recieving signal 2");
        pthread_mutex_lock(&shmem->sigusr2_recieve_mutex);
        shmem->sigusr2_recieve_counter++; // need to init this in main
        printf("SIGUSR1 signal recieved count: %d\n ", shmem->sigusr2_recieve_counter);
        pthread_mutex_unlock(&shmem->sigusr2_recieve_mutex);
    }else{
        puts("Error handling signal");
        exit(0);
    }
    while(1){
        pause();
    }
}

void handle_sigusr1(){
    puts("Reached handle sigusr1");
    // block_sigusrs(2);
    signal_handler(SIGUSR1);
    while(1) {
         pause();
    }
}

void handle_sigusr2(){
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
void report(){
    printf("Complete!!!\n");
    printf("SIGUSR1 SENT COUNTER: %d\n", shmem->sigusr1_sent_counter);
    printf("SIGUSR2 SENT COUNTER:  %d\n", shmem->sigusr2_sent_counter);
    printf("SIGUSR1 RECIEVE COUNTER: %d\n", shmem->sigusr2_recieve_counter);
    printf("SIGUSR2 RECIEVE COUNTER:  %d\n", shmem->sigusr2_recieve_counter);
    puts("Reporting");
}


void sleep_random_interval(double min, double max){
 
}

int randomGen(int min, int max){
    int random = rand() % (max + min);
    return random;
}