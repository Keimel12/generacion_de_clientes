#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#define NUM_THREAD 2
#define LEN_STRING 250
#define TIME_USER 600
#define MAX_TIME 360
#define MIN_TIME 240

// Evitara conflictos al compartir recursos con nuestra variable global 'sec'.
pthread_mutex_t mutexBloqueo;

// Si no se realiza la compra mandar al final de la cola

struct carrito
{
    // Hace referencia al tiempo que los clientes pasaran eligiendo productos
    int time;
    // Los productos que el usuario decidio comprar
    char products[LEN_STRING];
    // Enumera la cantidad de productos que estan en el carrito
    int quantity_product;
    // Si es verdadera entonces realizamos la compra a tiempo, de lo contrario, nos manda al final de la cola
    bool completed_sale;
    struct carrito *next;
};
struct carrito *after = NULL, *back = NULL;

// Segundos de ejecucion
unsigned int sec = 0;

/*
            Notas del creador :)

    1- Las lineas 87 al 93 (que comentan la funcion pthread_join), normalmente no se 
       comentarian, pues esa funcion se encarga de limpiar recursos al terminar un hilo.
       pero en nuestro ya que los hilos se ejecutan indefinidamente, no es necesaria...
       Y sobretodo porque no consumen mucha memoria los subprocesos que se estan ejecutando

    2- Las lineas 100 a 104 fueron usadas simulando un programa comun y corriente para
       comprobar que tan bien se ejecutaban los subprocesos. (ciclo while donde pide ingresar
       un numero).
    
    3- Las lineas 195 y 201 (ambas encerradas en comentarios), son usadas para comprobar
       que la creacion de clientes se ejecutaba correctamente.

    4- La funcion display_client() es puramente auxiliar y no sirve de nada en el programa
       solo fue usada para comprobar que la creacion de clientes era sastifactoria...
       PD: No fue borrada para que puedas comprobar el funcionamiento de ella.

    5- Cuando nos referimos a 'clientes' son elementos de la cola que son generados "aleatoriamente",
       o de una forma mas entendible, serian el equivalente a personas desconocidas en frente o detras
       de ti en una cola. Mientras que 'usuario' nos referimos a nosotros mismos.

    6- Meencanta dejar estas notas de creador :]

    7- Cualquier fallo contacte con la alcaldia de caroni :v
*/

// Prototipos de funciones a usar

void *data_time();
void *dead_lock();
bool clientEmpty();
bool verify_client();
void display_client();
void decrease_time_client();
void generate_client(bool value);
unsigned long int client_time(unsigned long int min_value, unsigned long int max_value, unsigned int rest_value);

int main(int argc, char const *argv[])
{
    // Ejecutamos un srand para tener disponibilidad de numeros pseudoaleatorios
    srand(time(NULL));
    // Haciendo uso de la libreria pthread.h, declaramos los hilos a usar
    pthread_t hilo[NUM_THREAD];
    int i;
    int test;

    // Iniciamos un mutex para evitar complicaciones al ejecutar los subprocesos
    pthread_mutex_init(&mutexBloqueo, NULL);

    // Creamos los hilos
    for (i = 0; i < NUM_THREAD; i++)
    {
        if(i % 2 == 0)
        {
            if(pthread_create(&hilo[i], NULL, &data_time, NULL) != 0)
            {
                perror("Error al crear el hilo");
            }
        } else {
            if(pthread_create(&hilo[i], NULL, &dead_lock, NULL) != 0)
            {
                perror("Error al crear el hilo");
            }         
        }
    }

//    for (i = 0; i < NUM_THREAD; i++)
//    {
//        if(pthread_join(hilo[i], NULL) != 0)
//        {
//            perror("Error al unir el hilo");
//        } 
//    }

    while(1)
    {
        printf("ingrese un numero: \n");
        scanf("%d", &test);
    }

    // Destruimos el mutex, ya que dejara de ser usado para este momento
    pthread_mutex_destroy(&mutexBloqueo);
    return 0;
}

/************************************************************************************
* dead_lock: Se encargara de pausar la entrada de datos mientras no sea nuestro turno
*            en la cola. (Subproceso).  
*
************************************************************************************/

void *dead_lock()
    {
        struct timespec tiempo = {1, 0};
        struct carrito *temp;
        int count = 0;

        while(1)
        {

            while(clientEmpty() == false)
            {
                // Cuando el usuario tenga el control no sucedera nada
                if(verify_client() == false)
                {
                    pthread_delay_np(&tiempo);
                    fflush(stdin);
                } 
                else 
                {
                // Cuando tome el control un cliente el usuario no podra hacer nada.
                    getch();
                }
            }
        }
    }

/********************************************************************************
* verify_client(): Detectara cuando un usuario esta de regreso en la cola, ya que 
*                  no pudo realizar la compra a tiempo.
*
********************************************************************************/


bool verify_client()
    {
        struct carrito *temp;
        temp = after;

        if(temp -> completed_sale == false)
        {
            return false;
        } else {
            return true;
        }
    }

/**********************************************************************************
* data_time: Se encargara de contar los segundos que han pasado desde el inicio del
*            programa y ejecutar las funciones necesarias. (Subproceso).
*
**********************************************************************************/

