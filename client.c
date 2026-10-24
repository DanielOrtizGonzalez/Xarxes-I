// client.c
// Grup: 10f
// Daniel Ortiz González
// Jordi Martorell Planas
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
	//longitud de 7 nombres maxim
	int id; 
	char nom[50];
	float mitjana;
	int edat;
} alumne;


int main(int argc, char *argv[]) {
	
	// Estrucutura d'alumne i llistas
	alumne nou; 
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
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { //L'argument AF_INET es la familia d'adreces IPv4, el SOCK_STREAM es el tipus de comunicacions en aquest cas es TCP, el "0" es el protocol en aquest cas TCP
		printf("\nError en crear el socket\n"); //Per el segon argument tambe hi ha la opcio "SOCK_DATAGRAM" que posa el tipus de comunicacio a UDP
		return -1;
	}
	// Afegiu comentari explicant els arguments, i de quines altres opcions hi ha
	// per al segon d'ells (ara SOCK_STREAM)

	// Afegiu comentari sobre què són aquests 3 paràmetres,
	// per què s'utilitzen els valors que hi ha,
	// i per què s'utilitza htons()
	// htons: De nombre enter desde el format natiu del client (el nostre ordinador) al format estandar de la xarxa
	// i per què s'utilitzen els valors que hi ha
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convertir adreça IPv4 a binari
	inet_pton(AF_INET, domini, &serv_addr.sin_addr);	
		
	// Connectar amb el servidor
	// Afegiu comentari explicant els arguments
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { //El "sock" es la direccio del socket al que ens volem connectar, "(struct sockaddr *)&serv_addr" es l'adreça del servidor transformada en un tipus que el client la pugui entendre,
		printf("\nConnexio fallida\n"); //"sizeof(serv_addr)" es la mida de l'adreça del servidor
		return -1;
	}

	printf("Connectat al servidor. Podeu començar a enviar missatges.\n");

	int option;

	//Sempre es fara el while fins que hi hagi un break
	while (1) { 
		// Netegem el buffer
		memset(buffer, 0, sizeof(buffer)); 

		// Menú principal
		// Cal que implementeu un petit servei remot amb almenys 4 funcionalitats noves
		// Definiu vosaltres mateixos les dades a enviar (demanar en el client, i analitzar al servidor) i rebre
		// Si per donar més sentit al servei cal alguna opció més, la podeu afegir
		// Deixeu l'opció 1 com a Enviar missatge i l'última per Sortir

		printf("Menú principal:\n");
		printf("1. Enviar missatge\n");
		printf("2. Afegir Alumne\n");
		printf("3. Consultar Alumne\n");
		printf("4. Llistar Alumne\n");
		printf("5. Eliminar alumne\n");
		printf("6. Sortir\n");
		printf("Opció: ");

		scanf("%d", &option);
		// buida el buffer fins al salt de línia
		while (getchar() != '\n');  

		switch (option)
		{
		case 1:
		case 6:
			if (option==1){
				printf("Introdueix el missatge a enviar ('EXIT' per tancar el servidor i sortir): ");
				fgets(cadena, BUFFER_SIZE, stdin);
				// Eliminar \n final
				cadena[strcspn(cadena, "\n")] = '\0';  
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

			//memset serveix per netejar el buffer amb " "
			memset(buffer, 0, BUFFER_SIZE); 
			recv(sock, buffer, BUFFER_SIZE,0);
			printf("Resposta del servidor: %s\n", buffer);

			if (strcmp(cadena, "EXIT") == 0) {
				close(sock);
				return 0;
			}
			break;

		case 2:{
			
			// AFEGIR ALUMNE
			memset(&nou, 0, sizeof(nou));

			//Demanem totes les dades al client i les guardem a una estructura
			printf("Introdueix la ID: ");
			scanf("%d", &nou.id); 
			// Buida el buffer fins al salt de línia perque no hi hagin problemes al introduir el nom
			while (getchar() != '\n'); 

			printf("Introdueix el nom: ");
			fgets(nou.nom, sizeof(nou.nom), stdin);
			// Eliminar \n final i el canvia per un \0
			nou.nom[strcspn(nou.nom, "\n")] = '\0';  

			printf("Introdueix mitjana: ");
			scanf("%f", &nou.mitjana);
			
			printf("Introdueix edat: ");
			scanf("%d", &nou.edat);
			// Buida el buffer fins al salt de línia perque no hi hagin problemes al introduir el nom
			while (getchar() != '\n');  
			
			//Netegem el buffer 
			memset(buffer, 0, BUFFER_SIZE);
			//Senyalitzem que es la funcio 2 la que estem demanant a la primera posició del buffer
			buffer[0] = '2';
			
			//Cargem al buffer totes les dades que volem enviar
			strncpy(buffer+1, nou.nom, 49);
			memcpy(buffer+51, &nou.id, sizeof(int));
			memcpy(buffer+55, &nou.mitjana, sizeof(float));
			memcpy(buffer+59, &nou.edat, sizeof(int));
			
			//enviem el buffer a traves del sock i borrem tota l'informacio del buffer (incluim un control d'errors al send)
			if (send(sock, buffer, BUFFER_SIZE, 0) < 0) {
				printf("Error en enviar la comanda\n");
				break;
			}
			memset(buffer, 0, BUFFER_SIZE);

			//Rebem la resposta del servidor (incluim un control d'errors al recv);
			if (recv(sock, buffer, BUFFER_SIZE - 1, 0) <= 0) {
				
				printf("Error en rebre la resposta del servidor\n");
				break;
			}
			// Mostrem la resposta del servidor
			printf("Resposta del servidor: %s\n", buffer);

			break;
		}

		case 3:

			// Consultar Alumne
			// Demanem el nom de l'alumne que volem consultar i canviem el "\n" per un "\0"
			printf("Introdueix nom de l'alumne: ");
            fgets(nou.nom, sizeof(nou.nom), stdin);
            nou.nom[strcspn(nou.nom, "\n")] = '\0';
			// Netegem el buffer 
            memset(buffer, 0, BUFFER_SIZE);
            // Indiquem que es la tercera opcio
			buffer[0] = '3';
            // Copia al buffer el nom de l'alumne pero nomes fins a 49 caracters
			strncpy(buffer + 1, nou.nom, 49);
            // Enviem el buffer al servidor el nom del alumne i posem un control d'errors
            if (send(sock, buffer, BUFFER_SIZE, 0) < 0) {
                printf("Error en enviar la comanda\n");
                break;
            }
			// Netegem el buffer 
            memset(buffer, 0, BUFFER_SIZE);
			// Rebem les dades del alumne i posem un control d'errors
            if (recv(sock, buffer, BUFFER_SIZE - 1, 0) <= 0) {
                printf("Error en rebre la resposta del servidor\n");
                break;
            }
			// Mostrem les dades del alumne
            printf("Resposta del servidor:\n%s\n", buffer);
            break;


		case 4: {
			
			// Llistar Alumne
			// Declaracio de variables
			int num_alumnes = 0;
			int total = 0;
		
			// Enviar la comanda al servidor perque sapiga que ha de llistar
			memset(buffer, 0, BUFFER_SIZE);
			// Indiquem l'opció al servidor.
			buffer[0] = '4';

			// Comprovació de que el servidor rep les dades corre
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
			total = ntohl(num_alumnes);
			printf("--- Llista d'alumnes (Total de %d) ---\n", total);

			// Bucle per rebre l'estrucutra i mostrar als alumnes
			for (int i = 0; i < total; i++) {
                alumne temp;
                int dades = recv(sock, &temp, sizeof(alumne), 0);
                if (dades <= 0) {
                    printf("Error en rebre les dades d'un alumne\n");
                    break;
                }
                printf("%d. ID: %d | Nom: %s | Mitjana: %.2f | Edat: %d\n",
                       i + 1, temp.id, temp.nom, temp.mitjana, temp.edat);
            }
			printf("--------------------------------------\n");
		
			break;
		}

		case 5: {
		
			// Eliminar Alumnes
			// Declarar variables
			int id_eliminar;
			int id_net;

			// Demarem a l'usuari que introdueixi la ID de l'alumne a elimianr
			printf("Introdueix la ID de l'alumne a eliminar: ");
            scanf("%d", &id_eliminar);
			// Buida el buffer fins al salt de línia perque no hi hagin problemes al introduir el nom
            while (getchar() != '\n');
			
			// Netejearem la memoria
            memset(buffer, 0, BUFFER_SIZE);
			// Notificarem que es l'opció 5.
            buffer[0] = '5';
			// Transforma de format de xarxa a format local
            id_net = htonl(id_eliminar);
			// Copia el valor que hi ha al buffer.
            memcpy(buffer + 1, &id_net, sizeof(int));
			
			// Enivem tot el contigut del buffer per enviar al servidor
            if (send(sock, buffer, BUFFER_SIZE, 0) < 0) {
                printf("Error en enviar la sol·licitud\n");
                break;
            }
			
			// Netejearem la memoria
            memset(buffer, 0, BUFFER_SIZE);
			// Esperem a rebre les dades del servidor.
            if (recv(sock, buffer, BUFFER_SIZE - 1, 0) <= 0) {
                printf("Error en rebre la confirmacio del servidor\n");
                break;
            }
			// Mostrem el missatge que ens retorna el servidor.
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