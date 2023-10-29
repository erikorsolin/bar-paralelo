#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

// N -> número de clientes
// G -> número de garçons
// Gn -> quantidade de clientes que cada Garçom  atende
// R -> número de rodadas
// max_conversa -> tempo máximo de conversa
// max_consumo -> tempo máximo de consumo

int N, G, Gn, R, Gl, rodada = 0, index_clientes = 0, index_garcons = 0, pedidos_entregues = 0; 
int max_conversa_micro, max_conversa_mili, max_conversa_seg, max_consumo_micro, max_consumo_mili, max_consumo_seg;
sem_t sem_pedidos, sem_controle, mutex_clientes, mutex_garcons, mutex_rodada, garcons_prontos, garcons_liberados;

// definindo as structs para passar os parâmetros para as threads
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

// função das threads cliente
void* cliente(void* info_cliente) {
    
    info_clientes* cliente = (info_clientes*) info_cliente; // cast para o tipo info_clientes

    sem_t sem_cliente;  // semáforo usado para o cliente receber seu pedido
    sem_init(&sem_cliente, 0, 0); 
    
    // loop de comportamento do cliente
    while (rodada < R) {

        // conversando
        if (max_conversa_seg) sleep(rand() % max_conversa_seg);
        if (max_conversa_micro) usleep((rand() % 1000) * (max_conversa_micro / 1000));

        // fazendo pedido
        sem_wait(&sem_controle); // semáforo que garante que G*Gn clientes façam seus pedidos
        if (rodada == R) break; 

        sem_wait(&mutex_clientes); // mutex para proteger o uso da variável index_clientes

        printf("CLIENTE %02d PEDIU\n", cliente->id_cliente);
        cliente->lista_id_clientes[(index_clientes)%N] = cliente->id_cliente; // adicionando o id do cliente na lista de ids
        cliente->lista_sem_clientes[(index_clientes++)%N] = &sem_cliente;     // adicionando o semáforo do cliente na lista de semáforos
        sem_post(&sem_pedidos); // sinalizando para os garçons que um pedido foi feito

        sem_post(&mutex_clientes);

        // esperando pedido
        sem_wait(&sem_cliente);

        // comendo
        if (max_consumo_seg) sleep(rand() % max_consumo_seg);
        if (max_consumo_micro) usleep((rand() % 1000) * (max_consumo_micro / 1000));
    }

    pthread_exit(NULL);
}

