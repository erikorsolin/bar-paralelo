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


void* cliente(void* id_cliente) {
    while (rodada < R) {

        // Conversando
        sleep(rand() % max_conversa);

        // Fazendo o pedido
        sem_wait(&mutex_pedidos);

        if (rodada < R) {

            pedidos_pendentes++;
            printf("Cliente %d faz pedido                    ++pedidos pendentes = %d\n", *(int*)id_cliente, pedidos_pendentes);
            sem_post(&sem_pedidos);
            
        } else exit(1);

        sem_post(&mutex_pedidos);

        // Esperando o pedido ser entregue
        sem_wait(&sem_entregas);

        // Recebendo o pedido
        sem_wait(&mutex_pedidos);

        printf("Cliente %d recebe pedido                 --pedidos pendentes = %d\n", *(int*)id_cliente, --pedidos_pendentes);
        if (!(pedidos_pendentes)) printf("\nFim da rodada %d\n\n", ++rodada);

        sem_post(&mutex_pedidos);

        // Comendo
        sleep(rand() % max_consumo);
    
    }
    return 0;
}

void* garcom(void* id_garcom) {
    while (rodada < R) {

        // Esperando os pedidos
        for (int i = 0; i < Gn; i++) {
            sem_wait(&sem_pedidos);
            printf("Garçom  %d anota %dº pedido\n", *(int*)id_garcom, i + 1);
        }
        
        // Entregando o pedido
        sem_wait(&mutex_entregas);

        printf("Garçom  %d entrega pedidos\n", *(int*)id_garcom);
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
    int id_clientes[N], id_garcons[G];
    pthread_t clientes[N];
    pthread_t garcons[G];

    // Abrindo o bar
    printf("\nBAR ABERTO: %d RODADAS GRÁTIS!!!\n\n", R);

    // Criando as threads
    for (int i = 0; i < N; i++) {
        id_clientes[i] = i + 1;
        pthread_create(&clientes[i], NULL, cliente, &id_clientes[i]);
    }

    for (int i = 0; i < G; i++) {
        id_garcons[i] = i + 1;
        pthread_create(&garcons[i], NULL, garcom, &id_garcons[i]);
    }

    // Esperando as threads terminarem
    for (int i = 0; i < N; i++) {
        pthread_join(clientes[i], NULL);
    }

    for (int i = 0; i < G; i++) {
        pthread_join(garcons[i], NULL);
    }

    // Fechando o bar
    printf("\nBAR FECHADO, ATÉ A PRÓXIMA!!!\n");

    // Destruindo os semáforos
    sem_destroy(&sem_pedidos);
    sem_destroy(&sem_entregas);
    sem_destroy(&mutex_pedidos);
    sem_destroy(&mutex_entregas);

    return 0;
}