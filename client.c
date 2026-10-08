// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define DEFAULT_PORT 8080
#define DEFAULT_DOMAIN "localhost"
#define BUFFER_SIZE 1024

//Creem l'estructura per guardar les dades de l'alumne
typedef struct{

	int id; //longitud de 7 nombre maxim
	char nom[50];
	float mitjana;
	int edat;
} alumne;


int main(int argc, char *argv[]) {
	
	// Estrucutura d'alumne i llistas
	alumne nou;
	alumne llista[200]; 
	char dades_alumne[BUFFER_SIZE];
	int sock = 0;
	struct sockaddr_in serv_addr;
	char buffer[BUFFER_SIZE] = {0};
	char cadena[BUFFER_SIZE] = "";

	char *domini;
	int port;
	char *endptr;

	if (argc == 1) {
		// Cap argument -> valors per defecte
		domini = DEFAULT_DOMAIN;
		port = DEFAULT_PORT;
		printf("Cap argument indicat. Utilitzant valors per defecte: %s %d\n", domini, port);
	} 
	else if (argc == 3) {
		domini = argv[1];

		errno = 0;
		port = strtol(argv[2], &endptr, 10);

		// Comprovacions de validesa
		if (errno != 0 || *endptr != '\0' || port < 1 || port > 65535) {
			fprintf(stderr, "Error: el port ha de ser un enter entre 1 i 65535.\n");
			exit(EXIT_FAILURE);
		}
	} 
	else {
		fprintf(stderr, "Ús: %s [nom_de_domini port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}



	// Crear el socket
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\nError en crear el socket\n");
		return -1;
	}
	// Afegiu comentari explicant els arguments, i de quines altres opcions hi ha
	// per al segon d'ells (ara SOCK_STREAM)

	// Afegiu comentari sobre què són aquests 3 paràmetres,
	// per què s'utilitzen els valors que hi ha,
	// i per què s'utilitza htons()

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	inet_pton(AF_INET, domini, &serv_addr.sin_addr);	// Convertir adreça IPv4 a binari
		
	// Connectar amb el servidor
	// Afegiu comentari explicant els arguments
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\nConnexio fallida\n");
		return -1;
	}

	printf("Connectat al servidor. Podeu començar a enviar missatges.\n");

	int option;

	while (1) {
		memset(buffer, 0, sizeof(buffer)); // Netegem el buffer

		// Menú principal
		// Cal que implementeu un petit servei remot amb almenys 4 funcionalitats noves
		// Definiu vosaltres mateixos les dades a enviar (demanar en el client, i analitzar al servidor) i rebre
		// Si per donar més sentit al servei cal alguna opció més, la podeu afegir
		// Deixeu l'opció 1 com a Enviar missatge i l'última per Sortir

		printf("Menú principal:\n");
		printf("1. Enviar missatge\n");
		printf("2. Opció 2: Afegir Alumne\n");
		printf("3. Opció 3: Consultar Alumne\n");
		printf("4. Opció 4: Llistar Alumne\n");
		printf("5. Opció 5: Eliminar alumne\n");
		printf("6. Sortir\n");
		printf("Opció: ");

		scanf("%d", &option);
		while (getchar() != '\n');  // buida el buffer fins al salt de línia

		switch (option)
		{
		case 1:
		case 6:
			if (option==1){
				printf("Introdueix el missatge a enviar ('EXIT' per tancar el servidor i sortir): ");
				fgets(cadena, BUFFER_SIZE, stdin);
				cadena[strcspn(cadena, "\n")] = '\0';  // Eliminar \n final
			} else {
				strcpy(cadena, "EXIT");
			}

			// Enviar missatge al servidor
			// Afegiu control d'errors
			// Afegiu comentari explicant els arguments

			send(sock, cadena, strlen(cadena), 0);

			// Llegir resposta del servidor
			// Afegiu un control d'errors al recv()
			// Afegiu comentari explicant què fa i per què s'utilitza memset
			// Afegiu comentari explicant els arguments de la crida a recv()

			memset(buffer, 0, BUFFER_SIZE); //memset serveix per netejar el buffer amb " "
			recv(sock, buffer, BUFFER_SIZE,0);
			printf("Resposta del servidor: %s\n", buffer);

			if (strcmp(cadena, "EXIT") == 0) {
				close(sock);
				return 0;
			}
			break;

		case 2:

			// AFEGIR ALUMNE
			//Demanem totes les dades al client i les guardem a una estructura
			printf("Introdueix la ID: ");
			scanf("%d", &nou.id); //Posible error al deixar el \n(el fgets pot ser que l'entengui com una linea acabada deixant la cadena buida)
			while (getchar() != '\n');  // buida el buffer fins al salt de línia

			printf("Introdueix el nom: ");
			fgets(nou.nom, sizeof(nou.nom), stdin);
			nou.nom[strcspn(nou.nom, "\n")] = '\0';  // Eliminar \n final

			printf("Introdueix mitjana: ");
			scanf("%f", &nou.mitjana);
			printf("Introdueix edat: ");
			scanf("%d", &nou.edat);
			while (getchar() != '\n');  // buida el buffer fins al salt de línia

			//Senyalitzem que es la funcio 2 la que estem demanant a la primera posició del buffer
			strcpy(buffer, "2");
			//Cargem al buffer totes les dades que volem enviar
			strcpy(buffer+1, nou.nom);
			memcpy(buffer+51, &nou.id, sizeof(nou.id));
			memcpy(buffer+52, &nou.mitjana, sizeof(nou.mitjana));
			memcpy(buffer+53, &nou.edat, sizeof(nou.edat));
			
			//enviem el buffer a traves del sock i borrem tota l'informacio del buffer (incluim un control d'errors al send)
			if (send(sock, buffer, BUFFER_SIZE, 0) < 0) {
				printf("Error en enviar la comanda\n");
				break;
			}
			memset(buffer, 0, BUFFER_SIZE);

			//Rebem la resposta del servidor (incluim un control d'errors al recv);
			if (recv(sock, buffer, BUFFER_SIZE,0) < 0) {
				
				printf("Error en rebre la resposta del servidor\n");
				break;
			}
			printf("Resposta del servidor: %s\n", buffer);

			break;

		case 3:
			


			// Consultar Alumne
			printf("Introdueix nom de l'alumne: ");
			fgets(nou.nom, sizeof(nou.nom), stdin);
			nou.nom[strcspn(nou.nom, "\n")] = '\0';  // Eliminar \n final

			memset(buffer, 0, BUFFER_SIZE);
			strcpy(buffer, "3");
			strcpy(buffer+1, nou.nom);
			
			if (send(sock, buffer, BUFFER_SIZE, 0) < 0) {
				printf("Error en enviar la comanda\n");
				break;
			}
			memset(buffer, 0, BUFFER_SIZE);

			//Rebem la resposta del servidor
			if (recv(sock, buffer, BUFFER_SIZE,0) < 0) {
				
				printf("Error en rebre la resposta del servidor\n");
				break;
			}
			printf("Resposta del servidor: %s\n", buffer);
			//Crearem una funcio que agafara el buffer(60 posicions) que tindra la id, el nom, la mitja i l'edat, i les ensenyara per pantalla

			printf("ID: %d\n", buffer[1]);
				//Per posar el nom hem de copiar el nom que teniem guardat a nou.nom
			printf("Nom: %s\n", nou.nom);
			printf("Mitjana: %.2d\n", buffer[52]);
			printf("Edat: %d\n", buffer[53]);
			
			
			

			break;

		case 4: {
			
			// Llistar Alumne
			// Declaracio de variables
			int num_alumnes = 0;
			int trans;
			int i;

			// Enviar la comanda al servidor perque sapiga que ha de llistar
			memset(buffer, 0, BUFFER_SIZE);
			buffer[0] = 4;

			if(send(sock,buffer, BUFFER_SIZE, 0) < 0){
				printf("Error en enviar la comanda\n");
				break;
			}
			
			// Rebre la quantitat d'alumnes que enviara el servidor
			if(recv(sock, &num_alumnes, sizeof(num_alumnes), 0) <= 0){
				printf("Error en rebre la quantitat d'alumnes\n");
				break;
			}

			/* Tradueix el format del servidor al formar del client 
			nthol: De nombre enter desde el format estandar de la xarxa al format natiu del client (el nostre ordinador)*/ 
			trans = ntohl(num_alumnes);

			printf("--- Llista d'alumnes (Total de %d) ---\n", trans);

			// Bucle per rebre l'estrucutra i mostrar als alumnes
			for(i = 0; i < trans; i++){
				
				// Llegeix les dades que envia el servidor i comprova que no hi haa errors
				int dades = recv(sock, &buffer, sizeof(alumne), 0);
				if (dades <=0){

					printf("Error en rebre les dades de l'alumne\n");
					break;
				}

				// Mostrar les dades rebudes d'alumnes
				//printf("%d. ID: %d | Nom: %s | Mitjana: %.2f | Edat: %d\n",
				//	 i + 1, nou.id, nou.nom, nou.mitjana, nou.edat);
				
				printf("ID: %d\n", buffer[51]);
				//Per posar el nom hem de copiar el nom que teniem guardat a nou.nom
				memcpy(nou.nom, &buffer[1], 50);
				printf("Nom: %s", nou.nom);
				printf("Mitjana: %.2d\n", buffer[52]);
				printf("Edat: %d\n", buffer[53]);
			
			}

			printf("--------------------------------------\n");
		
			break;
		}

		case 5: {
		
			// Eliminar Alumnes
			// Declarar variables
			int id_eliminar;
			int id_net;
			int dades_rebudes;

			// Enviar la comanda al servidor perque sapiga que ha de fer
			memset(buffer, 0, BUFFER_SIZE);
			buffer[0] = 5;

			if(send(sock, buffer, BUFFER_SIZE, 0) < 0){
				printf("Error en enviar la comanda d'eliminacio\n");
				break;
			}
			
			// Demanar el ID de l'alumne que es vol eliminar
			printf("Introdueix la ID de l'alumne a eliminar: ");
			scanf("%d", &id_eliminar);
			while (getchar() != '\n'); // Buida el buffer
			
			// Convertir l'ID en format de xarxa i enviar-lo
			id_net = htonl(id_eliminar);
			if(send(sock, &id_net, sizeof(id_net), 0) < 0){
				
				printf("Error en enviar la ID\n");
				break;
			}

			// Esperar a la confirmacio del servidor
			memset(buffer, 0, BUFFER_SIZE);
			dades_rebudes = recv(sock, buffer, BUFFER_SIZE - 1, 0);

			if(dades_rebudes <= 0){

				printf("Error en rebre la confirmacio del servidor\n");
				break;
			}
			
			// Mostrar la resposta del servidor
			printf("Resposta del servidor: %s\n", buffer);

			break;
		}

		default:
			printf("Opció invàlida\n");
			break;
		}
	}

	close(sock);
	return 0;
}



			//strcpy(buffer+1, nou.nom);
			//memcpy(buffer+51, &nou.id, sizeof(nou.id));
			//memcpy(buffer+51+sizeof(nou.id), &nou.mitjana, sizeof(nou.mitjana));
			//memcpy(buffer+51+sizeof(nou.id)+sizeof(nou.mitjana), &nou.edat, sizeof(nou.edat));