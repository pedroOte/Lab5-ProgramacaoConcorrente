/*Pedro Luis Mello Otero
121074528
*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

#define MAX_CARACTERES 1000 //Número máximo de caracteres do buffer
#define MAX_LINHAS 100

sem_t slotCheio, slotVazio;
sem_t mutexCons, mutexProd;

char buffer[MAX_LINHAS][MAX_CARACTERES]; 

void retira(){
    static int out=0;
    char * item;
    //aguarda slot cheio
    sem_wait(&slotCheio);
    //exclusao mutua entre consumidores
    sem_wait(&mutexCons);
    item = buffer[out];        

    printf("%s", item);

    out = (out + 1) % MAX_LINHAS;
    sem_post(&mutexCons);
    //sinaliza um slot vazio
    sem_post(&slotVazio);
}

void *consumidor(){
    while (1){
        retira();
    }
}

int main(int argc, char *argv[]){ //Produtora
    int nthreads;

    FILE *text; //cria o arquivo de teste

    //recebe o numero de threads
    if(argc < 3){
        printf("Insira o numero de threads e o arquivo txt na linha de comando\n");
        return 1;
    }
    nthreads = atoi(argv[1]); 

    //atribui o arquivo txt a variável text
    text = fopen(argv[2], "r"); 
    if (text == NULL){
        printf("Arquivo txt nao acessado\n");
        return 2;
    }

    //inicializa os semáforos
    sem_init(&slotCheio, 0, 0);
    sem_init(&slotVazio, 0, MAX_LINHAS);
    sem_init(&mutexCons, 0, 1);
    sem_init(&mutexProd, 0, 1);
    
    //indentificadores de threads
    pthread_t tid[nthreads];

    // cria as threads
    for(int i = 0; i < nthreads; i++){
        pthread_create(&tid[i], NULL, consumidor, NULL);
    }


    //Passa o arquivo txt para o buffer
    int in = 0;
    while(!feof(text) && !ferror(text)){
        sem_wait(&slotVazio);
        sem_wait(&mutexProd);


        if(fgets(buffer[in], MAX_CARACTERES, text) != NULL){
            in = (in + 1) % MAX_LINHAS;
        }

        sem_post(&mutexProd);
        sem_post(&slotCheio);
    }

    fclose(text); //fecha o arquivo

    //Junta as threads
    for(int i = 0; i < nthreads; i++){
        if(pthread_join(tid[i], NULL)){
            printf("Erro no Join\n");
            return 3;
        }
    }

    //libera os semaforos
    sem_destroy(&slotCheio);
    sem_destroy(&slotVazio);
    sem_destroy(&mutexCons);
    sem_destroy(&mutexProd);
}     
