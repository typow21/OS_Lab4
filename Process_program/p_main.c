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

void signal_gen();
void sig_handle();
int randomGen(int min, int max);
void sleep_random_interval(int min, int max);

struct shared_mem {
    int sigusr1_sent_counter;
    int sigusr2_sent_counter;
    int sigusr1_recieve_counter;
    int siguser2_recieve_counter;
    pthread_mutex_t sigusr1_sent_mutex;
    pthread_mutex_t sigusr1_recieve_mutex;
    pthread_mutex_t sigusr2_sent_mutex;
    pthread_mutex_t sigusr1_recieve_mutex;
}*shmem;

int main(){
    // create the shared memory region here
    int shm_id; // shared memory id
    shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_mem), IPC_CREAT | 0666); // created shared mem region
    assert(shm_id >= 0); // error check memory creation
    shmem = (struct shared_mem *) shmat(shm_id, NULL, 0); // attach memory
    assert(shmem != (struct shared_mem *) -1); // error check memory attachment
    // init variables
    shmem->sigusr1_sent_counter;
    shmem->sigusr2_sent_counter;
    shmem->sigusr1_recieve_counter;
    shmem->siguser2_recieve_counter;

    // init mutex
    // pthread_mutexattr_t(attr);
     // do I need multiple attr? ( try without and see what happens )
    pthread_mutexattr_t attr;
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(shmem->sigusr1_sent_mutex), &attr);
    pthread_mutex_init(&(shmem->sigusr2_sent_mutex), &attr);
    pthread_mutex_init(&(shmem->sigusr1_recieve_mutex), &attr);
    pthread_mutex_init(&(shmem->sigusr2_recieve_mutex), &attr);

    // creating muliple children code
    pid_t pids[8];
    for(int i = 0; i < 8; ++i){
        pids[i] = fork();
        if(pids[i] == 0){
            // run code depending on the value of i
            // the value of i determines what kind of process it is
            if(i == 0 || i == 1 || i == 2){
                // signal generating processes
                puts("Signal sent...")
                // signal_gen();
            }
            else if(i == 3 || i == 4 || i == 5 || i == 6){
                // signal recieving processes
                // signal_handler(pids[i]);
                puts("Signal recieved...");
            }
            else if(i == 7){
                // reporting process
            }
            //the code below my be wrong?
            puts("This is the child");

            // what does this code do? 
            // signal(SIGUSR1, signal_handler); 
            // signal(SIGUSR2, signal_handler);

            while(1){
                sleep(1);
            }
            shmdt(shmem);
            exit(0);
        } 
        
        else {
            // continue until i == 7, then wait
            sleep(1); // give child time to spawn
            parent_func(pids[i]); // not sure if that param is right. It wasn't on the slides but I think it is right
            waitpid(pids[i], NULL, 0); 
        }
    }

    for(int j = 0 ; j < 8; ++j){
        kill(pids[j], SIGTERM);
    }
}


void signal_gen(){
    while(1){
        sleep_random_interval(.01, .1); // THIS STILL NEEDS WORK
        
        int signalInd = randomsignal();
        if(signalInd == 1){
            kill(0, SIGUSR1);
        }
        else if(signalInd == 0){
            kill(0, SIGUSR2);
        }
        kill(0, signal); // remember that putting 0 for the pid sends a signal to everyone within a process group as per slides
        if(signal == SIGUSR1){
            // lock_aquire();
            pthread_mutex_lock(&shmem->sigusr1_sent_mutex);
            shmem->sigusr1_sent_counter++; // need to init this in main
            pthread_mutex_unlock(&shmem->sigusr1_sent_mutex);
            // lock_release();
        }else{
            pthread_mutex_lock(&shmem->sigusr2_sent_mutex);
            shmem->sigusr2_sent_counter++; // need to init this in main
            pthread_mutex_unlock(&shmem->sigusr2_sent_mutex);
        }
    }
}

void signal_handler(int sig){
    if(sig == SIGUSR1){
        puts("child recieved SIGUSR1");
    }else{
        puts("child recieved SIGUSR2");
        fflush(stdout);
    }
    exit(0);
}

void sleep_random_interval(int min, int max){
    int random  = randomGen(min, max);
    // seconds to nanoseconds
    int nanoMin = min * 1000000000; // this method does not work
    printf("Nano min: %d",nanoMin);
    nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
}

int randomGen(int min, int max){
    int random = rand() % (max + min);
    return random;
}