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
void signal_gen();
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
    srand(1);
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

    // what are these used for?
    int *gen_pids[3]; // change how this is implemented
    int *hand_pids[4];
    int *rep_pid; 
    for(int i = 0; i < 8; i++){
        int gen_i = 0;
        int hand_i = 0;
        if(i == 0 || i == 1 || i == 2){ // generate signals
            pid = fork(); // create child
            if(pid == -1){ // error check
                puts("Sig Gen fork failed");
            }else if(pid == 0){ // child process
                // sig_gen();
                puts("Signals generate process");
                exit(0);
            }else{
                gen_pids[gen_i] = &pid; // should I use a pointer to the pid?
                gen_i++;
            }
        }
        else if(i == 3 || i == 4 ){ // handle sig 1
            pid = fork();
            if(pid == -1){
                puts("SIGUSR1 Handler fork failed");
            }else if(pid == 0){ // handler sigusr1 process
                // handle_sigusr1();
                puts("Signal usr1 recieve process");
                exit(0);
            }else{ // parent for hand sig1
                hand_pids[hand_i] = &pid;
                hand_i++;
            }
        }
        else if(i==5 || i == 6){ // handle sig 2 
            pid = fork();
            if(pid == -1){
                puts("SIGUSR2 Handler fork failed");
            }else if(pid == 0){ // handler sigusr2 process
                // handle_sigusr2();
                puts("Signals usr2 recieve process");
                exit(0);
            }else{
                hand_pids[hand_i] = &pid;
                hand_i++;
            }
        }
        else if(i == 7){ // report
            pid = fork();
            if(pid == -1){
                printf("Report fork failed");
            }else if(pid == 0){
                // report();
                exit(0);
            }else{
                rep_pid = &pid;
            }
        }
        block_sigusrs(0);
    }

    sleep(5);
    printf("Complete!!!\n");
    printf("SIGUSR1 SENT COUNTER: %d\n", shmem->sigusr1_sent_counter);
    printf("SIGUSR2 SENT COUNTER:  %d\n", shmem->sigusr2_sent_counter);
    printf("SIGUSR1 RECIEVE COUNTER: %d\n", shmem->sigusr2_recieve_counter);
    printf("SIGUSR2 RECIEVE COUNTER:  %d\n", shmem->sigusr2_recieve_counter);
    shmdt(shmem);
    exit(0);

    // block_sigusrs(0);
    // // creating muliple children code
    // pid_t pids[8];
    // for(int i = 0; i < 8; ++i){
    //     pids[i] = fork();
    //     if(pids[i] == -1){
    //         printf("Parent failed to fork...\n");
    //     }
    //     else if(pids[i] == 0){
    //         // puts("This is the child");
    //         // run code depending on the value of i
    //         // the value of i determines what kind of process it is
    //         if(i == 0 || i == 1 || i == 2){
    //             // signal generating processes
    //             puts("Signal sent...");
    //             // block_sigusrs(0);
    //             signal_gen();
    //             exit(0);
    //         }
    //         else if(i == 3 || i == 4){
    //             // signal recieving processes
    //             // signal_handler(pids[i]);
    //             puts("Signal 1 recieved...");
    //             // block_sigusrs(2);
    //             handle_sigusr1();
    //             // while(signal(SIGUSR1, signal_handler) != SIG_ERR){
    //             //     // puts("Error recieving signal");
    //             // }
    //             exit(0);
    //         }else if(i == 5 || i == 6){
    //             puts("Signal 2 recieved...");
    //             // block_sigusrs(1);
    //             // while(signal(SIGUSR2, signal_handler) != SIG_ERR){
    //             //     // puts("Error recieving signal");
    //             // }
    //             handle_sigusr2();
    //             exit(0);
    //         }
    //         else if(i == 7){
    //             // reporting processs
    //             report();
    //             exit(0);
    //         }
    //     } 
        
    //     else {
    //         // continue until i == 7, then wait
    //         // puts("This is the parent");
    //         sleep(1); // give child time to spawn
    //         // parent_func(pids[i]); // not sure if that param is right. It wasn't on the slides but I think it is right
    //         // puts("Reporting...");
    //         waitpid(pids[i], NULL, 0); 
    //     }
        
        
    // }
}


