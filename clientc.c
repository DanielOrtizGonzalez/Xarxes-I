// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define DEFAULT_PORT 8080
#define DEFAULT_DOMAIN "127.0.0.1"
#define BUFFER_SIZE 1024

typedef struct {
    int id; 
    char nom[50];
    float mitjana;
    int edat;
} alumne;

int main(int argc, char *argv[]) {
    alumne nou;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char cadena[BUFFER_SIZE] = "";

    char *domini;
    int port;
    char *endptr;

    if (argc == 1) {
        domini = DEFAULT_DOMAIN;
        port = DEFAULT_PORT;
        printf("Cap argument indicat. Utilitzant valors per defecte: %s %d\n", domini, port);
    } 
    else if (argc == 3) {
        domini = argv[1];
        errno = 0;
        port = strtol(argv[2], &endptr, 10);

        if (errno != 0 || *endptr != '\0' || port < 1 || port > 65535) {
            fprintf(stderr, "Error: el port ha de ser un enter entre 1 i 65535.\n");
            exit(EXIT_FAILURE);
        }
    } 
    else {
        fprintf(stderr, "Ús: %s [nom_de_domini port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Crear el socket: AF_INET (IPv4), SOCK_STREAM (TCP), 0 (Protocol per defecte TCP)
    // Opció alternativa per al segon argument: SOCK_DGRAM (Protocol UDP sense connexió)
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nError en crear el socket\n");
        return -1;
    }

    // Assignació de paràmetres de l'adreça del servidor:
    // sin_family: Família d'adreces (IPv4)
    // sin_port: Port de connexió convertit a format de xarxa (Big-Endian) amb htons()
    // sin_addr: Adreça IP convertida
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, domini, &serv_addr.sin_addr) <= 0) {
        printf("\nAdreça no vàlida o no suportada\n");
        return -1;
    }
        
    // Connectar amb el servidor:
    // Argument 1: socket creat
    // Argument 2: punter a l'estructura de l'adreça del servidor (castejada a sockaddr*)
    // Argument 3: mida de l'estructura sockaddr_in
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnexio fallida\n");
        return -1;
    }

    printf("Connectat al servidor. Podeu començar a enviar missatges.\n");

    int option;

    while (1) {
        printf("\n--- Menú principal ---\n");
        printf("1. Enviar missatge\n");
        printf("2. Afegir Alumne\n");
        printf("3. Consultar Alumne\n");
        printf("4. Llistar Alumnes\n");
        printf("5. Eliminar Alumne\n");
        printf("6. Sortir\n");
        printf("Opció: ");

        if (scanf("%d", &option) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n'); 

        switch (option) {
        case 1:
        case 6:
            if (option == 1) {
                printf("Introdueix el missatge a enviar ('EXIT' per sortir): ");
                fgets(cadena, BUFFER_SIZE, stdin);
                cadena[strcspn(cadena, "\n")] = '\0';
            } else {
                strcpy(cadena, "EXIT");
            }

            if (send(sock, cadena, strlen(cadena), 0) < 0) {
                printf("Error en enviar el missatge\n");
                break;
            }

            memset(buffer, 0, BUFFER_SIZE);
            if (recv(sock, buffer, BUFFER_SIZE - 1, 0) <= 0) {
                printf("Error en rebre la resposta del servidor\n");
                break;
            }
            printf("Resposta del servidor: %s\n", buffer);

            if (strcmp(cadena, "EXIT") == 0) {
                close(sock);
                return 0;
            }
            break;

        case 2: // AFEGIR ALUMNE
            memset(&nou, 0, sizeof(nou));

            printf("Introdueix la ID: ");
            scanf("%d", &nou.id);
            while (getchar() != '\n');

            printf("Introdueix el nom: ");
            fgets(nou.nom, sizeof(nou.nom), stdin);
            nou.nom[strcspn(nou.nom, "\n")] = '\0';

            printf("Introdueix mitjana: ");
            scanf("%f", &nou.mitjana);

            printf("Introdueix edat: ");
            scanf("%d", &nou.edat);
            while (getchar() != '\n');

            // Empaquetem les dades respectant els offsets exactes
            memset(buffer, 0, BUFFER_SIZE);
            buffer[0] = '2';
            strncpy(buffer + 1, nou.nom, 49);
            memcpy(buffer + 51, &nou.id, sizeof(int));
            memcpy(buffer + 55, &nou.mitjana, sizeof(float));
            memcpy(buffer + 59, &nou.edat, sizeof(int));
            
            if (send(sock, buffer, BUFFER_SIZE, 0) < 0) {
                printf("Error en enviar la comanda\n");
                break;
            }

            memset(buffer, 0, BUFFER_SIZE);
            if (recv(sock, buffer, BUFFER_SIZE - 1, 0) <= 0) {
                printf("Error en rebre la resposta del servidor\n");
                break;
            }
            printf("Resposta del servidor: %s\n", buffer);
            break;

        case 3: // CONSULTAR ALUMNE
            printf("Introdueix nom de l'alumne: ");
            fgets(nou.nom, sizeof(nou.nom), stdin);
            nou.nom[strcspn(nou.nom, "\n")] = '\0';

            memset(buffer, 0, BUFFER_SIZE);
            buffer[0] = '3';
            strncpy(buffer + 1, nou.nom, 49);
            
            if (send(sock, buffer, BUFFER_SIZE, 0) < 0) {
                printf("Error en enviar la comanda\n");
                break;
            }

            memset(buffer, 0, BUFFER_SIZE);
            if (recv(sock, buffer, BUFFER_SIZE - 1, 0) <= 0) {
                printf("Error en rebre la resposta del servidor\n");
                break;
            }
            printf("Resposta del servidor:\n%s\n", buffer);
            break;

        case 4: { // LLISTAR ALUMNES
            int num_alumnes = 0;
            int total = 0;

            memset(buffer, 0, BUFFER_SIZE);
            buffer[0] = '4';

            if (send(sock, buffer, BUFFER_SIZE, 0) < 0) {
                printf("Error en enviar la comanda\n");
                break;
            }
            
            if (recv(sock, &num_alumnes, sizeof(num_alumnes), 0) <= 0) {
                printf("Error en rebre la quantitat d'alumnes\n");
                break;
            }

            total = ntohl(num_alumnes);
            printf("--- Llista d'alumnes (Total: %d) ---\n", total);

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

        case 5: { // ELIMINAR ALUMNE
            int id_eliminar;
            int id_net;

            printf("Introdueix la ID de l'alumne a eliminar: ");
            scanf("%d", &id_eliminar);
            while (getchar() != '\n');
            
            memset(buffer, 0, BUFFER_SIZE);
            buffer[0] = '5';
            id_net = htonl(id_eliminar);
            memcpy(buffer + 1, &id_net, sizeof(int));

            if (send(sock, buffer, BUFFER_SIZE, 0) < 0) {
                printf("Error en enviar la sol·licitud\n");
                break;
            }

            memset(buffer, 0, BUFFER_SIZE);
            if (recv(sock, buffer, BUFFER_SIZE - 1, 0) <= 0) {
                printf("Error en rebre la confirmacio del servidor\n");
                break;
            }
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