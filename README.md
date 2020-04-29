# OS Lab 4: Signaling with multi-process and multi-threaded programs

Signaling with multi-process and multi-threaded programs


This project contains two seperate programs that share much of the same functionality. This overall idea of the programs is to run analysis of the performance in sending signals between processes or threads (depending on the program). That is where the differences between the two programs comes in. 


**Multiprocess signaling**


For this program there is a single parent process with: 3 signal generating child processes, 4 signal recieving child processes, and a reporting child process. 


The 3 signal generating processes will execute in a loop sending out SIGUSR1 and SIGUSR2 signals. Once the signals get sent out they increment a counter that is stored in a shared memory region. After those 3 processes are created 4 signal handling processes are created. These processes also run in a loop waiting for a signal to be sent so that it can recieve it. Once a the signal handler recieves the sent signal it increments the shared counter. The final process that gets created is the reporting process. This process runs in a loop as well. While in the loop it waits until it recieves a signal. Once it does, it keeps track of a few statistics about the signal that was recieved. Every 10 signals that it recieves it prints a report to the shell. 


This program runs until either 1) 30 seconds goes by or 2) 100,000 signals are sent. After the program runs you can see a more detailed report of your analytics. 


**Multithread signaling**

This program is nearly identical to the previous program. The only difference is that it is being made using threads instead of processes.


**Testing**


I plan to test these programs first by printing each report thoroughly and by looking for any inconsistancies/anomolies. After chekcing for any blatant errors I then plan to compare the results from both programs, once again checking for inconsistancies. If any are found I plan to add identical print statement throughout each program to further attempt to find the problem. After this process is done I should be able to clearly demonstrate that the program executes correctly. 


Unfortunately I did not have time to execute my testing plan. But there it is none the less. :)
