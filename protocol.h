/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. de Ingeníería de Telecomunicación
 * Escuela Politécnia Superior de Linares
 * Universidad de Jaén
 *
 *******************************************************
 * Práctica 1
 *******************************************************
 * Fichero: protocol.h
 * Versión: 4.1
 * Curso: 2026/2027
 * Descripción: Fichero de encabezado para práctica 3
 * Autor: Juan Carlos Cuevas Martínez
 *
 *******************************************************
 * PONGA SU NOMBRE AQUÍ:
 * Estudiante 1: Nerea Gil Mora
 * Estudiante 2 [si lo hubiera]:
 *
 ******************************************************/
#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile


// COMANDOS DE APLICACION
#define HELO "HELO"
#define PASS "PASS"
#define QUIT "QUIT"
#define ECHO "ECHO"
#define ROOM "ROOM" //Comando creado para la tarea 4


// RESPUESTAS A COMANDOS DE APLICACION
#define OK  "OK"
#define ER  "ER"

//FIN DE RESPUESTA
#define CRLF "\r\n"

//ESTADOS
#define S_INIT 0
#define S_HELO 1
#define S_PASS 2
#define S_DATA 3
#define S_QUIT 4


//PUERTO DEL SERVICIO
#define TCP_SERVICE_PORT	25



#endif