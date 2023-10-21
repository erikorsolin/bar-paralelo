#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>



// N -> número de clientes
// G -> número de garçons
// Gn -> quantidade de clientes que cada garçom atende
// R -> número de rodadas

int N, G, Gn, R, max_conversa, max_consumo, rodada = 0;
sem_t sem_pedido, sem_entrega, mutex;


void* cliente(void* arg) {
    while (rodada < R) {
        // Conversando
        sleep(rand() % max_conversa);

        // Fazendo o pedido
        sem_wait(&mutex);
        
        if (rodada < R) {
        sem_post(&sem_pedido);
        printf("Cliente %ld faz pedido\n", pthread_self());
        }
        
        sem_post(&mutex);

        // Esperando o pedido ser entregue
        sem_wait(&sem_entrega);

        sem_wait(&mutex);
        printf("Cliente %ld recebe pedido\n", pthread_self());
        sem_post(&mutex);

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
            sem_wait(&sem_pedido);

            sem_wait(&mutex);
            printf("Garçom %ld anota pedido\n", pthread_self());
            sem_post(&mutex);
        }

        // Entregando o pedido
        sem_wait(&mutex);
        for (int i = 0; i < Gn; i++) {
            sem_post(&sem_entrega);
            printf("Garçom %ld entrega pedido\n", pthread_self());
        }
        sem_post(&mutex);
        
        // Verificando se a rodada acabou
        int value_pedidos, value_entregas;

        sem_getvalue(&sem_pedido, &value_pedidos);
        sem_getvalue(&sem_entrega, &value_entregas);

        sem_wait(&mutex);
        if (!value_pedidos && !value_entregas && rodada_local == rodada) {
            printf("\nGarçom %ld terminou a rodada %d\n\n", pthread_self(), (rodada + 1));
            rodada++;
        }
        sem_post(&mutex);

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
    sem_init(&sem_pedido, 0, 0);
    sem_init(&sem_entrega, 0, 0);
    sem_init(&mutex, 0, 1);

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
    sem_destroy(&sem_pedido);
    sem_destroy(&sem_entrega);
    sem_destroy(&mutex);

    return 0;
}