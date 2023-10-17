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
        usleep(rand() % max_conversa); // conversaComAmigos
        sem_wait(&sem_pedido); // fazPedido
        // esperaPedido
        sem_wait(&sem_entrega); // recebePedido
        usleep(rand() % max_consumo); // consomePedido
    }
    return NULL;
}



void* garcom(void* arg) {
    while (rodada < R) {
        for(int i = 0; i < Gn; i++) {
            sem_post(&sem_pedido); // recebeMaximoPedidos
        }
        // registraPedidos
        for(int i = 0; i < Gn; i++) {
            sem_post(&sem_entrega); // entregaPedidos
        }
        sem_wait(&mutex);
        rodada++;
        sem_post(&mutex);
    }
    return NULL;
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


}