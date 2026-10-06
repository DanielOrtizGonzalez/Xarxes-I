// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

typedef struct{

	int id; //longitud de 7 nombre maxim
	char nom[50];
	float mitjana;
	int edat;

}alumne;

int main(int argc, char *argv[]) {
	int sock, new_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	char buffer[BUFFER_SIZE] = {0};
	char resposta[BUFFER_SIZE] = {0};

	int port;
	char *endptr;

	alumne llista_alumnes[200];
	int total_alumnes = 2;

	llista_alumnes[0].id = 1;
	strcpy(llista_alumnes[0].nom, "Joan");
	llista_alumnes[0].mitjana = 8.5;	
	llista_alumnes[0].edat = 20;
	
	llista_alumnes[1].id = 2;
	strcpy(llista_alumnes[1].nom, "Maria");
	llista_alumnes[1].mitjana = 9.0;
	llista_alumnes[1].edat = 22;

	if (argc == 1) {
		// Cap argument -> port per defecte
		port = DEFAULT_PORT;
		printf("Cap port indicat. Utilitzant valor per defecte: %d\n", port);
	}
	else if (argc == 2) {
		errno = 0;
		port = strtol(argv[1], &endptr, 10);

		// Comprovacions
		if (errno != 0 || *endptr != '\0' || port < 1 || port > 65535) {
			fprintf(stderr, "Error: el port ha de ser un enter entre 1 i 65535.\n");
			exit(EXIT_FAILURE);
		}
	}
	else {
		fprintf(stderr, "Ús: %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}


	// Crear el socket
	// Afegiu comentari explicant els arguments
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Error en crear el socket");
		exit(EXIT_FAILURE);
	}


	// Afegiu comentari explicant què són aquests 3 paràmetres,
	// per què s'utilitzen htons() i htonl(),
	// nthol: De nombre enter desde el format estandar de la xarxa al format natiu del client (el nostre ordinador)
	// htons: De nombre enter desde el format natiu del client (el nostre ordinador) al format estandar de la xarxa
	// i per què s'utilitzen els valors que hi ha

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);	// Per a INADDR_ANY no faria falta htonl(), però li posem per coherència
	address.sin_port = htons(port);

	// Enllaçar el socket al port especificat
	// Afegiu comentari explicant els arguments
	if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("Error en fer el bind");
		close(sock);
		exit(EXIT_FAILURE);
	}

	// Escoltar connexions entrants
	// Afegiu comentari explicant els arguments
	
	if (listen(sock, 3) < 0) {
		perror("Error en escoltar");
		close(sock);
		exit(EXIT_FAILURE);
	}

	printf("Servidor en funcionament, esperant connexions...\n");

	while (1) {
		// Acceptar connexions de clients
		// Afegiu comentari explicant els arguments i per què hi ha i cal new_socket si ja tenim sock
		// new_soccket: És el socket que s'utilitza per a la comunicació amb el client acceptat. 
		// Cada vegada que un client es connecta, accept() retorna un nou socket específic per a aquesta connexió, mentre que sock continua escoltant per a noves connexions.
		if ((new_socket = accept(sock, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
			perror("Error en acceptar la connexió");
			close(sock);
			exit(EXIT_FAILURE);
		}

		 while (1) {
			memset(buffer, 0, BUFFER_SIZE);
			
			// Llegir missatge del client
			if (recv(new_socket, buffer, BUFFER_SIZE, 0) <= 0) {
				printf("Client desconnectat (PID: %d)\n", getpid());
				break;
			}

			// Aquí haureu d'implementar l'anàlisi de la cadena rebuda per saber l'operació,
			// i si n'hi ha els arguments, executar-la i tornar el(s) resultat(s)

			if(buffer[0] == '2'){
				if(total_alumnes < 200){

					alumne nou;

					strcopy(nou.nom, buffer+1);
					nou.nom[strcspn(nou.nom, "\n")] = '\0';  // Eliminar \n final

					memcpy(&nou.id, buffer+51, sizeof(int));
					memcpy(&nou.mitjana, buffer+52, sizeof(float));
					memcpy(&nou.edat, buffer+53, sizeof(int));

					llista_alumnes[total_alumnes] = nou;
					total_alumnes++;

					printf("Alumne afegir -> ID: %d, nom: %s, mitjana: %.2f, edat: %d\n", nou.id, nou.nom, nou.mitjana, nou.edat);
					send(new_socket, "Alumne afegit correctament\n", strlen("Alumne afegit correctament\n"), 0);
				}else{
				
					send(new_socket, "Error: Llista d'alumnes plena\n", strlen("Error: Llista d'alumnes plena\n"), 0);
				}
			} else if(buffer[0] == '3'){

				char nom_buscar[50]; // Variable per guardar el nom que el client vol consultar
               	strcpy(nom_buscar, buffer + 1); // Extraiem el nom copiant des de la posició buffer + 1
               	nom_buscar[strcspn(nom_buscar, "\n")] = '\0'; // Netegem qualsevol salt de línia (\n) que hagi pogut posar el fgets del client

               	int trobat = -1; // Variable per guardar l'índex de l'alumne si el trobem (-1 vol dir no trobat)
               	for (int i = 0; i < total_alumnes; i++) { // Recorrem tota la llista d'alumnes actual
                   	if (strcmp(llista_alumnes[i].nom, nom_buscar) == 0) { // Comparem el nom de cada alumne amb el nom que busquem
                       	trobat = i; // Si coincideixen, guardem la posició on l'hem trobat
                       	break; // Sortim del bucle for perquè ja no cal seguir buscant
                   	}
               	}

               	char resposta[BUFFER_SIZE]; // Creem un buffer per preparar el missatge de resposta
               	memset(resposta, 0, BUFFER_SIZE); // Netegem el buffer de resposta amb zeros

               	if (trobat != -1) { // Si trobat és diferent de -1, vol dir que l'alumne existeix
                   	// Formatem un text amb totes les dades perquè el printf("%s", buffer) del teu client ho mostri directament per pantalla
                   	snprintf(resposta, BUFFER_SIZE, "ID: %d | Nom: %s | Mitjana: %.2f | Edat: %d",
                       	     llista_alumnes[trobat].id, llista_alumnes[trobat].nom,
                           	 llista_alumnes[trobat].mitjana, llista_alumnes[trobat].edat);
    			} 
			} else if(buffer[0] == 4){

				printf("Enviant llista de %d alumnes...\n", total_alumnes);

				int num_net = htonl(total_alumnes); // Convertim el nombre d'alumnes a format de xarxa
				send(new_socket, &num_net, sizeof(num_net), 0); // Enviem el nombre d'alumnes al client

				for(int i = 0; i < total_alumnes; i++){
					send(new_socket, &llista_alumnes[i], sizeof(alumne), 0); // Enviem cada alumne al client
				}

			} 
			

			// A continuació hi ha el codi corresponent a l'opció d'Enviar missatge, amb el retorn d'una cadena fixa,
			// i la de Sortida de la connexió/bucle si és el cas.
			// Modifiqueu la cadena de retorn per tal que sigui la que diu l'enunciat.

			printf("Missatge rebut del client (PID: %d): %s\n", getpid(), buffer);

			// Comprovar si el client vol tancar la connexió
			if (strcmp(buffer, "EXIT") == 0) {
				printf("Tancant connexió amb el client (PID: %d)...\n", getpid());
				send(new_socket, "Connexió tancada\n", strlen("Connexió tancada\n"), 0);
				break;	// Sortir del bucle
			} else { // Si no s'ha trobat cap alumne amb aquest nom
                    	strcpy(resposta, "Error: Alumne no trobat."); // Posem un missatge d'error a la resposta
                	}

                	send(new_socket, resposta, strlen(resposta), 0); // Enviem la resposta amb les dades (o l'error) al client


		}
			
				
		send(new_socket, "Missatge rebut\n", strlen("Missatge rebut\n"), 0);
			
		}
	{
		close(new_socket); // Tancar la connexió amb el client
	}

	close(sock);
	return 0;
}