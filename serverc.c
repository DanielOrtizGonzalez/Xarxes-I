// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    int id; 
    char nom[50];
    float mitjana;
    int edat;
} alumne;

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

    // Dades inicials
    llista_alumnes[0].id = 1;
    strcpy(llista_alumnes[0].nom, "Joan");
    llista_alumnes[0].mitjana = 8.5;    
    llista_alumnes[0].edat = 20;
    
    llista_alumnes[1].id = 2;
    strcpy(llista_alumnes[1].nom, "Maria");
    llista_alumnes[1].mitjana = 9.0;
    llista_alumnes[1].edat = 22;

    if (argc == 1) {
        port = DEFAULT_PORT;
        printf("Cap port indicat. Utilitzant valor per defecte: %d\n", port);
    }
    else if (argc == 2) {
        errno = 0;
        port = strtol(argv[1], &endptr, 10);

        if (errno != 0 || *endptr != '\0' || port < 1 || port > 65535) {
            fprintf(stderr, "Error: el port ha de ser un enter entre 1 i 65535.\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        fprintf(stderr, "Ús: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error en crear el socket");
        exit(EXIT_FAILURE);
    }

    // Assignació de l'adreça del servidor:
    // sin_family: Família IPv4
    // sin_addr.s_addr: Accepta qualsevol adreça IP local amb INADDR_ANY
    // sin_port: Port convertit a ordrement de xarxa (Big-Endian) amb htons()
    // htonl() / htons(): Convertir dades de format natiu de la màquina host al format estàndard de xarxa.
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY); 
    address.sin_port = htons(port);

    // Enllaçar socket (bind)
    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error en fer el bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Escoltar connexions
    if (listen(sock, 3) < 0) {
        perror("Error en escoltar");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Servidor en funcionament, esperant connexions...\n");

    while (1) {
        // accept(): Retorna new_socket específic per a la comunicació amb el client connectat,
        // mentre que sock es manté escoltant noves connexions entrants.
        if ((new_socket = accept(sock, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Error en acceptar la connexió");
            continue;
        }

        printf("Client connectat.\n");

        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            
            int bytes_rebuts = recv(new_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_rebuts <= 0) {
                printf("Client desconnectat.\n");
                break;
            }

            // OPCIÓ 2: AFEGIR ALUMNE
            if (buffer[0] == '2') {
                if (total_alumnes < 200) {
                    alumne nou;
                    memset(&nou, 0, sizeof(nou));

                    strncpy(nou.nom, buffer + 1, 49);
                    memcpy(&nou.id, buffer + 51, sizeof(int));
                    memcpy(&nou.mitjana, buffer + 55, sizeof(float));
                    memcpy(&nou.edat, buffer + 59, sizeof(int));

                    llista_alumnes[total_alumnes] = nou;
                    total_alumnes++;

                    printf("Alumne afegit -> ID: %d, Nom: %s, mitjana: %.2f, edat: %d\n", nou.id, nou.nom, nou.mitjana, nou.edat);
                    send(new_socket, "Alumne afegit correctament\n", strlen("Alumne afegit correctament\n"), 0);
                } else {
                    send(new_socket, "Error: Llista d'alumnes plena\n", strlen("Error: Llista d'alumnes plena\n"), 0);
                }
            } 
            // OPCIÓ 3: CONSULTAR ALUMNE
            else if (buffer[0] == '3') {
                char nom_buscar[50] = {0};
                strncpy(nom_buscar, buffer + 1, 49);

                int trobat = -1; //Com pot ser la posicio 0
                for (int i = 0; i < total_alumnes; i++) {
                    if (strcmp(llista_alumnes[i].nom, nom_buscar) == 0) {
                        trobat = i;
                        break;
                    }
                }

                memset(resposta, 0, BUFFER_SIZE);
                if (trobat != -1) {
                    snprintf(resposta, BUFFER_SIZE, "ID: %d | Nom: %s | Mitjana: %.2f | Edat: %d",
                             llista_alumnes[trobat].id, llista_alumnes[trobat].nom,
                             llista_alumnes[trobat].mitjana, llista_alumnes[trobat].edat);
                } else {
                    strcpy(resposta, "Error: Alumne no trobat.");
                }

                send(new_socket, resposta, strlen(resposta), 0);
            } 
            // OPCIÓ 4: LLISTAR ALUMNES
            else if (buffer[0] == '4') {
                printf("Enviant llista de %d alumnes...\n", total_alumnes);

                int num_net = htonl(total_alumnes);
                send(new_socket, &num_net, sizeof(num_net), 0);

                for (int i = 0; i < total_alumnes; i++) {
                    send(new_socket, &llista_alumnes[i], sizeof(alumne), 0);
                }
            } 
            // OPCIÓ 5: ELIMINAR ALUMNE
            else if (buffer[0] == '5') {
                int id_net;
                memcpy(&id_net, buffer + 1, sizeof(int));
                int id_eliminar = ntohl(id_net);

                int trobat = -1;
                for (int i = 0; i < total_alumnes; i++) {
                    if (llista_alumnes[i].id == id_eliminar) {
                        trobat = i;
                        break;
                    }
                }

                memset(resposta, 0, BUFFER_SIZE);
                if (trobat != -1) {
                    for (int i = trobat; i < total_alumnes - 1; i++) {
                        llista_alumnes[i] = llista_alumnes[i + 1];
                    }
                    total_alumnes--;
                    strcpy(resposta, "Alumne eliminat correctament.");
                } else {
                    strcpy(resposta, "Error: No s'ha trobat cap alumne amb aquesta ID.");
                }

                send(new_socket, resposta, strlen(resposta), 0);
            }
            // OPCIÓ 1 / EXIT O ALTRES
            else {
                if (strcmp(buffer, "EXIT") == 0) {
                    printf("Tancant connexió amb el client...\n");
                    send(new_socket, "Connexió tancada\n", strlen("Connexió tancada\n"), 0);
                    break;
                } else {
                    snprintf(resposta, BUFFER_SIZE, "Missatge rebut: %.900s", buffer);
                    send(new_socket, resposta, strlen(resposta), 0);
                }
            }
        }

        close(new_socket);
    }

    close(sock);
    return 0;
}