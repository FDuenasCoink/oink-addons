/**
 * @file ValidatorAzkoyen.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Header de las funciones del validador Azkoyen
 * @version 1.1
 * @date 2023-05-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef VALIDATORAzkoyen
#define VALIDATORAzkoyen

#include <stdio.h>
#include <cstring> // To include strerror
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // To include errno
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/ioctl.h> //To use flush
#include <bitset> //To use bitset in HandleResponseInfo

#include "spdlog/spdlog.h" //Logging library
#include "spdlog/sinks/daily_file_sink.h" //Logging library - daily file

#include <vector>

namespace ValidatorAzkoyen{

    struct SpdlogLevels_t{
        int Code;
        std::string Message;
    };

    struct ErrorCodePolling_t{
        int Code;
        std::string Message;
        int Static;
        int Critical;
    };

    struct CoinPolling_t{
        int Channel;
        int Coin;
    };

    struct ErrorCodeExComm_t{
        int Code;
        std::string Message;
    };

    struct FaultCode_t{
        int Code;
        std::string Message;
    };


    class AzkoyenClass{
        public:

            // --------------- EXTERNAL VARIABLES --------------------//

            // READ ONLY

            /**
             * @brief Descriptor de archivo del puerto del validador
             */
            int SerialPort;

            /**
             * @brief Bandera que incica si fue exitosa la conexion al puerto serial del validador
             */
            bool SuccessConnect;

            /**
             * @brief Este puerto es el resultante del escaneo de la funcion ScanPorts
             */

            int PortO;

            /**
             * @brief Evento actual en el validador (Funcion StPolling)
             */
            int CoinEvent;

            /**
             * @brief Evento anterior en el validador (Funcion StPolling)
             */
            int CoinEventPrev;

            /**
             * @brief Cantidad de monedas de 50 faltantes, cuando hay perdida de eventos
             */
            int CoinCinc;
            /**
             * @brief Cantidad de monedas de 100 faltantes, cuando hay perdida de eventos
             */
            int CoinCien;
            /**
             * @brief Cantidad de monedas de 200 faltantes, cuando hay perdida de eventos
             */
            int CoinDosc;
            /**
             * @brief Cantidad de monedas de 500 faltantes, cuando hay perdida de eventos
             */
            int CoinQuin;
            /**
             * @brief Cantidad de monedas de 1000 faltantes, cuando hay perdida de eventos
             */
            int CoinMil;
            
            /**
             * @brief Bandera que indica si sucedio un error (Funcion StPolling)
             */
            bool ErrorHappened;

            /**
             * @brief Bandera que indica si sucedio un error critico en alguno de los 5 eventos (Funcion StPolling)
             */
            bool CriticalError;

            /**
             * @brief Codigo de error de polling
             */
            int ErrorOCode;

            /**
             * @brief Mensaje de error de polling
             */
            std::string ErrorOMsg;

            /**
             * @brief Indica si la moneda fue rechazada mientras corria 'polling'
             */
            int ErrorOStatic;

            /**
             * @brief Indica si el error es tan critico como para cerrar el proceso
             */
            int ErrorOCritical;
            
            /**
             * @brief Codigo de falla general en el validador
             */
            int FaultOCode;

            /**
             * @brief Mensaje asociado al codigo de falla general en el validador
             */
            std::string FaultOMsg;

            /**
             * @brief Bandera que indica si hubo un cambio en el bit no usado de los 4 bits correspondientes a los optoestados
             */
            bool NoUsedBit;

            /**
             * @brief Bandera que indica si el sensor de medida esta bloqueado
             */
            bool MeasurePhotoBlocked;

            /**
             * @brief Bandera que indica si el sensor de salida esta bloqueado
             */
            bool OutPhotoBlocked;

            /**
             * @brief Bandera que indica si se activo la alerta de moneda en cuerda en el validador
             */
            bool COSAlert;

            /**
             * @brief Ultima moneda reconocida
             */
            int ActOCoin;
            /**
             * @brief Ultimo canal detectado asociado a la moneda reconocida
             */
            int ActOChannel;

            // WRITE ONLY

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

            // --------------- CONSTRUCTOR FUNCTIONS --------------------//

            /**
             * @brief Constructor de la clase AzkoyenClass
             */
            AzkoyenClass();
            
            /**
             * @brief Destructor de la clase AzkoyenClass
             */
            ~AzkoyenClass();

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
            * @brief Busca el codigo de error entero que devuelve la funcion ExecuteCommand
            * @param Code Codigo de error entero que va desde -6 hasta 6
            * @return ErrorCodeExComm_t Estructura que tiene el codigo y el mensaje de ExecuteCommand
            */
            ErrorCodeExComm_t SearchErrorCodeExComm (int Code);

            /**
            * @brief Funcion que busca la moneda asociada al canal ingresado
            * @param Channel Canal de moneda que informa el validador
            * @return CoinPolling_t Estructura que contiene un canal y un valor de moneda entero
            */
            CoinPolling_t SearchCoin (int Channel);

            /**
            * @brief Funcion que busca un mensaje de error, una bandera de rechazo y una bandera de error critico en el polling de acuerdo al codigo ingresado
            * @param Code Codigo de error en polling
            * @return CoinPolling_t Estructura que contiene un codigo, un mensaje, una bandera de rechazo y una bandera de error critico
            */
            ErrorCodePolling_t SearchErrorCodePolling (int Code);

            /**
            * @brief Funcion que busca un codigo de fallo para el header 232 de CCtalk
            * @param Code Codigo de fallo
            * @return CoinPolling_t Estructura que contiene un codigo de fallo y un mensaje
            */
            FaultCode_t SearchFaultCode (int Code);

            // --------------- STATES OF MACHINE STATE (FUNCTIONS) --------------------//

            /**
            * @brief Primer estado de la maquina de estados, configura el logger de spdlog
            * @return int - Retorna 0 siempre
            */
            int StIdle();

            /**
            * @brief Segundo estado, escanea los puertos, envia un comando simplepoll al validador y se conecta al puerto que le responda exitosamente
            * @return int - Retorna 0 si se pudo conectar al puerto
            * @return int - Retorna 1 si no se pudo conectar al puerto
            */
            int StConnect();

            /**
            * @brief Tercer estado, revisa la comunicacion, corre revision inicial y lee los opto estados
            * @return int - Retorna 0 si pudo correr las 3 anteriores funciones exitosamente
            * @return int - Retorna 1 si no se pudo correr alguna de las 3 funciones anteriores o si hubo algun codigo de falla grave
            * @return int - Retorna 2 si el mecanismo COS fue activado o si hubo un cambio del bit 0 (NotUsed)
            */
            int StCheck();

            /**
            * @brief Cuarto estado, resetea el validador, habilita las monedas, revisa que el evento este en cero
            * @return int - Retorna 0 si pudo correr las 3 anteriores funciones exitosamente
            * @return int - Retorna 1 si no se pudo correr alguna de las 3 funciones anteriores o si el evento no esta en cero
            */
            int StWaitPoll();

            /**
            * @brief Quinto estado, hace una revision de los ultimos 5 eventos sucedidos en el validador
            * @return Si retorna -2 -> [SC] Hubo un error haciendo el polling
            * @return Si retorna -1 -> [SC] Hubo un error grave enviando el comando
            * @return Si retorna  0 -> [SC] Todo funciona correctamente
            * @return Si retorna  1 -> [SC] Hay que repetir el envio del comando, ya que el validador no lo pudo reconocer
            * @return Si retorna  2 -> [SC] No ejecuto el comando
            */
            int StPolling();

            /**
            * @brief Septimo estado, resetea el validador y corre una revision de los canales para revisar que el evento se haya reseteado a 0
            * @return int - Retorna 0 si pudo correr las 2 anteriores funciones exitosamente
            * @return int - Retorna 1 si no se pudo correr alguna de las 2 funciones anteriores o si el evento no esta en cero
            */
            int StReset();

            /**
            * @brief Estado de error, revisa la conexion 
            * @return int - Retorna 0 si la conexion esta bien
            * @return int - Retorna 1 si el validador no responde bien al comando simplepoll
            */
            int StError();

            // --------------- MAIN FUNCTIONS --------------------//
            
            /**
            * @brief Abre el archivo donde se van a guardar los logs
            * @param Path Ruta donde se debe guardar el log de Spdlog
            */
            void InitLogger(const std::string& Path);
            
            /**
            * @brief Se conecta al puerto con las caracteristicas definidas en la hoja de datos del validador
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
            * @brief Corre la funcion ConnectSerial(n) y luego la funcion SimplePoll() en cada puerto, 
            * @brief si logra correr las dos sin errores devuelve el numero de puerto n
            * @brief [Solo deberia correrse una vez]
            * @return int - Retorna el numero de puerto desde 0 en adelante si encontro, en otro caso devuelve -1
            */
            int ScanPorts();

            /**
            * @brief Maneja la respuesta de Execute command para que solo sean 5 respuestas
            * @param Comm Comando a escribir en el puerto
            * @return Si retorna -2 -> [SC] Hubo un error haciendo el polling
            * @return Si retorna -1 -> [SC] Hubo un error grave enviando el comando
            * @return Si retorna  0 -> [SC] Todo funciona correctamente
            * @return Si retorna  1 -> [SC] Hay que repetir el envio del comando, ya que el validador no lo pudo reconocer
            * @return Si retorna  2 -> [SC] No ejecuto el comando
            */
            int SendingCommand(std::vector<unsigned char> Comm);

            /**
            * @brief Escribe el comando Comm en el puerto, luego lee la respuesta. 
            * @brief Si la respuesta no es mayor o igual que la longitud del comando escrito, envia codigo diferente de 0
            * @brief Si la longitud de la respuesta es igual a la longitud del comando escrito menos uno (longitud real), quiere decir que no reconoce el comando o la direccion de destino 
            * @param Comm Comando a escribir en el puerto
            * @return Si retorna -6 -> [EC|HR|HRP|HRI] No ejecuto el comando EC|HR|HRP|HRI
            * @return Si retorna -5 -> [EC] Hubo un error de escritura de comando
            * @return Si retorna -4 -> [EC|HRI] Hubo un error de lectura de respuesta
            * @return Si retorna -3 -> [EC] El validador no responde, tiempo de espera agotado
            * @return Si retorna -2 -> [HR] El validador esta ocupado
            * @return Si retorna -1 -> [HR] ACK negativo recibido
            * @return Si retorna  0 -> [HR] Respuesta de exito identificada
            * @return Si retorna  1 -> [HR] Dato desconocido en posicion de ACK
            * @return Si retorna  2 -> [HR] El mensaje no se recibio completo
            * @return Si retorna  3 -> [HRP] Los datos que recibe del polling son incorrectos, tal vez hay que resetear el validador
            * @return Si retorna  4 -> [HRP] El validador detecto un error en el polling
            * @return Si retorna  5 -> [EC] La respuesta llego muy corta, no se dio el tiempo de espera suficiente para leer
            * @return Si retorna  6 -> [EC|HR] La respuesta es la misma que el comando, el validador no reconoce el comando
            */
            int ExecuteCommand(std::vector<unsigned char> Comm);

            /**
            * @brief Maneja la respuesta que llega, asegura la integridad de los datos, clasifica la respuesta en exito o fallo y envia ACK de confirmacion
            * @param Response Respuesta que envia el validador
            * @param Rdlen Longitud de la respuesta
            * @param Xlen Longitud del comando
            * @return Si retorna -6 -> [HR] No ejecuto el comando
            * @return Si retorna -2 -> [HR] El validador esta ocupado
            * @return Si retorna -1 -> [HR] ACK negativo recibido
            * @return Si retorna  0 -> [HR] Respuesta de exito identificada
            * @return Si retorna  1 -> [HR] Dato desconocido en posicion de ACK
            * @return Si retorna  2 -> [HR] El mensaje no se recibio completo
            * @return Si retorna  6 -> [HR] Error en el header, no se reconoce el comando
            */
            int HandleResponse(std::vector<unsigned char> Response, int Rdlen, int Xlen);

            /**
            * @brief Maneja la respuesta que llega, detecta el ACK, calsifica la respuesta en polling, info o las demas
            * @param Response Respuesta que envia el validador
            * @param Rdlen Longitud de la respuesta
            * @return Si retorna -6 -> [HRP] No ejecuto el comando
            * @return Si retorna  0 -> [HRP] El comando de polling corrio exitosamente
            * @return Si retorna  3 -> [HRP] Los datos que recibe del polling son incorrectos, tal vez hay que resetear el validador
            * @return Si retorna  4 -> [HRP] El validador detecto un error en el polling
            */
            int HandleResponsePolling(std::vector<unsigned char> Response, int Rdlen);

            /**
            * @brief Maneja la respuesta del comando self check o de read opto states
            * @param Response Respuesta que envia el validador
            * @param Rdlen Longitud de la respuesta
            * @return Si retorna -6 -> [HRI] No ejecuto el comando
            * @return Si retorna -4 -> [EC|HRI] Hubo un error de lectura de respuesta
            * @return Si retorna  0 -> [HRI] El comando pudo ser identificado y las variables importantes fueron extraidas
            */
            int HandleResponseInfo(std::vector<unsigned char> Response, int Rdlen);

            /**
            * @brief Corre el comando CMDREADOPTOST para saber 4 cosas, si hay algo en la bandeja, si la puerta esta abierta y si los dos sensores de monedas estan bien
            * @return Si retorna -2 -> Hay que repetir el comando, ya que no lo pudo ejecutar
            * @return Si retorna -1 -> Error grave enviando el comando
            * @return Si retorna  0 -> Todo corrio exitosamente, las 4 cosas tienen estado OK
            * @return Si retorna  1 -> Alguno de los 2 sensores esta bloqueado
            * @return Si retorna  2 -> Hay algo en la bandeja o la puerta de basura esta abierta
            */
            int CheckOptoStates();

            /**
            * @brief Corre el comando CMDSIMPLEPOLL para revisar la comunicacion y verificar que el validador retorne ACK
            * @return Si retorna -1 -> Error grave enviando el comando, el validador no retorno ACK
            * @return Si retorna  0 -> Todo corrio exitosamente, validador retorna ACK
            */
            int SimplePoll();

            /**
            * @brief Corre el comando CMDSELFCHECK para saber cual es el estado de falla del validador
            * @return Si retorna -2 -> Hay que repetir el comando, ya que no lo pudo ejecutar
            * @return Si retorna -1 -> Error grave enviando el comando
            * @return Si retorna  0 -> Todo corrio exitosamente, las 4 cosas tienen estado OK
            * @return Si retorna  1 -> Validador esta bloqueado
            * @return Si retorna  2 -> Error de hardware (interno del validador)
            * @return Si retorna  3 -> Error de software (interno del validador)
            */
            int SelfCheck();

            /**
            * @brief Corre el comando CMDENABLE para habilitar los canales del validador
            * @return Si retorna -1 -> Error grave enviando el comando, el validador no pudo habilitar los canales
            * @return Si retorna  0 -> Todo corrio exitosamente, validador habilito los canales
            */
            int EnableChannels();

            /**
            * @brief Corre el comando CMDSTARTPOLL para verificar que el evento se haya reseteado
            * @return Si retorna -2 -> Hay que repetir el comando, ya que no lo pudo ejecutar
            * @return Si retorna -1 -> Error grave enviando el comando
            * @return Si retorna  0 -> El evento esta borrado y listo
            * @return Si retorna  1 -> El evento no se reseteo
            */
            int CheckEventReset();

            /**
            * @brief Corre el comando CMDRESETDEVICE para reiniciar el dispositivo
            * @return Si retorna -1 -> Error grave enviando el comando, no se pudo reiniciar el dispositivo
            * @return Si retorna  0 -> Todo corrio exitosamente, validador reiniciado
            */
            int ResetDevice();

            /**
            * @brief Construye un comando personalizado para las monedas que se desean inhibir
            * @param InhibitMask1 es la mascara de inhibicion de los primeros 8 canales
            * @param InhibitMask2 es la mascara de inhibicion de los ultimos 8 canales
            * @return std::vector<unsigned char> Retorna el comando completo a escribir en el validador
            */
            std::vector<unsigned char> BuildCmdModifyInhibit(int InhibitMask1, int InhibitMask2);

            /**
            * @brief Corre el comando personalizado para inhibir las monedas seleccionadas
            * @param InhibitMask1 es la mascara de inhibicion de los primeros 8 canales
            * @param InhibitMask2 es la mascara de inhibicion de los ultimos 8 canales
            * @return Si retorna -1 -> Error grave enviando el comando, no se pudo inhibir los canales
            * @return Si retorna  0 -> Todo corrio exitosamente, validador pudo inhibir los canales
            */
            int ChangeInhibitChannels(int InhibitMask1, int InhibitMask2);
    };
}

#endif /* VALIDATORAzkoyen */