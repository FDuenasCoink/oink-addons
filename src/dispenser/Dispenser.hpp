/**
 * @file Dispenser.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Header de las funciones del dispensador
 * @version 1.1
 * @date 2023-05-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef DISPENSER_HPP
#define DISPENSER_HPP

#include <stdio.h>
#include <cstring> // To include strerror
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // To include errno
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/ioctl.h> //To use flush
#include <vector>

#include "spdlog/spdlog.h" //Logging library
#include "spdlog/sinks/daily_file_sink.h" //Logging library - daily file

#include <string>   //To include string definitions

namespace Dispenser{
    
    struct StatusCodesRow_t{
        std::string Status;
        std::string Message;
        int Priority;
    };

    struct ErrorCodesRow_t{
        std::string ErrorCode;
        std::string Message;
        int Priority;
    };

    struct ErrorCodeExComm_t{
        int Code;
        std::string Message;
        int Priority;
    };

    struct SpdlogLevels_t{
        int Code;
        std::string Message;
    };

    class DispenserClass{
        public:
            // --------------- EXTERNAL VARIABLES --------------------//

            //READ ONLY

            /**
             * @brief Descriptor de archivo del puerto del dispensador
             */
            int SerialPort;

            /**
             * @brief Bandera que incica si fue exitosa la conexion al puerto serial del dispensador
             */
            bool SuccessConnect;
            
            /**
             * @brief Es la bandera que indica si el dispensador ha inicializado
             */
            bool Initialized;

            /**
             * @brief Este puerto es el resultante del escaneo de la funcion ScanPorts
             */
            int PortO;

            /**
             * @brief Bandera que indica si el dispensador tiene 1 o mas tarjetas
             */
            bool CardsInDispenser;

            /**
             * @brief Bandera que indica si hay una tarjeta en la puerta
             */
            bool CardInGate;

            /**
             * @brief Bandera que indica si hay una tarjeta atorada en el dispensador
             */
            bool RFICCardInGate;

            /**
             * @brief Bandera que indica si el dispensador tiene mas de 20 tarjetas
             */
            bool DispenserFull;

            /**
             * @brief Bandera que indica si la caja de reciclaje tiene 24 o mas tarjetas
             */
            bool RecyclingBoxFull;

            /**
             * @brief Codigo de error que retorna la funcion HandleResponseError
             */
            std::string ErrorOCode;

            /**
             * @brief Mensaje de error que retorna la funcion HandleResponseError
             */
            std::string ErrorOMsg;

            //WRITE ONLY

            /**
             * @brief Nivel de logging de la libreria Spdlog
             */
            int LoggerLevel;

            /**
             * @brief Ruta donde se guardan los logs de Spdlog
             */
            std::string LogFilePath;

            /**
             * @brief Cantidad maxima de puertos que escanea la funcion ScanPorts()
             */
            int MaxPorts;

            /**
             * @brief Cantidad maxima de veces que intenta iniciar el dispensador
             */
            int MaxInitAttempts;

            /**
             * @brief Cantidad de tiempo a esperar cuando se envia un comando que no mueve el motor
             */
            int ShortTime;

            /**
             * @brief Cantidad de tiempo a esperar cuando se envia un comando que mueve el motor
             */
            int LongTime;

            // --------------- CONSTRUCTOR FUNCTIONS --------------------//

             /**
             * @brief Constructor de la clase DispenserClass
             */
            DispenserClass();

            /**
             * @brief Destructor de la clase DispenserClass
             */
            ~DispenserClass();

            // --------------- LOGGER FUNCTIONS --------------------//   

            /**
            * @brief Busca el nivel de logging Splog
            * @param Code Nivel de logging entero que va desde 0 hasta 6
            * @return SpdlogLevels_t Devuelve el codigo de logging y la palabra correspondiente al nivel
            */
            SpdlogLevels_t SearchSpdlogLevel(int Code);

            /**
             * @brief Establece el nivel de Logging de acuerdo a la variable externa "LoggerLevel"
             */
            void SetSpdlogLevel();

            // --------------- SEARCH FUNCTIONS --------------------//

            /**
            * @brief Busca la informacion adicional que entrega el dispensador cuando retorna un codigo de exito (Asociado a la tarjeta atorada o en puerta)
            * @param SCode Codigo que puede ser '0' / '1' / '2'
            * @return StatusCodesRow_t Estructura que tiene el codigo de estado, un mensaje asociado y la prioridad del mensaje
            */
            StatusCodesRow_t SearchSuccessCode0 (std::string SCode);

            /**
            * @brief Busca la informacion adicional que entrega el dispensador cuando retorna un codigo de exito (Asociado a las tarjetas disponibles)
            * @param SCode Codigo que puede ser '0' / '1' / '2'
            * @return StatusCodesRow_t Estructura que tiene el codigo de estado, un mensaje asociado y la prioridad del mensaje
            */
            StatusCodesRow_t SearchSuccessCode1 (std::string SCode);

            /**
            * @brief Busca la informacion adicional que entrega el dispensador cuando retorna un codigo de exito (Asociado a las tarjetas en la caja de reciclaje)
            * @param SCode Codigo que puede ser '0' / '1'
            * @return StatusCodesRow_t Estructura que tiene el codigo de estado, un mensaje asociado y la prioridad del mensaje
            */
            StatusCodesRow_t SearchSuccessCode2 (std::string SCode);

            /**
            * @brief Busca el codigo de error asociado al ultimop comando enviado
            * @param ErrorC Codigo de error Encontrado
            * @return ErrorCodesRow_t Estrcutura que tiene el codigo de error, el mensaje asociado y la prioridad del mensaje
            */
            ErrorCodesRow_t SearchErrorCode (std::string ErrorC);

            /**
            * @brief Busca el codigo de error entero que devuelve la funcion ExecuteCommand
            * @param Code Codigo de error entero que va desde -6 hasta 5
            * @return ErrorCodeExComm_t Estructura que tiene el codigo, el mensaje de ExecuteCommand y la prioridad del mensaje
            */
            ErrorCodeExComm_t SearchErrorCodeExComm (int Code);

            // --------------- STATES OF MACHINE STATE (FUNCTIONS) --------------------//

            /**
            * @brief Establece el nivel de logging de Spdlog, es un estado base desde donde comienza y esta solo una vez
            * @return Siempre retorna 0
            */
            int StIdle();

            /**
            * @brief Corre la funcion ScanPorts()
            * @return Retorna 0 si se encontro un puerto del 0 en adelante
            * @return Retorna 1 si no se encontro puerto
            */
            int StConnect();

            /**
            * @brief Corre la funcion InitDispenser() si el dispensador no se ha inicializado antes
            * @return Retorna 0 si el dispensador ya fue inicializado
            * @return Retorna 1 si no se pudo inicializar el dispensador
            * @return Retorna -1 si no se pudo inicializar el dispensador por problemas de escritura/lectura
            */
            int StInit();

            /**
            * @brief Corre la funcion CheckStatus()
            * @return Retorna 0 si pudo revisar exitosamente el estado del dispensador
            * @return Retorna 1 si no pudo revisar el estado del dispensador
            */
            int StWait();

            /**
            * @brief Revisa si hay tarjetas y llama la funcion de dispensar
            * @return Retorna 0 si pudo sacar una tarjeta a la puerta
            * @return Retorna 1 si no pudo sacar la tarjeta a la puerta
            */
            int StMovingMotor();

            /**
            * @brief Revisa si hay tarjeta en puerta y la manda a la caja de reciclaje
            * @return Retorna 0 si pudo mandar la tarjeta a la caja de reciclaje o si no hay tarjeta en puerta
            * @return Retorna 1 si no pudo enviar la tarjeta a la caja de reciclaje por algun error
            * @return Retorna 2 si no pudo enviar la tarjeta a la caja de reciclaje por que no detecto ninguna en la puerta
            */
            int StHandingCard();

            /**
            * @brief Estado de error
            * @return Retorna 0 si pudo revisar el estado
            * @return Retorna 1 si no pudo revisar el estado
            */
            int StError();

            // --------------- MAIN FUNCTIONS --------------------//  
            
           /**
            * @brief Abre el archivo donde se van a guardar los logs
            * @param Path Ruta donde se debe guardar el log de Spdlog
            */
            void InitLogger(const std::string& Path);

            /**
            * @brief Se conecta al puerto con las caracteristicas definidas en la hoja de datos del dispensador
            * @brief Cambia bandera de conexion exitosa/fallida
            * @brief [Solo deberia correrse una vez]
            * @param Port Puerto a conectar entero igual o mayor que cero
            * @return int - Retorna 0 si la conexion fue exitosa
            * @return int - Retorna 1 si el puerto es invalido
            * @return int - Retorna 2 si no puede leer los parametros actuales del puerto
            * @return int - Retorna 3 si no puede escribir los nuevos parametros del puerto
            * @return int - Retorna 4 si no se pudo conectar al puerto
            */
            int ConnectSerial(int Port);

            /**
            * @brief Corre la funcion ConnectSerial(n) y luego la funcion InitDispenser() en cada puerto, 
            * @brief si logra correr las dos sin errores devuelve el numero de puerto n
            * @brief [Solo deberia correrse una vez]
            * @return int - Retorna el numero de puerto desde 0 en adelante si encontro, en otro caso devuelve -1
            */
            int ScanPorts();

            /**
            * @brief Maneja la respuesta de Execute command para que solo sean 3 respuestas
            * @param Comm Comando a escribir en el puerto
            * @param AdTime Tiempo de espera en segundos
            * @return Si retorna -1 -> [SC] Hubo un error grave enviando el comando
            * @return Si retorna  0 -> [SC] Todo funciona correctamente
            * @return Si retorna  1 -> [SC] Hay que repetir el envio del comando
            * @return Si retorna  2 -> [SC] Codigo de fallo detectado
            */
            int SendingCommand(std::vector<unsigned char> Comm, int AdTime);

            /**
            * @brief Escribe el comando Comm en el puerto, luego lee la respuesta. 
            * @brief Si no detecta un ACK (0x06) en el primer caracter rechaza la respuesta 
            * @param Comm Comando a escribir en el puerto
            * @param AdTime Tiempo de espera en segundos
            * @return Si retorna -6 -> [EC] No ejecuto la funcion
            * @return Si retorna -5 -> [EC] Pudo escribir, pero no pudo leer el puerto
            * @return Si retorna -4 -> [EC] No pudo escribir en el puerto
            * @return Si retorna -3 -> [HR] La respuesta no corresponde al comando enviado
            * @return Si retorna -2 -> [HR] Respuesta de fallo identificada pero no se pudo escribir el ACK
            * @return Si retorna -1 -> [HR] Respuesta de exito identificada pero no se pudo escribir el ACK
            * @return Si retorna  0 -> [HRS] Respuesta de exito identificada
            * @return Si retorna  1 -> [HRE] Respuesta de fallo identificada
            * @return Si retorna  2 -> [HRS/E] Codigo de respuesta desconocido
            * @return Si retorna  3 -> [EC/HR] Respuesta no identificada, datos llegaron mal
            * @return Si retorna  4 -> [EC] Dispensador no responde, tiempo de espera excedido
            * @return Si retorna  5 -> [EC] El dispositivo no retorna ACK
            */
            int ExecuteCommand(std::vector<unsigned char> Comm, int AdTime);   
            
            /**
            * @brief Maneja la respuesta que llega, asegura la integridad de los datos, clasifica la respuesta en exito o fallo y envia ACK de confirmacion
            * @param Response Respuesta que envia el dispensador
            * @param Cm Comando general enviado
            * @param Pm Parametros del comando general
            * @return Si retorna -3 -> [HR] La respuesta no corresponde al comando enviado
            * @return Si retorna -2 -> [HR] Respuesta de fallo identificada pero no se pudo escribir el ACK
            * @return Si retorna -1 -> [HR] Respuesta de exito identificada pero no se pudo escribir el ACK
            * @return Si retorna  0 -> [HRS] Respuesta de exito identificada
            * @return Si retorna  1 -> [HRE] Respuesta de fallo identificada
            * @return Si retorna  2 -> [HRS/E] Codigo de respuesta desconocido
            * @return Si retorna  3 -> [HR] Respuesta no identificada, datos llegaron mal
            */
            int HandleResponse(std::vector<unsigned char> Response, int Cm, int Pm);

            /**
            * @brief Escribe un ACK al dispensador cuando el comando fue reconocido por el host
            * @return Si retorna 0 -> Se escribio correctamente el ACK
            * @return Si retorna -1 -> No se pudo escribir el ACK
            */
            int WriteAck();

            /**
            * @brief Maneja la respuesta de exito y actualiza las variables CardInGate,DispenserFull,RecyclingBoxFull
            * @param Response Respuesta que envia el dispensador
            * @param Rdlen Longitud de la respuesta
            * @return Si retorna 0 -> Respuesta de exito identificada
            * @return Si retorna 2 -> Codigo de respuesta desconocido
            */
            int HandleResponseSuccess(std::vector<unsigned char> Response);

            /**
            * @brief Maneja la respuesta de error y busca el codigo asociado
            * @param Response Respuesta que envia el dispensador
            * @param Rdlen Longitud de la respuesta
            * @return Si retorna 1 -> Respuesta de fallo identificada
            * @return Si retorna 2 -> Codigo de respuesta desconocido
            */
            int HandleResponseError(std::vector<unsigned char> Response);

            /**
            * @brief Corre la funcion ExecuteCommand enviando el comando MSGINIT
            * @brief Intenta iniciar el dispensador 3 veces si no esta en modo escaneo (ScanPorts)
            * @return Retorna 0 si el inicio fue exitoso
            * @return Retorna 1 si el inicio fue fallido 
            */
            int InitDispenser();
            
            /**
            * @brief Corre la funcion ExecuteCommand enviando el comando MSGDISPENSECARD
            * @brief Dispensa una tarjeta
            * @return Retorna -1 si hubo un error de lectura/escritura
            * @return Retorna 0 si movio exitosamente la tarjeta a la puerta
            * @return Retorna 1 si no pudo mover la tarjeta a la puerta. (Se debe revisar la puerta de salida)
            */
            int DispenseCard();

            /**
            * @brief Corre la funcion ExecuteCommand enviando el comando MSGGETSTATUS
            * @brief Revisa el estado actual del dispensador
            * @return Retorna 0 si pudo leer con exito el ultimo estado del dispensador
            * @return Retorna 1 si no pudo leer el ultimo estado del dispensador
            */
            int CheckStatus();

            /**
            * @brief Corre la funcion ExecuteCommand enviando el comando MSGRETURNCARD
            * @brief Devuelve la tarjeta en puerta a la caja de reciclaje
            * @return Retorna 0 si pudo devolver con exito la tarjeta a la caja de reciclaje
            * @return Retorna 1 si no pudo devolver con exito la tarjeta a la caja de reciclaje. (Se debe revisar la puerta de salida)
            */
            int ReturnCardToBox();

            /**
            * @brief Imprime las variables mas importantes que se revisan cuando se corre la funcion CheckStatus()
            */
            void PrintCheck();     
    };
};

#endif /* DISPENSER_HPP */
