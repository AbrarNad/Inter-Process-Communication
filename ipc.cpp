#include<iostream>
#include<cstdio>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>
#include<cstring>
#include <time.h>
#include<stdlib.h>
#include<time.h>


using namespace std;

#define number_of_cycles 10
#define number_of_serviceman 3
#define number_of_payment 2

int ind = 0, departCount = 0, serviceCount = 0;
sem_t path, departPath;
sem_t servicing_arr[number_of_serviceman];
sem_t payment_arr;
pthread_mutex_t smutex;



void* get_service(void* arg){
 
    srand(time(0)); 
//servicing
    for (int i = 0; i < number_of_serviceman; i++)
    {
        sem_wait(&servicing_arr[i]);
        
        if (i==0)
        {
            sem_wait(&path);

            pthread_mutex_lock(&smutex);
            serviceCount++;
            
            if (serviceCount == 1)
            {
                sem_wait(&departPath);
            }


            sem_post(&path);
            pthread_mutex_unlock(&smutex);
        }
        
        printf("%s started taking service from serviceman %d\n",(char*)arg,i+1);
        if (i!=0)
        {
            sem_post(&servicing_arr[i-1]);
        }
        sleep(rand()%3);
        printf("%s finished taking service from serviceman %d\n",(char*)arg,i+1);
    }

    pthread_mutex_lock(&smutex);
    serviceCount--;
    
    if (serviceCount == 0)
    {  
        sem_post(&departPath);
    }
    
    sem_post(&servicing_arr[number_of_serviceman-1]);
    pthread_mutex_unlock(&smutex);


//payment 
    sem_wait(&payment_arr);
    printf("%s started paying the service bill\n",(char*)arg);
    sleep(rand()%3);
    printf("%s finished paying the service bill\n",(char*)arg);
    sem_post(&payment_arr);

//departure
    pthread_mutex_lock(&smutex);
    departCount++;
    pthread_mutex_unlock(&smutex);

    if (departCount == 1)
    {
        sem_wait(&path);
    }
    sem_wait(&departPath);
    

    sleep(rand()%2);
    pthread_mutex_lock(&smutex);
    printf("%s has departed\n",(char*)arg);

    
    departCount--;
    
    sem_post(&departPath);
    if (departCount == 0)
    {
        sem_post(&path);
    }
    pthread_mutex_unlock(&smutex);
}

void initialize_sem(){
    int res;

    for (int i = 0; i < number_of_serviceman; i++)
    {
        res = sem_init(&servicing_arr[i],0,1);
        if (res!=0)
        {
            printf("Servicing semaphore init Failed/n");
        }
        
    }

    res = sem_init(&payment_arr,0,number_of_payment);
    if(res != 0){
        printf("Failed\n");
    }

    
    res = sem_init(&path,0,1);
    if(res != 0){
        printf("Failed\n");
    }

    res = sem_init(&departPath,0,1);
    if(res != 0){
        printf("Failed\n");
    }

    res = pthread_mutex_init(&smutex,NULL);
    if(res != 0){
        printf("Mutex init Failed\n");
    }
}

void destroy_sem(){
    int res;
    res = sem_destroy(&path);
    if(res != 0){
        printf("Failed\n");
    }
    res = sem_destroy(&departPath);
    if(res != 0){
        printf("Failed\n");
    }

    res = pthread_mutex_destroy(&smutex);
    if(res != 0){
        printf("Failed\n");
    }


    for (int i = 0; i < number_of_cycles; i++)
    {
        res = sem_destroy(&servicing_arr[i]);
        if (res!=0)
        {
            printf("Failed\n");
        }
        
    }
    res = sem_destroy(&payment_arr);
    if(res != 0){
        printf("Failed\n");
    }
}

int main(int argc, char* argv[])
{
    
    int res;
    initialize_sem();

    pthread_t cycles[number_of_cycles];

    for (int i = 0; i < number_of_cycles; i++)
    {
        char *id = new char[3];
        strcpy(id,to_string(i+1).c_str());

        res = pthread_create(&cycles[i],NULL,get_service,(void *)id);
        if (res!=0)
        {
            printf("Thread creation failed\n");
        }
    }
    
    

    for(int i = 0; i < number_of_cycles; i++){
        void *result;
        pthread_join(cycles[i],&result);
        //printf("%s",(char*)result);
    }
    
    destroy_sem();

    return 0;
}
