# 🍻 Bar Paralelo
Um bar resolveu liberar um número específico de rodadas grátis para seus N clientes presentes no
estabelecimento antes de fechar. Esse bar possui G garçons. Cada garçom consegue atender a um
número limitado Gn de clientes por vez e esta capacidade é igual para todos os garçons. Cada garçom
somente vai para a copa para buscar os pedidos quando todos os Gn clientes que ele pode atender
tiverem feito pedido. Antes de fazer um novo pedido, cada cliente conversa com seus amigos durante
um tempo aleatório. Após ter seu pedido atendido, um cliente pode fazer um novo pedido após
consumir sua bebida (o que também leva um tempo aleatório). Uma nova rodada (R) somente pode
ocorrer quando foram atendidos todos os clientes que conseguiram fazer pedidos na rodada anterior.
Nem todos os clientes precisam pedir uma bebida a cada rodada. A simulação acaba quando o número
de rodadas R é atingido.

## 🚀 Sobre o Projeto

O projeto `Bar Paralelo` foi desenvolvido como parte da disciplina de Programação Concorrente, no curso de Ciências da Computação da Universidade Federal de Santa Catarina. Ele utiliza conceitos de programação concorrente vistos em aula tais como: 
-  Threads
-  Mutexes
-  Semáforos
-  Paralelismo
-  Deadlocks
  

## 🔧 Instalação e Uso
1. Clone o repositório:
   ```
   git clone https://github.com/erikorsolin/bar-paralelo.git
    ```
2. Navegue até o diretório do projeto:
   ```
   cd caminho/para/bar-paralelo
    ```

3. Compile o código com o Makefile fornecido:
   ```
   make
    ```

4. Execute o programa:
   ```
   ./program <clientes> <garcons> <clientes/garcon> <rodadas> <max.conversa> <max.consumo>
   ```

## 📌 Funcionalidades
- **Paralelismo**: O programa implementa o conceito de Threads utilizando a biblioteca POSIX Threads do Linux. Cada garçom ou cliente na simulação é representado por uma Thread específica, isso permite a concorrência do programa a execução de tarefas em paralelo.
  
- **Sincronização**: O programa utiliza estruturas de dados específicas para sincronizar as Threads e evitar Deadlocks, essas estruturas são:
    * `Mutexes` para evitar condição de corrida nas variáveis globais.
    * `Semáforos` para sincronizar pedidos de clientes e entregas de garçons.
      
- **Log de Atividades**: O programa mantém um log detalhado de todas as atividades ocorridas durante a simulação. Isso inclui quando um cliente faz um pedido, o garçom atende o pedido, o garçom entrega o pedido entre outras ações. Esse log ajuda na depuração e no monitoramento da simulação.

- **Configuração Dinâmica**: É possível ajustar diferentes parâmetros da simulação, como o número de clientes, garçons, mesas e duração da simulação. Esses parâmetros podem ser ajustados via linha de comando ou através de um arquivo de configuração.

## 📎 Contribuições

Contribuições são sempre bem-vindas! Aqui estão as etapas para contribuir:

1. Fork o projeto
2. Crie sua branch de feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request
   

## ✒️ Autores
* ##### [Rafael Correa Bitencourt](https://github.com/rafael-bitencourt)
* ##### [Erik Orsolin de Paula](https://github.com/erikorsolin)
* ##### [Pedro Augusto da Fontoura](https://github.com/Fontoura21)
   
   
