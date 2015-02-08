#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <syscall.h>
#include <time.h>
#include <signal.h>
#include <setjmp.h>

#define NO_OF_THREAD 20

pthread_cond_t cond_variable_to_act_all = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_variable_for_event_of_aperiodic[2];
pthread_mutex_t mutex_for_wait = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_task[10];
pthread_mutex_t mutex_cond_var_task[2];

pthread_t thread[NO_OF_THREAD];
pthread_attr_t attr[NO_OF_THREAD];
struct sched_param  param[NO_OF_THREAD];



int termination_all_threads = 0;

struct input_event ie;

typedef struct task_struct{
   char type;
   int priority;
   int period;
   int iteration;
   int lock[10];
   int unlock[10];
   int no_it_lock[10];
   int no_it_unlock[10];   
   int event_number;
}TASK;



void *periodic_task(void* fun){

   struct timespec tstart,tend;
   float computation_time;


   int message = 1;
   int i, j=0, period, iteration, lcount=0;
   TASK *funct = fun;
   period = funct->period;
   iteration = funct->iteration;


   pthread_mutex_lock(&mutex_for_wait);
   pthread_cond_wait(&cond_variable_to_act_all, &mutex_for_wait); // cond wait for activation of this task.
   pthread_mutex_unlock(&mutex_for_wait);


if(!termination_all_threads){
      clock_gettime(CLOCK_REALTIME, &tstart);
      printf("Period value in function 1 is : %d  and iteration is : %d\n",funct->period, funct->iteration);
      for(i=0; i<iteration; i++){
         printf("Thread b4 crearted in periodic task..................: %d from %d\n", j, message);
         j = j + 1;
      }
      i = j = 0;

      while(funct->lock[lcount] !=0 && funct->lock[lcount] <= 10){
         iteration = funct->no_it_lock[lcount];
         pthread_mutex_lock(&mutex_task[funct->lock[lcount]]);
         for(i=0; i<iteration; i++){
            j = j + 1;
            printf("Thread in mutex locked  after lock in periodic task..................: %d \n", j);
         }
         lcount++;
         j=0;
      }
      lcount = 0;
      while(funct->unlock[lcount] !=0 && funct->unlock[lcount] <= 10){
         iteration = funct->no_it_unlock[lcount];
         pthread_mutex_unlock(&mutex_task[funct->lock[lcount]]);         
         for(i=0; i<iteration; i++){
            j = j + 1;
            printf("Thread in mutex unlocked in periodic task..................: %d \n", j);
         }
         lcount++;
         j=0;
      }
      clock_gettime(CLOCK_REALTIME, &tend);
      computation_time = ((tend.tv_sec - tstart.tv_sec)/1000 + ((float)(tend.tv_nsec - tstart.tv_nsec))/1000000);
      printf("Computation time is %f\n", computation_time);
      if(period> computation_time)   usleep(1000*((float)period - computation_time));
   }   

}


void *aperiodic_task(void* fun){
      int message = 2;
      int i, iteration, j=0, lcount=0;
      TASK *funct = fun;
      iteration = funct->iteration;
      int event_num = funct->event_number;

   

   pthread_mutex_lock(&mutex_for_wait);
   pthread_cond_wait(&cond_variable_to_act_all, &mutex_for_wait); // cond wait for activation of this task.
   pthread_mutex_unlock(&mutex_for_wait);
   

   if(event_num == mouse_eve()){
/*
         pthread_mutex_lock(&mutex_cond_var_task[event_num]);
         pthread_cond_wait(&cond_variable_for_event_of_aperiodic[event_num],&mutex_cond_var_task[event_num]);
         pthread_mutex_unlock(&mutex_cond_var_task[event_num]);
*/
         printf("Number of iteration in function 2 is : %d \n",funct->iteration);
         for(i=0; i<iteration; i++){
            j = j + 1;
            printf("Thread b4 crearted. in aperiodic.............: %d from %d \n", j, message);
         }
         i = j =0;


         while(funct->lock[lcount] !=0 && funct->lock[lcount] <= 10){
            iteration = funct->no_it_lock[lcount];
            pthread_mutex_lock(&mutex_task[funct->lock[lcount]]);
            for(i=0; i<iteration; i++){
               j = j + 1;
               printf("Thread in mutex locked in aperiodic task..................: %d \n", j);
            }
//            pthread_mutex_unlock(&mutex_task[funct->lock[lcount]]);
            lcount++;
         }
         while(funct->unlock[lcount] !=0 && funct->unlock[lcount] <= 10){
            iteration = funct->no_it_unlock[lcount];
            pthread_mutex_unlock(&mutex_task[funct->lock[lcount]]);         
            for(i=0; i<iteration; i++){
               j = j + 1;
               printf("Thread in mutex unlocked in periodic task..................: %d \n", j);
          }
         lcount++;
         j=0;
         } 

 
               
   }


}