// função das threads garçom
void* garcom(void* info_garcom) {

    info_garcons* garcom = (info_garcons*) info_garcom; // cast para o tipo info_garcons

    int ids_anotados[Gn]; // vetor para a fila interna de ids de cada garçom
    sem_t* sem_anotados[Gn]; // vetor para a fila interna de semáforos de cada garçom

    // loop de comportamento do garçom
    while (rodada < R) {

        sem_wait(&garcons_liberados); // semáforo para impedir o deadlock em que N < (Gn - 1) * G

        // enchendo a bandeja
        for (int i = 0; i < Gn; i++) {

            // esperando ser feito um pedido
            sem_wait(&sem_pedidos);

            // anotando pedido
            sem_wait(&mutex_garcons); // mutex para proteger o uso da variável index_garcons

            printf("GARÇOM  %02d ANOTOU   O PEDIDO DO CLIENTE %02d (%d/%d)\n", garcom->id_garcom, garcom->lista_id_clientes[(index_garcons)%N], i + 1, Gn);
            ids_anotados[i] = garcom->lista_id_clientes[(index_garcons)%N]; // adicionando o id do cliente na fila interna
            sem_anotados[i] = garcom->lista_sem_clientes[(index_garcons++)%N]; // adicionando o semáforo do cliente na fila interna 

            sem_post(&mutex_garcons);
            
        }
        
        // entregando os pedidos
        for (int i = 0; i < Gn; i++) {
            printf("GARÇOM  %02d ENTREGOU O PEDIDO DO CLIENTE %02d\n", garcom->id_garcom, ids_anotados[i]);
            sem_post(sem_anotados[i]); // sinalizando para o cliente que seu pedido foi entregue
        }

        // comportamento do garçom que já atendeu seus clientes
        sem_wait(&mutex_rodada); // mutex para proteger o uso da variável rodada

        sem_post(&garcons_liberados); // sinalizando que mais um garçom pode atender sem causar o deadlock em que N < (Gn - 1) * G
        pedidos_entregues += Gn; // incrementando o número de pedidos entregues

        
        // caso nem todos os garçons tenham terminado de atender
        if (!(pedidos_entregues == G * Gn)) {

            sem_post(&mutex_rodada); // liberando o mutex para que mais garçons verifiquem o fim de rodada
            sem_wait(&garcons_prontos); // esperando o ultimo garçom incrementar a rodada

        } else {

            printf("\nFim da rodada %d\n\n", ++rodada); // incrementando a rodada e imprimindo o fim da rodada

            pedidos_entregues = 0; // resetando o número de pedidos entregues
            for(int i = 0; i < G - 1; i++) sem_post(&garcons_prontos); // liberando os garçons para a próxima rodada

            if (rodada < R) for (int i = 0; i < G * Gn; i++) sem_post(&sem_controle); // liberando G * Gn clientes para fazerem seus pedidos
            else for (int i = 0; i < N; i++) sem_post(&sem_controle); // liberando todos os clientes para saírem do bar

            sem_post(&mutex_rodada);
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]){

    // tratamento de exceções
    if (argc != 7 || atoi(argv[1]) <= 0 || atoi(argv[2]) <= 0 || atoi(argv[3]) <= 0 || atoi(argv[4]) <= 0 || atoi(argv[5]) <= 0 || atoi(argv[6]) <= 0) {
        printf("Uso: %s <clientes> <garcons> <clientes/garcon> <rodadas> <max.conversa> <max.consumo>\n", argv[0]);
        exit(1);
    } else if (atoi(argv[1]) < atoi(argv[3])) {
        printf("Erro: O número de clientes deve ser maior que a capacidade de cada garçom. Caso contrário ocorrerá um deadlock inevitável, quando seguidas as regras do enunciado.\n");
        exit(1);
    }

    // inicializando as variáveis com os argumentos
    N = atoi(argv[1]);
    G = atoi(argv[2]);
    Gn = atoi(argv[3]);
    R = atoi(argv[4]);
    max_conversa_mili = atoi(argv[5]); 
    max_consumo_mili = atoi(argv[6]);

    // converte de milésimos para segundos e microsegundos
    max_conversa_seg = max_conversa_mili / 1000; 
    max_conversa_micro = (max_conversa_mili % 1000) * 1000;
    max_consumo_seg = max_consumo_mili / 1000;
    max_consumo_micro = (max_consumo_mili % 1000) * 1000;

    // tratamento do caso em que N < (Gn - 1) * G
    Gl = G;
    while (N <= (Gn - 1) * Gl) Gl--;

    // declarando vetores
    int* lista_id_clientes = (int*) malloc(N * sizeof(int)); // alocando dinâmicamente a lista de ids dos clientes que pedirão 
    sem_t** lista_sem_clientes = (sem_t**) malloc(N * sizeof(sem_t*)); // alocando dinâmicamente a lista de semáforos dos clientes que pedirão
    info_clientes info_cliente[N]; // vetor de structs para passar os parâmetros para as threads clientes
    info_garcons info_garcom[G]; // vetor de structs para passar os parâmetros para as threads garçons

    // inicializando os semáforos
    sem_init(&sem_controle, 0, (G*Gn));
    sem_init(&garcons_liberados, 0, Gl);
    sem_init(&garcons_prontos, 0, 0);
    sem_init(&mutex_clientes, 0, 1);
    sem_init(&mutex_garcons, 0, 1);
    sem_init(&mutex_rodada, 0, 1);
    sem_init(&sem_pedidos, 0, 0);
    
    // declarando as threads
    pthread_t clientes[N];
    pthread_t garcons[G];

    // abrindo o bar
    printf("\nBAR ABERTO: %d RODADAS GRÁTIS!!!\n\n", R);

    // inicializando as threads dos garçons
    for (int i = 0; i < G; i++){

        // passando os parâmetros para as threads garçons
        info_garcom[i].id_garcom = i + 1; 
        info_garcom[i].lista_id_clientes = lista_id_clientes; 
        info_garcom[i].lista_sem_clientes = lista_sem_clientes; 

        pthread_create(&garcons[i], NULL, garcom, (void*)&info_garcom[i]);
    }

    // inicializando as threads dos clientes
    for (int i = 0; i < N; i++){

        // passando os parâmetros para as threads clientes
        info_cliente[i].id_cliente = i + 1;
        info_cliente[i].lista_id_clientes = lista_id_clientes;
        info_cliente[i].lista_sem_clientes = lista_sem_clientes;

        pthread_create(&clientes[i], NULL, cliente, (void*)&info_cliente[i]);
    }

    // esperando as threads garçons terminarem
    for (int i = 0; i < G; i++) pthread_join(garcons[i], NULL);

    // esperando as threads clientes terminarem
    for (int i = 0; i < N; i++) pthread_join(clientes[i], NULL);

    // fechando o bar
    printf("BAR FECHADO: ATÉ A PRÓXIMA!!!\n\n");

    // liberando a memória alocada
    free(lista_id_clientes);
    free(lista_sem_clientes);

    // destruindo os semáforos
    sem_destroy(&sem_pedidos);
    sem_destroy(&sem_controle);
    sem_destroy(&mutex_rodada);
    sem_destroy(&mutex_garcons);
    sem_destroy(&mutex_clientes);
    sem_destroy(&garcons_prontos);
    sem_destroy(&garcons_liberados);

    return 0;
}