/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. de Ingeníería de Telecomunicación
 * Escuela Politécnia Superior de Linares
 * Universidad de Jaén
 *
 *******************************************************
 * Práctica 3
 *******************************************************
 * Fichero: cliente.c
 * Versión: 3.3
 * Curso: 2026/2027
 * Descripción: Cliente sencillo SMTP para IPv4 e IPv6
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************************************************
 * PONGA SU NOMBRE AQUÍ:
 * Estudiante 1:Nerea Gil Mora
 * Estudiante 2 [si lo hubiera]:
 *
 ******************************************************/
#include <stdio.h>		// Biblioteca estándar de entrada y salida
#include <ws2tcpip.h>	// Necesaria para las funciones IPv6
#include <conio.h>		// Biblioteca de entrada salida básica
#include <locale.h>		// Para establecer el idioma de la codificación de texto, números, etc.
#include "protocol.h"	// Declarar constantes y funciones de la práctica
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")//Inserta en la vinculación (linking) la biblioteca Ws2_32.lib

 /*
 Uso de IA (ChatGPT (OpenAI), GPT-5.6 Sol):

 Antes de empezar a modificar el programa utilicé ChatGPT (OpenAI) para
 entender mejor el funcionamiento del código original del cliente, ya que
 había algunas partes que no entendía bien, sobre todo la máquina de estados,
 el intercambio de mensajes con el servidor y el funcionamiento de algunas
 primitivas Socket.

 Consulta:
 "Explícame paso a paso cómo funciona este cliente, qué hace cada estado y
 cómo se utilizan las primitivas Socket."

 */


 

int validNumber(char* text)
{
	int length = (int)strlen(text);

	if (length < 1 || length > 2)
		return 0;

	for (int i = 0; i < length; i++) {
		if (text[i] < '0' || text[i] > '9')
			return 0;
	}

	return 1;
}
/*
 Uso de IA:

 Mientras estaba implementando la comprobación de la fecha utilicé ChatGPT
 para resolver una duda sobre cómo validar de forma sencilla los días que
 puede tener cada mes.

 Prompt:
 "Estoy haciendo la validación del día y del mes. ¿Cómo puedo comprobar de
 forma sencilla que el día existe realmente en ese mes, teniendo en cuenta
 que febrero siempre tiene 28 días?"

 Partes en las que me ayudó:
 - Revisión y finalización de la función validDate().
 */
int validDate(unsigned int day, unsigned int month)
{
	if (month < 1 || month > 12 || day < 1)
		return 0;

	if (month == 2)
		return day <= 28;

	if (month == 4 || month == 6 || month == 9 || month == 11)
		return day <= 30;

	return day <= 31;
}



