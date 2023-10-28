#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>


// N -> número de clientes
// G -> número de garçons
// Gn -> quantidade de clientes que cada Garçom  atende
// R -> número de rodadas


int N, G, Gn, R, max_conversa, max_consumo, rodada = 0, index_clientes = 0, index_garcons = 0, pedidos_entregues = 0;
sem_t sem_pedidos, mutex_clientes, mutex_garcons, garcons_prontos, sem_controle;

typedef struct {
    int id_cliente;
    int* lista_id_clientes;
    sem_t** lista_sem_clientes;
} info_clientes;

typedef struct {
    int id_garcom;
    int* lista_id_clientes;
    sem_t** lista_sem_clientes;
} info_garcons;


void* cliente(void* info_cliente) {
    
    info_clientes* cliente = (info_clientes*) info_cliente;

    sem_t sem_cliente;
    sem_init(&sem_cliente, 0, 0);
    
    while (rodada < R) {

        // Conversando
        sleep(rand() % max_conversa);

        // Fazendo pedido
        sem_wait(&sem_controle);
        if (rodada == R) break;

        sem_wait(&mutex_clientes);

        printf("CLIENTE %02d SOLICITA ATENDIMENTO PARA PEDIR\n", cliente->id_cliente);
        cliente->lista_id_clientes[(index_clientes)%N] = cliente->id_cliente;
        cliente->lista_sem_clientes[(index_clientes++)%N] = &sem_cliente;
        sem_post(&sem_pedidos);

        sem_post(&mutex_clientes);

        // Esperando pedido
        sem_wait(&sem_cliente);

        // Comendo
        sleep(rand() % max_consumo);

    }

    printf("CLIENTE %02d SAI DO BAR\n", cliente->id_cliente);
    pthread_exit(NULL);
}


void* garcom(void* info_garcom) {

    info_garcons* garcom = (info_garcons*) info_garcom;

    int ids_anotados[Gn];
    sem_t* sem_anotados[Gn];

    while (rodada < R) {

        // Enchendo a bandeja
        for (int i = 0; i < Gn; i++) {

            // Esperando pedido
            sem_wait(&sem_pedidos);

            // Anotando pedido
            sem_wait(&mutex_garcons);

            printf("GARÇOM  %02d ANOTOU   O PEDIDO DO CLIENTE %02d (%d/%d)\n", garcom->id_garcom, garcom->lista_id_clientes[(index_garcons)%N], i + 1, Gn);
            ids_anotados[i] = garcom->lista_id_clientes[(index_garcons)%N];
            sem_anotados[i] = garcom->lista_sem_clientes[(index_garcons++)%N];

            sem_post(&mutex_garcons);
            
        }
        
        // Entregando os pedidos
        for (int i = 0; i < Gn; i++) {
            printf("GARÇOM  %02d ENTREGOU O PEDIDO DO CLIENTE %02d\n", garcom->id_garcom, ids_anotados[i]);
            sem_post(sem_anotados[i]);
        }

        // Esperando todos os garcons
        sem_wait(&mutex_clientes);

        pedidos_entregues += Gn;

        if (!(pedidos_entregues == G * Gn)) {

            sem_post(&mutex_clientes);
            sem_wait(&garcons_prontos);

        } else {

            printf("\nFim da rodada %d\n\n", ++rodada);
            pedidos_entregues = 0;
            for(int i = 0; i < G - 1; i++) sem_post(&garcons_prontos);
            for(int i = 0; i < G * Gn; i++) sem_post(&sem_controle);
            sem_post(&mutex_clientes);
        }
    }

    printf("GARÇOM  %02d SAI DO BAR\n", garcom->id_garcom);
    pthread_exit(NULL);
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
    int* lista_id_clientes = (int*) malloc(N * sizeof(int));
    sem_t** lista_sem_clientes = (sem_t**) malloc(N * sizeof(sem_t*));
    info_clientes info_cliente[N];
    info_garcons info_garcom[G];

    // Inicializando os semáforos
    sem_init(&sem_pedidos, 0, 0);
    sem_init(&mutex_clientes, 0, 1);
    sem_init(&mutex_garcons, 0, 1);
    sem_init(&garcons_prontos, 0, 0);
    sem_init(&sem_controle, 0, (G*Gn));

    // Inicializando as threads
    pthread_t clientes[N];
    pthread_t garcons[G];

    // Abrindo o bar
    printf("\nBAR ABERTO: %d RODADAS GRÁTIS!!!\n\n", R);

    // Criando as threads dos garçons
    for (int i = 0; i < G; i++){

        info_garcom[i].id_garcom = i + 1;
        info_garcom[i].lista_id_clientes = lista_id_clientes;
        info_garcom[i].lista_sem_clientes = lista_sem_clientes;

        pthread_create(&garcons[i], NULL, garcom, (void*)&info_garcom[i]);
    }

    // Criando as threads dos clientes
    for (int i = 0; i < N; i++){

        info_cliente[i].id_cliente = i + 1;
        info_cliente[i].lista_id_clientes = lista_id_clientes;
        info_cliente[i].lista_sem_clientes = lista_sem_clientes;

        pthread_create(&clientes[i], NULL, cliente, (void*)&info_cliente[i]);
    }

    // Esperando as threads garçons terminarem
    for (int i = 0; i < G; i++) pthread_join(garcons[i], NULL);

    // Esperando as threads clientes terminarem
    for (int i = 0; i < N; i++) pthread_join(clientes[i], NULL);

    // Fechando o bar
    printf("\nBAR FECHADO, ATÉ A PRÓXIMA!!!\n\n");

    // Destruindo os semáforos
    sem_destroy(&sem_pedidos);
    sem_destroy(&mutex_clientes);
    sem_destroy(&mutex_garcons);
    sem_destroy(&garcons_prontos);

    return 0;
}