void *data_time()
    {
        struct timespec tiempo = {1, 0};
        struct carrito *temp;
        generate_client(true); 

        unsigned int contador_final = client_time(MIN_TIME, MAX_TIME, 0);
        unsigned int contador_inicial = 0;
        unsigned int count = 0;

        while(1)
        {
            pthread_delay_np(&tiempo);
            sec++;
            contador_inicial++;
//            display_client();

            // En caso de que se igualen los valores, se creara un nuevo cliente
            if(contador_inicial == contador_final)
            {
                pthread_mutex_lock(&mutexBloqueo);
                generate_client(false);
//                system("cls");
                contador_final = client_time(MIN_TIME, MAX_TIME, 0);
                contador_inicial = 0;
                pthread_mutex_unlock(&mutexBloqueo);
            }

            if(sec == TIME_USER)
            {
                printf("Se ha terminado su tiempo\n");
            }

            // Disminuira el tiempo que el usuario tiene para realizar la compra
            if(clientEmpty() == false)
            {
                pthread_mutex_lock(&mutexBloqueo); 
                decrease_time_client();             
                pthread_mutex_unlock(&mutexBloqueo); 
            }

            // Si el usuario no le dio tiempo para hacer la compra, y ya es su turno
            // para ralizar la compra, le saldra este mensaje.
            if(verify_client() == false)
            {
                if(count == 1)
                {
                    printf("*Mensaje de bienvenida promedio*\n");     
                    count = 0;
                }
            } 
            else 
            {
                // Si el usuario no pudo realizar la compra a tiempo, le saldra este
                // mensaje para hacerle saber que tiene que esperar su turno en la cola.
                if(count == 0)
                {
                    printf("*Mensaje de esperar en cola promedio*\n");
                    count = 1;
                }
            }            
        }
    }

/*******************************************************************************
* ClientEmpty(): Permite determinar si la cola esta vacia, para evitar underflow
*                Retorna 'true' si la cola esta vacia, de lo contrario 'false'.
*******************************************************************************/

bool clientEmpty()
    {
        struct carrito *temp;
        temp = after;

        if(after == NULL)
        {
            return true;
        } else {
            return false;
        }
    }

/************************************************************************************
* remove_client(): Permite remover el primer elemento de la cola, y en caso de que el
*                  usuario se le acabe el tiempo y no pudo realizar su compra, lo
*                  reinsertara en la cola.
************************************************************************************/

void remove_client()
    {
        struct carrito *temp;   
        temp = after;

        if(temp != NULL)
        {
            if(temp -> completed_sale == true)
            {
                after = after -> next;
                free(temp);
            } else {
                after = after -> next;
                generate_client(true);
                free(temp);
            }
        } 
        else {
            after = NULL;
            back = NULL;
        }
    }

/*********************************************************************************
* decrease_time_client(): Descontara el tiempo que transcurra del usuario mientras
*                         no realice la compra de los productos. (En 1 segundo).
*********************************************************************************/

void decrease_time_client()
    {
        struct carrito *temp;
        temp = after;
    
        if(temp->time != 0) 
        {
           temp->time = temp->time-1;

            if((temp->time == 0) && (temp != NULL))
            { 
                remove_client();
            }
        }   
        
    }

/**************************************************************************************
* client_time(): Proporcionara un valor pseudoaleatorio al tiempo que los clientes
*                podran transcurrir el realizar su compra.
*
*                   Argumentos
*                   ----------
*
* [unsigned long int] min_value: El tiempo minimo que el cliente estara en la cola
* [unsigned long int] max_value: El tiempo maximo que el cliente estara en la cola
* [unsigned int] rest_value: Un valor que sera restado al tiempo para ser mas aleatorio
**************************************************************************************/

unsigned long int client_time(unsigned long int min_value, unsigned long int max_value, unsigned int rest_value)
    {
        // Almacenara el tiempo total.
        int value = 0;

        // Generamos un valor aelatorio entre el max_time y min_time a traves de un rango 
        value = (rand() % (max_value - min_value + 1)) + min_value;
        
        // Se resta con un valor establecido.
        if(rest_value > 0)
        {
            value = value - rand() % rest_value;            
        }

        return value;
    }

/***********************************************************************************
* display_client(): Funcion completamente auxiliar que sera usada para demostrar que
*                   las funciones se ejecutan correctamente... Al menos en esta pc 
*   :]  :)  :p  :$  :v 
***********************************************************************************/

void display_client()
    {
        struct carrito *temp;
        temp = after;

        while(temp != NULL)
        {
            printf("[%d] [%d]\n", temp->time, temp->completed_sale);
            temp = temp->next;
        }
    }   

/***********************************************************************************
* generate_client(): Generara a los clientes y al usuario (en caso de no realizar la
*                    la compra a tiempo) con su respectivo tiempo limite.
*
*               Argumento
*               ---------
*
* [bool] value: En caso de ser 'true' significa que se creara un cliente, en caso
*               contrario, es decir 'false' creara un usuario.
***********************************************************************************/

void generate_client(bool value)
    {
        unsigned short int rest_value = 0;

        struct carrito *temp;

        // Reservamos memoria para el nodo
        temp = malloc(sizeof(struct carrito));
        
        // Creacion del usuario
        if(value == true)
        {
            temp->time = TIME_USER;
            temp->completed_sale = false;
        } 
        // Creacion de clientes
        if(value == false)
        {
            temp->time = client_time(MIN_TIME, MAX_TIME, rest_value);
            temp->completed_sale = true;
        }
        temp->next = NULL;
        
        if(back == NULL)
        { 
            after = temp;
            back = temp; 
        }
        else

        back -> next = temp;
        back = temp; 
    }