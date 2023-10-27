#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>


// N -> número de clientes
// G -> número de garçons
// Gn -> quantidade de clientes que cada Garçom  atende
// R -> número de rodadas


int N, G, Gn, R, max_conversa, max_consumo, rodada = 0, index_pedidos = 0, index_entregas = 0;
sem_t sem_pedidos, sem_entregas, mutex_pedidos, mutex_entregas, garcons_prontos;

typedef struct {
    int id_cliente;
    int* lista_id;
    sem_t** lista_mutex;
} info_clientes;

typedef struct {
    int id_garcom;
    int* lista_id;
    sem_t** lista_mutex;
} info_garcons;


void* cliente(void* info_cliente) {
    
    info_clientes* cliente = (info_clientes*) info_cliente;

    sem_t mutex_cliente;
    sem_init(&mutex_cliente, 0, 0);
    
    while (rodada < R) {

        // Conversando
        sleep(rand() % max_conversa);

        // Fazendo pedido
        sem_wait(&mutex_pedidos);

        printf("CLIENTE %02d PEDIU\n", cliente->id_cliente);
        cliente->lista_id[(index_pedidos)%N] = cliente->id_cliente;
        cliente->lista_mutex[(index_pedidos++)%N] = &mutex_cliente;
        sem_post(&sem_pedidos);

        sem_post(&mutex_pedidos);

        // Esperando pedido
        sem_wait(&mutex_cliente);

        // Recebendo pedido
        printf("CLIENTE %02d RECEBEU\n", cliente->id_cliente);

        // Comendo
        sleep(rand() % max_consumo);
    
    }

    sem_destroy(&mutex_cliente);
    return 0;
}



void* garcom(void* info_garcom) {

    info_garcons* garcom = (info_garcons*) info_garcom;

    int ids_anotados[Gn];
    sem_t* mutex_anotados[Gn];

    while (rodada < R) {

        // Enchendo a bandeja
        for (int i = 0; i < Gn; i++) {

            // Esperando pedido
            sem_wait(&sem_pedidos);

            // Anotando pedido
            sem_wait(&mutex_entregas);

            printf("GARÇOM  %02d ANOTOU   O PEDIDO DO CLIENTE %02d (%d/%d)\n", garcom->id_garcom, garcom->lista_id[(index_entregas)%N], i + 1, Gn);
            ids_anotados[i] = garcom->lista_id[(index_entregas)%N];
            mutex_anotados[i] = garcom->lista_mutex[(index_entregas++)%N];

            sem_post(&mutex_entregas);
            
        }
        
        // Entregando os pedidos
        for (int i = 0; i < Gn; i++) {
            printf("GARÇOM  %02d ENTREGOU O PEDIDO DO CLIENTE %02d\n", garcom->id_garcom, ids_anotados[i]);
            sem_post(mutex_anotados[i]);
        }

        // Esperando todos os garcons
        sem_wait(&mutex_entregas);
        if (index_entregas % (G * Gn)) {

            sem_post(&mutex_entregas);
            sem_wait(&garcons_prontos);

        } else {

            for(int i = 0; i < G - 1; i++) sem_post(&garcons_prontos);
            printf("\nFim da rodada %d\n\n", ++rodada);
            sem_post(&mutex_entregas);

        }
        
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

    // Inicializando vetores
    int* lista_id = (int*) malloc(N * sizeof(int));
    sem_t** lista_mutex = (sem_t**) malloc(N * sizeof(sem_t*));
    info_clientes info_cliente[N];
    info_garcons info_garcom[G];

    // Inicializando os semáforos
    sem_init(&sem_pedidos, 0, 0);
    sem_init(&sem_entregas, 0, 0);
    sem_init(&mutex_pedidos, 0, 1);
    sem_init(&mutex_entregas, 0, 1);
    sem_init(&garcons_prontos, 0, 0);

    // Inicializando as threads
    pthread_t clientes[N];
    pthread_t garcons[G];

    // Abrindo o bar
    printf("\nBAR ABERTO: %d RODADAS GRÁTIS!!!\n\n", R);

    // Criando as threads dos clientes
    for (int i = 0; i < N; i++){

        info_cliente[i].id_cliente = i + 1;
        info_cliente[i].lista_id = lista_id;
        info_cliente[i].lista_mutex = lista_mutex;

        pthread_create(&clientes[i], NULL, cliente, (void*)&info_cliente[i]);
    }

    // Criando as threads dos garçons
    for (int i = 0; i < G; i++){

        info_garcom[i].id_garcom = i + 1;
        info_garcom[i].lista_id = lista_id;
        info_garcom[i].lista_mutex = lista_mutex;

        pthread_create(&garcons[i], NULL, garcom, (void*)&info_garcom[i]);
    }

    // Esperando as threads clientes terminarem
    for (int i = 0; i < N; i++){
        pthread_join(clientes[i], NULL);
    }

    // Esperando as threads garçons terminarem
    for (int i = 0; i < G; i++){
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