void signal_gen(){
    int count = 0;
    while(count < 10){
        // puts("Reached");
        block_sigusrs(0);
        sleep_random_interval(.01, .1); // THIS STILL NEEDS WORK
        // int signalInd = randomSignal();
        int signalInd = randomSignal();
        if(signalInd == 1){
            puts("Sending signal 1");
            block_sigusrs(1);
            // signal(SIGUSR1, signal_handler);
            kill(0, SIGUSR1);
            pthread_mutex_lock(&shmem->sigusr1_sent_mutex);
            shmem->sigusr1_sent_counter++; // need to init this in main
            // printf("SIGUSR1 signal sent count: %d\n ", shmem->sigusr1_sent_counter);
            pthread_mutex_unlock(&shmem->sigusr1_sent_mutex);
        }
        else if(signalInd == 0){
            puts("Sending signal 2");
            block_sigusrs(2);
            // signal(SIGUSR2, signal_handler);
            kill(0, SIGUSR2);
            // puts("reached post sig 2");
            pthread_mutex_lock(&shmem->sigusr2_sent_mutex);
            shmem->sigusr2_sent_counter++; // need to init this in main
            // printf("SIGUSR2 signal sent count: %d\n", shmem->sigusr2_sent_counter);
            pthread_mutex_unlock(&shmem->sigusr2_sent_mutex);
        }
        count++;
        // kill(0, signal); // remember that putting 0 for the pid sends a signal to everyone within a process group as per slides
    }
}

int randomSignal(){
    // holder value for now
    return 0;
}

void signal_handler(int sig){
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
}

void handle_sigusr1(){
    
    block_sigusrs(2);
    signal(SIGUSR1, signal_handler);
    // while(1) {
    //      pause();
    // }
}

void handle_sigusr2(){
    
    block_sigusrs(1);
    signal(SIGUSR2, signal_handler);
    // while(1){
    //     pause();
    // }
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
    puts("Reporting");
}


void sleep_random_interval(double min, double max){
    // int random  = randomGen(min, max);
    // seconds to nanoseconds
    // int nanoMin = min * 1000000000; // this method does not work
    // printf("Nano min: %d",nanoMin);
    // nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
    // sleep(1);
}

int randomGen(int min, int max){
    int random = rand() % (max + min);
    return random;
}


// void printReport() {

// 	struct timespec startTime, currTime;
//     int rep_sigusr1_counter = 0;
//     int rep_sigusr2_counter = 0;
//     int total;
//     long sigusr1Times[10]; // 10 because once filled you report 
//     long sigusr2Times[10];
//     double elapsedTime;
//     long secs;
//     long nanosecs;

// 	clock_gettime(CLOCK_REALTIME, &currTime);
// // //	seconds = currentTime.tv_sec - start.tv_sec;
// 	nanosecs = currTime.tv_nsec - startTime.tv_nsec;
    
// // 	totalTime_1 = sigusr1Times[rep_sigusr1_counter -1 ] - sig1Times[0];
// // 	avgTime_1 = (double)totalTime_1 / (double)report_sig1Count;

// //     totalTime_2 = sigusr2Times[rep_sigusr2_counter -1 ] - sigusr2Times[0];
// //     avgTime_2 = (double)totalTime_2 / (double)rep_sigusr2_counter;

// 	printf("Simulation Time: %e\n", timeElapsed);
// 	printf("Signal 1 Count: %d\n", report_sig1Count);
// 	printf("Signal 1 Average Time: %e\n", avgTime_1);
// 	printf("Signal 2 Count: %d\n", rep_sigusr2_counter);
//     printf("Signal 2 Average Time:%e\n", avgTime_2); 

// }
