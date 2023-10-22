#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>



// N -> número de clientes
// G -> número de garçons
// Gn -> quantidade de clientes que cada Garçom  atende
// R -> número de rodadas

int N, G, Gn, R, max_conversa, max_consumo, rodada = 0, pedidos_pendentes = 0;
sem_t sem_pedidos, sem_entregas, mutex_pedidos, mutex_entregas;


void* cliente(void* arg) {
    while (rodada < R) {

        // Conversando
        sleep(rand() % max_conversa);

        // Fazendo o pedido
        sem_wait(&mutex_pedidos);

        if (rodada < R) {
            pedidos_pendentes++;
            printf("Cliente %ld faz pedido\n", pthread_self());
            sem_post(&sem_pedidos);
        }
        
        sem_post(&mutex_pedidos);

        // Esperando o pedido ser entregue
        sem_wait(&sem_entregas);

        // Recebendo o pedido
        sem_wait(&mutex_pedidos);

        printf("Cliente %ld recebe pedido\n", pthread_self());
        if (!(--pedidos_pendentes)) printf("\nFim da rodada %d\n\n", ++rodada);

        sem_post(&mutex_pedidos);

        // Comendo
        sleep(rand() % max_consumo);
    
    }
    return 0;
}

void* garcom(void* arg) {
    while (rodada < R) {
        int rodada_local = rodada;

        // Esperando os pedidos
        for (int i = 0; i < Gn; i++) {

            sem_wait(&sem_pedidos);
            printf("Garçom  %ld anota %dº pedido\n", pthread_self(), i + 1);
        }

        // Entregando o pedido
        sem_wait(&mutex_entregas);

        printf("Garçom  %ld entrega pedidos\n", pthread_self());
        for (int i = 0; i < Gn; i++) sem_post(&sem_entregas);
        
        sem_post(&mutex_entregas);

    }
    return 0;

}



int main(int argc, char* argv[]){
    if (argc != 7) {
        printf("Uso: %s <clientes> <garcons> <clientes/garcon> <rodadas> <max.conversa> <max.consumo>\n", argv[0]);
        exit(1);
    }

    // Inicializando as variáveis com os argumentos
    N = atoi(argv[1]);
    G = atoi(argv[2]);
    Gn = atoi(argv[3]);
    R = atoi(argv[4]);
    max_conversa = atoi(argv[5]);
    max_consumo = atoi(argv[6]);

    // Inicializando os semáforos
    sem_init(&sem_pedidos, 0, 0);
    sem_init(&sem_entregas, 0, 0);
    sem_init(&mutex_pedidos, 0, 1);
    sem_init(&mutex_entregas, 0, 1);

    // Inicializando as threads
    pthread_t clientes[N];
    pthread_t garcons[G];

    // Criando as threads
    for (int i = 0; i < N; i++) {
        pthread_create(&clientes[i], NULL, cliente, NULL);
    }

    for (int i = 0; i < G; i++) {
        pthread_create(&garcons[i], NULL, garcom, NULL);
    }

    // Esperando as threads terminarem
    for (int i = 0; i < N; i++) {
        pthread_join(clientes[i], NULL);
    }

    for (int i = 0; i < G; i++) {
        pthread_join(garcons[i], NULL);
    }

    // Destruindo os semáforos
    sem_destroy(&sem_pedidos);
    sem_destroy(&sem_entregas);
    sem_destroy(&mutex_pedidos);
    sem_destroy(&mutex_entregas);

    return 0;
}