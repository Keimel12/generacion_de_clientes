#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

/*
	Notas del creador :p

	1- La generacion de clientes son cada 4 a 6 min... pero legalmente como se generan
	tambien deben ser capaces de eliminarse, aqui nace un problema y es que no tenemos
	especificado el tiempo para eliminar el cliente. Entonces el tiempo que ocupara en 
	generarse tambien sera el tiempo que tardara en eliminarse. Ez.

	2- Le dejo la funcion display_client() para que puedas ver el funcionamiento.

	3- El condicional sec == 429000000 es usado para evitar un desbordamiento en el futuro
	del programa... Aunque duraria como unos simples 136 años hasta que pase eso xd.

	4- Falta poner al usuario(nosotros) con un valor de true.

	5- .... Creo que seria todo 
*/


struct cliente
{
	// Hace referencia al tiempo que los clientes pasaran eligiendo productos
	int time;

	// Separa a los clientes generados(con un valor de false), del usuario(con un valor de true)
	bool is_i;
	struct cliente *next;
};

struct cliente *after = NULL, *back = NULL;

unsigned long int client_time(unsigned long int min_value, unsigned long int max_value, unsigned int rest_value);
void generate_client();
void generate_user();
void display_client();
void remove_client();
void *subprocess();
void decrease_time_client();
int client_void();


int main(void)
{
	int opcion;

	pthread_t thread1;
	pthread_create(&thread1, NULL, subprocess, NULL);

	do
	{
		printf("- Presione '1' ver la cola\n");
		printf("- Presione '2' para salir\n");

		scanf("%d", &opcion);
		fflush(stdin);

		switch(opcion)
		{
			case 1:	display_client();
					break;
			case 2: printf("Haz salido del programa\n");
					break;
		}

	} while(opcion != 2);

	return 0;
}

/**************************************************************************************
*	client_time():	Genera aleatoriamente el tiempo que tardaran los clientes en elegir
*				  	sus productos en cola.
*
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

/******************************************************************
* generate_client(): Genera un nuevo elemento en la cola (cliente)
*					 el cual es incorporado.
*
******************************************************************/

void generate_client()
	{
		// Es el plazo maximo de tiempo que tardaran en escoger sus productos y concretar la compra
		unsigned short int max_time = 360;
		// Es el plazo minimo de tiempo q ue tardaran en escoger sus productos y concretar la compra
		unsigned short int min_time = 270;
		unsigned short int rest_value = 30;


		time_t t; 
		srand((unsigned) time(&t));

		struct cliente *aux;

		// Reservamos memoria para el nodo
		aux = malloc(sizeof(struct cliente));
		aux->time = client_time(min_time, max_time, rest_value);
		aux->is_i = false;
		aux->next = NULL;
		
		if(back == NULL)
		{ 
			after = aux;
			back = aux; 
		}
		else

		back -> next = aux;
		back = aux; 
	}


void display_client()
	{
		struct cliente *temp;

		temp = after;

		while(temp != NULL)
		{
			printf("[%d] [%d]\n", temp->time, temp->is_i);
			temp = temp->next;
		}
	}	

/****************************************************************************
* remove_client():	Se encarga de remover el cliente cuando ya paso su tiempo
*					limite.
*
****************************************************************************/

void remove_client()
	{
		struct cliente *temp;	

		temp = after;

		if(temp != NULL)
		{
			after = after -> next;
			free(temp);
		}

		else
		{
			after = NULL;
			back = NULL;
		}
	}

void *subprocess()
	{
		// Es el plazo maximo de tiempo que tardaran en escoger sus productos y concretar la compra
		unsigned short int max_time = 360;
		// Es el plazo minimo de tiempo que tardaran en escoger sus productos y concretar la compra
		unsigned short int min_time = 240;

		struct timespec tiempo = {1 , 0};
		struct cliente *temp;

		unsigned int sec = 0;
		unsigned int auxiliar = client_time(min_time, max_time, 0);
		unsigned int auxiliar_contador = 0;

		while(1)
		{
			pthread_delay_np(&tiempo);

			++sec;
			++auxiliar_contador;
			temp = after;

			if(auxiliar == auxiliar_contador)
			{
				printf("Se ha generado un elemento de cola nueva\n");
				generate_client();
				auxiliar = client_time(min_time, max_time, 0);
				auxiliar_contador = 0;
			}

			if((sec % 600 == 0) && (temp->is_i == false))
			{
				printf("Se ha terminado su tiempo\n");
				generate_user();
			}	

			if((temp != NULL) && (temp->is_i == false))
			{
				decrease_time_client();
			}

			// Evita que sec se desborde debido a su data type
			if (sec == 429000000)
			{
				sec = 0;
			}
		}
	}	

/*************************************************************************************
* decrease_time_client():	Cada vez que pase 1 segundo en el programa, se encarga de 
*							disminuir un segundo al tiempo de los clientes.				
*
*************************************************************************************/


void decrease_time_client()
	{
		struct cliente *temp;
		temp = after;
				
		if(temp->time != 0) 
		{
			temp->time = temp->time-1;

			if((temp->time == 0) && (client_void() == false))
			{ 
				printf("Se ha removido un elemento de cola\n");
				remove_client();
			}
		}
	}

/*************************************************************************************
* client_void(): Determina si la cola esta vacia(true) o hay elementos en ella(false).
*	
*************************************************************************************/


int client_void()
	{
		if(after == NULL)
		{
			return true;
		}
		else
			return false;
	}

/******************************************************************************************
* generate_user(): 	Permite al usuario volver a la cola, despues de terminar los 10 minutos
*					de su tiempo.
*
******************************************************************************************/

void generate_user()
	{
		struct cliente *temp;

		temp = malloc(sizeof(struct cliente));
		temp->time = 40;
		temp->is_i = true;
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