int mouse_eve(){
   int fd;
      printf("Hello \n");
      fd = open("/dev/input/event3", O_RDONLY);
      printf("Line number is %d \n", __LINE__);
      if(fd == -1){
         printf("Failed read operation\n");
         exit(EXIT_FAILURE);
      }
      while(1){
         while(read(fd, &ie, sizeof(struct input_event))){

            if(ie.type == EV_KEY){
               if(ie.code == BTN_LEFT && ie.value == 0){
                  printf("Left BTN pressed: %d \n", ie.value);
//                  pthread_cond_broadcast(&cond_variable_for_event_of_aperiodic[0]);
                  return 0;
               }
               else if(ie.code == BTN_RIGHT && ie.value == 0){
                  printf("Right BTN pressed: \n");
//                  pthread_cond_broadcast(&cond_variable_for_event_of_aperiodic[1]);
                  return 1;
               }
            }
         }

      }
      return 0;
}


int main(){
	FILE *fr;

	int n;
	char line[80], test[80];
 	char *token;
 	int no_of_thr;
   TASK *task_array;
 	int exec_time;
 	int ret, i;
 	char task_type;

 	int line_count =0, index = 0;
   
   pthread_mutexattr_t mutex_attr;
   pthread_mutexattr_init(&mutex_attr); 
//   pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_INHERIT);

   for(i=0; i<10; i++){
      ret = pthread_mutex_init(&mutex_task[i], &mutex_attr);
      if(ret != 0)printf("Error in mutex initialization :");
   }

   for (i = 0; i < 2; i++) { //Initializing condition variables.
      pthread_mutex_init(&mutex_cond_var_task[i],NULL);
   }

   for (i = 0; i < 2; i++) { //Initializing condition variables.
      pthread_cond_init(&cond_variable_for_event_of_aperiodic[i], NULL);
   }
/*
      pthread_t mouse_event_thread;
      pthread_attr_t thr_atr;
      struct sched_param sch_para;

      sch_para.sched_priority = 99; //set highest priority of keyboard reading task.
      pthread_attr_init(&thr_atr);
      pthread_attr_setschedpolicy(&thr_atr, SCHED_FIFO);
      pthread_attr_setschedparam(&thr_atr, &sch_para);
      pthread_create(&mouse_event_thread, &thr_atr, (void *)mouse_eve, NULL);

*/
   fr = fopen ("/home/esp/Desktop/input.txt", "rt");
   printf("OPening\n");

   if (fgets(line,80, fr) != NULL)
   {
   		token = strtok(line, " ");
   		no_of_thr = atoi(token);
 

   		token = strtok(NULL, " ");
   		exec_time = atoi(token);

   		printf ("Number of tasks: %d Exec Time: %d \n", no_of_thr, exec_time);

   		printf("Size of struct array :");
         task_array = (TASK*)calloc(no_of_thr, sizeof(TASK));
         printf("size of array is %ld\n", sizeof(task_array));
   }


   while(fgets(line, 80, fr) != NULL)
   {

         int lock_index =0;
         token = strtok(line, " ");
    /* walk through other tokens */
   		while( token != NULL ) 
   		{	
   			line_count++;
   			if(line_count == 1){
   				task_array[index].type = *((char*)token);
   				printf("Task type %c\n", task_array[index].type);
   				token = strtok(NULL, " ");
   			}
   			else if(task_array[index].type == 'P'){
   				if(line_count == 2){
			   		task_array[index].priority = atoi(token);
			   		printf( "Task priority is :%d\n", task_array[index].priority);    
		      		token = strtok(NULL, " ");
			   		
                  task_array[index].period = atoi(token);
			   		printf( "Task period is :%d\n", task_array[index].period);    
		      		token = strtok(NULL, " ");

			   		task_array[index].iteration = atoi(token);
			   		printf( "Busy loop iteration is :%d\n", task_array[index].iteration);    
		      		token = strtok(NULL, " ");
                  if( token == NULL){
                     printf("\n");
                     break;
                  }


				  }
            if(token[0] == 'L'){
               char n = *((char*)token+1);
               task_array[index].lock[lock_index]  = atoi(&n);
               printf("Value of lock is: %d\n",task_array[index].lock[lock_index]);
               token = strtok(NULL, " ");
               task_array[index].no_it_lock[lock_index] = atoi(token);
               token = strtok(NULL, " ");
               printf("Value of lock iteration is: %d\n",task_array[index].no_it_lock[lock_index]);
               lock_index++; 
            }
            lock_index=0;
            if(token[0] = 'U'){
               char n = *((char*)token+1);
               task_array[index].unlock[lock_index]  = atoi(&n);
               printf("Value of unlock is: %d\n",task_array[index].lock[lock_index]);
               token = strtok(NULL, " ");
               task_array[index].no_it_unlock[lock_index] = atoi(token);
               token = strtok(NULL, " ");
               printf("Value of unlock iteration is: %d\n",task_array[index].no_it_unlock[lock_index]);
               lock_index++;
               printf("\n");
            }   
   			}

   			else if(task_array[index].type == 'A'){
   				if(line_count == 2){
			   		task_array[index].priority = atoi(token);
			   		printf( "Task priority is :%d\n", task_array[index].priority);    
		      		token = strtok(NULL, " ");

			   		task_array[index].event_number = atoi(token);
			   		printf( "Task event number is :%d\n", task_array[index].event_number);    
		      		token = strtok(NULL, " ");

			   		task_array[index].iteration = atoi(token);
			   		printf( "Busy loop iteration is :%d\n", task_array[index].iteration); 

                  token = strtok(NULL, " ");
                  if( token == NULL){
                     printf("\n");
                     break;
                  }


				}

            if(token[0] == 'L'){
               char n = *((char*)token+1);
               task_array[index].lock[lock_index]  = atoi(&n);
               printf("Value of lock is: %d\n",task_array[index].lock[lock_index]);
               token = strtok(NULL, " ");
               task_array[index].no_it_lock[lock_index] = atoi(token);
               token = strtok(NULL, " ");
               printf("Value of lock is: %d\n",task_array[index].no_it_lock[lock_index]);
               lock_index++; 
               }
            if(token[0] = 'U'){
               char n = *((char*)token+1);
               task_array[index].unlock[lock_index]  = atoi(&n);
               printf("Value of unlock is: %d\n",task_array[index].lock[lock_index]);
               token = strtok(NULL, " ");
               task_array[index].no_it_unlock[lock_index] = atoi(token);
               token = strtok(NULL, " ");
               printf("Value of unlock teration is: %d\n",task_array[index].no_it_unlock[lock_index]);
               lock_index++;
               printf("\n");
            }

            }

      	}
         index++;
         printf("Value of index is %d\n", index);
      	line_count = 0;

   }



   for(i=0; i<no_of_thr; ++i){
      
         ret = pthread_attr_init(&attr[i]);
         param[i].sched_priority = task_array[i].priority;
         ret = pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);
         ret = pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);
         ret = pthread_attr_setschedparam(&attr[i], &param[i]);
         printf("RETURN VALUE IS :%d\n", ret);

   }

   for(i=0; i< no_of_thr; i++){
         
         if(task_array[i].type == 'P'){
            printf("In the if of %d\n", i);
//            printf("%p\n",task_array );
            pthread_create(&thread[i], &attr[i], periodic_task, (void *)(task_array + i));
            usleep(1);
         }
         if(task_array[i].type == 'A'){
            printf("In the if of %d\n", i);
            pthread_create(&thread[i], &attr[i], aperiodic_task, (void *)(task_array + i));
            usleep(1);
         }

   }

pthread_cond_broadcast(&cond_variable_to_act_all);


   for(i=0; i<no_of_thr; i++){
         pthread_join(thread[i], NULL);
         printf("Thread joined: %d \n", i);
   }


   free((void*)task_array);
   fclose(fr); 





   return 0;
}