int main(int* argc, char* argv[])
{
	SOCKET sockfd;
	struct sockaddr* server_in = NULL;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024];   //in almacena datos recibidos del servidor y out el mensaje que vamos a mandar al servidor
	int received = 0, sent = 0;                            //input almacena lo que introduce el usuario por teclado
	int status;
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256];
	char default_ip4[16] = "127.0.0.1";                    //loopback para permitir al equipo comunicarse consigo mismo
	char default_ip6[64] = "::1";

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	//Inicialización de idioma
	setlocale(LC_ALL, "es_ES.UTF8");


	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
		return(0);
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	printf("**************\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n*************\r\n");

	do {
		printf("CLIENTE> ¿Qué versión de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));       //Lee lo que escribe el usuario por teclado y lo almacena en ipdest

		if (strcmp(ipdest, "6") == 0) {                       
			//Si se introduce 6 se empleará IPv6
			ipversion = AF_INET6;
		}
		else { //Distinto de 6 se elige la versión IPv4
			ipversion = AF_INET;
		}

		sockfd = socket(ipversion, SOCK_STREAM, 0);     ///SOCKET Creates a TCP socket for the selected IP version.
		                                                //SOCKET Parameters:
		                                                //SOCKET - ipversion: address family (AF_INET for IPv4 or AF_INET6 for IPv6).
		                                                //SOCKET - SOCK_STREAM: creates a connection-oriented socket that uses a byte stream.
		                                                //SOCKET - 0: selects the default protocol for this socket type, in this case TCP.
		                                                //SOCKET Return value: a valid socket descriptor if the socket is created successfully,
		                                                //SOCKET or INVALID_SOCKET if an error occurs.

		if (sockfd == INVALID_SOCKET) {
			printf("CLIENTE> ERROR\r\n");
			WSACleanup();
			exit(-1);
		}
		else {
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets_s(ipdest, sizeof(ipdest));

			//Dirección por defecto según la familia
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET)
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);

			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET6)
				strcpy_s(ipdest, sizeof(ipdest), default_ip6);

			if (ipversion == AF_INET) {
				server_in4.sin_family = AF_INET;
				server_in4.sin_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*) & server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in6.sin6_addr);
				server_in = (struct sockaddr*) & server_in6;
				address_size = sizeof(server_in6);
			}

			//Cada nueva conexión establece el estado incial en
			status = S_INIT;

			if (connect(sockfd, server_in, address_size) == 0) {//SOCKET Requests the establishment of a TCP connection with the server.
                                                                //SOCKET Parameters:
                                                                //SOCKET - sockfd: descriptor of the socket created previously.
                                                                //SOCKET - server_in: pointer to the structure containing the server IP address and port.
                                                                //SOCKET - address_size: size in bytes of the address structure.
                                                                //SOCKET Return value: 0 if the connection is established successfully,
                                                                //SOCKET or SOCKET_ERROR (-1) if an error occurs.
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);

				//Inicio de la máquina de estados
				do {
					switch (status) {
					case S_INIT:
						// Se recibe el mensaje de bienvenida
						break;
					case S_USER:
						// establece la conexion de aplicacion 
						printf("CLIENTE> Introduzca el usuario (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", QUIT, CRLF);
							status = S_QUIT;
						}
						else {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", USER, input, CRLF);
						}
						break;
					case S_PASS:
						printf("CLIENTE> Introduzca la clave (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", QUIT, CRLF);
							status = S_QUIT;
						}
						else
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", PASS, input, CRLF);
						break;


						/*
	                    Uso de IA:

	                    Primero implementé por mi cuenta la parte del cliente correspondiente al
	                    nuevo servicio de reservas y al comando ROOM. Después utilicé ChatGPT para
	                    comprobar si la lógica que había hecho era correcta y revisar un pequeño
	                    error que apareció durante las pruebas.

	                    Consulta resumida:
	                    "He implementado el comando ROOM en el cliente, pidiendo la sala, el día y
	                    el mes y formando después el mensaje que se envía al servidor. ¿Puedes
	                    revisar si está bien y ayudarme a corregir el error que tengo?"

	                    Partes en las que me ayudó:
	                    - Revisión de la lógica del comando ROOM.
	                    - Corrección de un pequeño error en la implementación.
	                    */
					case S_DATA:
						printf("Available rooms:\r\n");
						printf("Meeting room (cw00).\r\n");
						printf("Large north coworking room (cw01).\r\n");
						printf("Large south coworking room (cw02).\r\n");
						printf("Junior coworking room (cw03).\r\n");
						do {
							printf("CLIENT> Enter room code (press enter to exit): ");
							gets_s(input, sizeof(input));

							if (strlen(input) == 0) {
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", QUIT, CRLF);
								status = S_QUIT;
								break;
							}

							if (strlen(input) != 4) {
								printf("CLIENT> The room code must contain exactly 4 characters.\r\n");
							}

						} while (strlen(input) != 4);
						if (status != S_QUIT) {
							char day[16] = "1";
							char month[16] = "1";
							unsigned int dayint = 0; // Alternativa a hacerlo con cadenas o comprobar valores
							unsigned monthint = 1; // Alternativa a hacerlo con cadenas o comprobar valores
							int valid = 0;

							do {
								printf("CLIENT> Enter day: ");
								gets_s(day, sizeof(day));

								printf("CLIENT> Enter month: ");
								gets_s(month, sizeof(month));

								if (validNumber(day) && validNumber(month)) {

									dayint = atoi(day);
									monthint = atoi(month);

									valid = validDate(dayint, monthint);
								}
								else {
									valid = 0;
								}

								if (!valid) {
									printf("CLIENT> Invalid date. Please try again.\r\n");
								}

							} while (!valid);

							// Generar mensaje ROOM = "ROOM" SP CODE SP DAY SP MONTH CRLF; Reserva
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s %u %u%s", ROOM, input, dayint, monthint, CRLF);
						}
						break;
					} 

					/*
                    Uso de IA:

                    Después de empezar a añadir el control de errores de las primitivas Socket,
                    utilicé ChatGPT para revisar lo que había hecho y corregir algunos problemas
                    que iban apareciendo durante las pruebas.

                    Consulta:
                    "He añadido comprobaciones de error a las primitivas Socket del cliente.
                    Revísame si están bien controlados los errores de connect(), send(), recv()
                    y closesocket() y dime si hay algún caso en el que el programa pueda
                    continuar cuando no debería."

                    Partes en las que me ayudó:
                    - Revisión del tratamiento de errores de las primitivas Socket.
                    - Uso de WSAGetLastError().
                    - Corrección del flujo cuando la comunicación ya no puede continuar.
                    */

					if (status != S_INIT) {
						sent = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);   //SOCKET Sends the PDU stored in buffer_out to the server through TCP.
                                                                                       //SOCKET Parameters:
                                                                                       //SOCKET - sockfd: descriptor of the socket connected to the server.
                                                                                       //SOCKET - buffer_out: buffer containing the message to be sent.
                                                                                       //SOCKET - strlen(buffer_out): number of bytes of the message to be sent.
                                                                                       //SOCKET - 0: flags; normal operation of send().
                                                                                       //SOCKET Return value: number of bytes sent if successful,
                                                                                       //SOCKET or SOCKET_ERROR (-1) if an error occurs.
						if (sent == SOCKET_ERROR) {
							printf("CLIENT> Error sending data: %d\r\n", WSAGetLastError());
							status = S_QUIT;
							continue;// La sentencia continue hace que la ejecución dentro de un
									 // bucle salte hasta la comprobación del mismo.
						}
					}

					received = recv(sockfd, buffer_in, 512, 0);    //SOCKET Receives the response sent by the server through the TCP connection.
                                                                   //SOCKET Parameters:
                                                                   //SOCKET - sockfd: descriptor of the socket connected to the server.
                                                                   //SOCKET - buffer_in: buffer where the received data will be stored.
                                                                   //SOCKET - 512: maximum number of bytes that can be received in this call.
                                                                   //SOCKET - 0: flags; normal operation of recv().
                                                                   //SOCKET Return value: number of bytes received if successful,
                                                                   //SOCKET 0 if the server closes the connection in an orderly way,
                                                                   //SOCKET or SOCKET_ERROR (-1) if an error occurs.
					if (received <= 0) {         
						DWORD error = WSAGetLastError();
						if (received < 0) {
							printf("CLIENTE> Error %d en la recepción de datos\r\n", error);
							status = S_QUIT;
						}
						else {
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							status = S_QUIT;
						}
					}
					else {
						buffer_in[received] = 0x00;
						printf("%s",buffer_in);

						//Motor de transición de estados
						switch (status) {
						case S_INIT:
							if (strncmp(buffer_in, OK, 3) == 0) {

							}
						}
						if (status != S_DATA && strncmp(buffer_in, ST220, 3) == 0){
							status++;
						}
						//Si la autenticación no es correcta se vuelve al estado S_HELO
						if (status == S_PASS && strncmp(buffer_in, OK, 2) != 0) {
							status = S_HELO;
						}
					}

				} while (status != S_QUIT);
			}
			else {
				int error_code = WSAGetLastError();
				printf("CLIENT> ERROR CONNECTING TO %s:%d. Error %d\r\n",ipdest, TCP_SERVICE_PORT, error_code);
			}                
			if (closesocket(sockfd) == SOCKET_ERROR) {     //SOCKET Closes the socket used by the client.
                                                           //SOCKET Parameters:
                                                           //SOCKET - sockfd: descriptor of the socket to be closed.
                                                           //SOCKET Return value: 0 if the socket is closed successfully,
                                                           //SOCKET or SOCKET_ERROR (-1) if an error occurs.
				printf("CLIENT> Error closing the socket: %d\r\n", WSAGetLastError());
			}                     
		}
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option = _getche();

	} while (option != 'n' && option != 'N');

	WSACleanup();
	return(0);
}
