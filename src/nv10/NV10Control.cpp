/**
 * @file NV10Control.cpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Archivo principal que expone las funciones del validador NV10
 * @version 1.1
 * @date 2023-05-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "NV10Control.hpp"

namespace NV10Control{

    using namespace NV10StateMachine;
    using namespace ValidatorNV10;

    // --------------- EXTERNAL VARIABLES --------------------//

    // READ ONLY
    int PortO;

    // WRITE ONLY
    std::string Path;
    int LogLvl;
    int MaximumPorts;
    
    // --------------- INTERNAL VARIABLES --------------------//
    
    std::string VERSION = "1.1";
    std::string DEFAULTERROR = "DeafaultError";
    bool FlagReading;
    int Inhibit;
    
    Response_t Response;
    BillError_t ResponseBEdef;
    BillError_t LastResponseBE;
    
    NV10ControlClass::NV10ControlClass(){
        
        PortO = 0;

        Path = "logs/NV10.log";
        LogLvl = 1;             
        MaximumPorts = 10;

        FlagReading = false;
        Inhibit = 255;
        Response.StatusCode = 404;
        Response.Message = DEFAULTERROR;

        LastResponseBE.StatusCode = 404;
        LastResponseBE.Bill = 0;
        LastResponseBE.Message = DEFAULTERROR;

        ResponseBEdef.StatusCode = 302;
        ResponseBEdef.Bill = 0;
        ResponseBEdef.Message = "Billetero OK. No hay nueva informacion";
    }

    NV10ControlClass::~NV10ControlClass(){}

    void NV10ControlClass::InitLog(){
        Globals.NV10Object.LoggerLevel = LogLvl;
        Globals.NV10Object.InitLogger(Path);
        Globals.NV10Object.MaxPorts = MaximumPorts;
    }

    Response_t NV10ControlClass::Connect() {
        
        int Connection = -1;
        int Disable = -1;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        //std::cout<<"Iniciando maquina de estados"<<std::endl;
        //Cambio de estado: [Cualquiera] ---> ST_IDLE
        Globals.SMObject.InitStateMachine();
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        //Cambio de estado: ST_IDLE ---> ST_CONNECT
        Connection = Globals.SMObject.StateMachineRun(NV10SMClass::EV_ANY);
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if (Connection == 0){
            
            PortO = Globals.NV10Object.PortO;
            //Cambio de estado: ST_CONNECT ---> ST_DISABLE
            Disable = Globals.SMObject.StateMachineRun(NV10SMClass::EV_SUCCESS_CONN);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

            if (Disable == 0){
                Response.StatusCode = 200;
                Response.Message = "Billetero OK. Se sincronizo exitosamente";
            }
            else {
                Response.StatusCode = 501;
                Response.Message = "Fallo con el billetero. No responde";
            }
        }
        else {
            Response.StatusCode = 502;
            Response.Message = "Fallo en la conexion con el billetero, puerto no encontrado";
        }
        
        if (Response.StatusCode != 200){
            //Cambio de estado: [Cualquiera] ---> ST_ERROR
            Globals.SMObject.StateMachineRun(NV10SMClass::EV_ERROR);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        }

        return Response;
    }

    Response_t NV10ControlClass::CheckDevice() {

        int Check = -1;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        Check = Globals.SMObject.RunCheck();
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        
        if (Check == 0){
            Response.StatusCode = 201;
            Response.Message = "Billetero OK. Se reviso exitosamente";
        }
        else {
            Response.StatusCode = 501;
            Response.Message = "Fallo con el billetero. No responde";
        }

        if (Response.StatusCode != 201){
            //Cambio de estado: [Cualquiera] ---> ST_ERROR
            Globals.SMObject.StateMachineRun(NV10SMClass::EV_ERROR);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        }

        return Response;
    }

    Response_t NV10ControlClass::StartReader() {
        
        int Enable = -1;
        int Poll = -1;

        bool FlagReady = false;
        bool FlagInState = false;
        bool FlagFinish = false;

        //Deberia entrar aca desde el estado ST_DISABLE
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_DISABLE") == 0){
            FlagReady = true;
        }
        else if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_POLLING") == 0){
            FlagInState = true;
        }
        else {
            //Para cualquier otro estado diferente a ST_DISABLE o ST_POLLING se vuelve a reiniciar la maquina de estados, llevandola hasta ST_DISABLE
            //Cambio de estado: [Cualquiera] ---> ST_DISABLE
            Response = Connect();
            if (Response.StatusCode == 200){
                FlagReady = true;
            }    
        }

        if (FlagReady){
            //Si llega hasta este punto, debe estar en el estado ST_DISABLE
            //Cambio de estado: ST_DISABLE ---> ST_ENABLE
            Enable = Globals.SMObject.StateMachineRun(NV10SMClass::EV_READY);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
            if (Enable == 0){
                //Cambio de estado: ST_ENABLE ---> ST_POLLING
                Poll = Globals.SMObject.StateMachineRun(NV10SMClass::EV_CALL_POLLING);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                if (Poll == 0){
                    Response.StatusCode = 202;
                    Response.Message = "Billetero OK. Listo para iniciar a leer billetes";
                    FlagFinish = true;
                }
            }
        }

        if (FlagFinish == false){
            if (FlagInState){
                Response.StatusCode = 203;
                Response.Message = "Billetero OK. Start reader corrio nuevamente. Listo para iniciar a leer billetes";
            }
            else {
                Response.StatusCode = 501;
                Response.Message = "Fallo con el billetero. No responde";
            }
        }

        if ((Response.StatusCode != 202) & (Response.StatusCode != 203)){
            //Cambio de estado: [Cualquiera] ---> ST_ERROR
            Globals.SMObject.StateMachineRun(NV10SMClass::EV_ERROR);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        }
        return Response;
    }

    BillError_t NV10ControlClass::GetBill() {

        BillError_t ResponseBE;

        ResponseBE.StatusCode = 404;
        ResponseBE.Bill = 0;
        ResponseBE.Message = DEFAULTERROR;
        
        int Poll = -1;
        int Length = 0;
        int ErrorC = 0;
        int EventC = 0;
        int AdEventC = 0;
        int Bill = 0;
        int BillC = 0;

        //Deberia entrar aca desde el estado ST_POLLING
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_POLLING") == 0){

            //Cambio de estado: ST_POLLING ---> ST_POLLING
            Poll = Globals.SMObject.StateMachineRun(NV10SMClass::EV_POLL);

            //Si Poll retorna una lectura valida entra aca
            if (Poll == 0){
                Length = Globals.NV10Object.LengthData;
                ErrorC = Globals.NV10Object.ErrorOCode;
                //Si la longitud es 1 significa que solo hay que mirar el codigo de error
                if ((Length == 1) & (ErrorC == 240)){
                    ResponseBE.StatusCode = 302;
                    ResponseBE.Message = "Billetero OK. No hay nueva informacion";
                }
                //Si el codigo de error es diferente de OK-240 no es necesario mirar datos adicionales
                else if (ErrorC != 240){
                    //Si el codigo de error es diferente de CMD_NO_PROCESSED-245 es porque hay un error de software
                    if (ErrorC != 245){
                        std::string ErrorM = Globals.NV10Object.ErrorOMsg;
                        ResponseBE.StatusCode = 504;
                        ResponseBE.Message = "Falla en el comando. Comando retorna: " + ErrorM;
                    }
                    //Si el codigo de error es CMD_NO_PROCESSED-245 significa que no se puede hacer una accion sobre un billete que no existe
                    else {
                        ResponseBE.StatusCode = 505;
                        ResponseBE.Message = "No hay billete. Comando no puede ser procesado";
                    }
                }
                //Si la longitud es igual o mayor a 2 se puede comenzar a revisar los eventos
                else if (Length >= 2){
                    EventC = Globals.NV10Object.EventOCode;
                    //Si el codigo de evento es READ-239 significa que detecto un billete en la entrada
                    if (EventC == 239){
                        Bill = Globals.NV10Object.Bill;
                        ResponseBE.Bill = Bill;
                        // Si el billete es 0 esta en el estado inicial de READ
                        if (Bill == 0){
                            // El estado normal de FlagReading en este punto es false, si es true es porque algo salio mal antes
                            if (FlagReading == false){
                                ResponseBE.StatusCode = 303;
                                ResponseBE.Message = "Leyendo billete. Se desconoce su valor";
                            }
                            else {
                                ResponseBE.StatusCode = 507;
                                ResponseBE.Message = "Error en secuencia del billetero. El anterior billete se pudo perder";
                            }
                        }
                        else {
                            BillC = Globals.NV10Object.Channel;
                            if ((BillC >= 1) & (BillC <= 7)){
                                std::bitset<8> Bits(Inhibit);
                                if (Bits[BillC-1] == 0){
                                    Response = Reject();
                                    if (Response.StatusCode == 206){
                                        ResponseBE.StatusCode = 311;
                                        ResponseBE.Message = "Billete inhibido. Esperando a que el usuario retire el billete";
                                        FlagReading = false;
                                    }
                                    else {
                                        ResponseBE.StatusCode = 501;
                                        ResponseBE.Message = "Fallo con el billetero. No responde";
                                        FlagReading = false;
                                    }
                                }
                                else {
                                    ResponseBE.StatusCode = 304;
                                    ResponseBE.Message = "Leyendo billete. Billete detectado exitosamente";
                                    FlagReading = true;
                                }
                            }
                            else {
                                ResponseBE.StatusCode = 511;
                                ResponseBE.Message = "Fallo con el codigo. Canal de billete desconocido";
                                FlagReading = false;
                            }
                        }
                    }
                    //Si el codigo de evento es REJECTING-237 significa que rechazo el billete que detecto pero no lo han retirado
                    else if (EventC == 237){
                        ResponseBE.StatusCode = 305;
                        ResponseBE.Message = "Billete rechazado. Esperando a que el usuario retire el billete";
                        FlagReading = false;
                    }
                    //Si el codigo de evento es REJECTED-236 significa que el cliente retiro el billete que se rechazo
                    else if (EventC == 236){
                        ResponseBE.StatusCode = 306;
                        ResponseBE.Message = "Billete rechazado. Usuario retiro el billete";
                        FlagReading = false;

                    }
                    //Si el codigo de evento es STACKING-204 significa que esta apilando el billete que ya leyo
                    else if (EventC == 204){
                        Bill = Globals.NV10Object.Bill;
                        ResponseBE.Bill = Bill;
                        FlagReading = true;
                        ResponseBE.StatusCode = 307;
                        ResponseBE.Message = "Billete leido. Apilando billete";                    
                    }
                    //Si el codigo de evento es STACKED-235 significa que apilo el billete leido
                    else if (EventC == 235){
                        if (FlagReading){
                            if (LastResponseBE.StatusCode != 312){
                                Bill = Globals.NV10Object.Bill;
                                ResponseBE.Bill = Bill;
                                ResponseBE.StatusCode = 308;
                                ResponseBE.Message = "Billete apilado";
                                FlagReading = false;
                            }
                            else{
                                ResponseBE.StatusCode = 302;
                                ResponseBE.Message = "Billetero OK. No hay nueva informacion";
                            }
                        }
                        else {
                            ResponseBE.StatusCode = 302;
                            ResponseBE.Message = "Billetero OK. No hay nueva informacion";
                        }
                    }
                    // Si el codigo de evento es CREDIT-238 significa que reconocio el billete y probablemente ya esta apilado
                    else if (EventC == 238){
                        Bill = Globals.NV10Object.Bill;
                        ResponseBE.Bill = Bill;
                        //Si la longitud es 3 no hay evento de codigo adicional
                        if (Length == 3){
                            ResponseBE.StatusCode = 309;
                            ResponseBE.Message = "Billete acreditado, listo para apilar";
                        }
                        //Si la longitud es 4, hay evento de codigo adicional
                        else if (Length == 4){
                            AdEventC = Globals.NV10Object.AdEventOCode;
                            // Si el codigo de evento adicional es STACKED-235 significa que apilo el billete leido
                            if ((AdEventC == 204) | (AdEventC == 235)){
                                if (FlagReading){
                                    ResponseBE.StatusCode = 312;
                                    ResponseBE.Message = "Billete acreditado y apilado";
                                    FlagReading = false;
                                }
                                else {
                                    ResponseBE.Bill = 0;
                                    ResponseBE.StatusCode = 302;
                                    ResponseBE.Message = "Billetero OK. No hay nueva informacion";
                                }
                            }
                            // Si el evento adicional no es ni STACKING ni STACKED, hubo un error grave con la respuesta
                            else {
                                ResponseBE.StatusCode = 508;
                                ResponseBE.Message = "Billete acreditado, pero con error: " + Globals.NV10Object.AdEventOMsg;
                            }
                        }
                        // Para un evento CREDIT la longitud no puede ser diferente de 3 o de 4
                        else {
                            ResponseBE.StatusCode = 508;
                            ResponseBE.Message = "Billete acreditado, pero con error: " + Globals.NV10Object.AdEventOMsg;
                        }
                        FlagReading = false;
                    }
                    // Si el codigo de evento es STACKED-235 significa que apilo el billete que anteriormente leyo y acredito
                    else if (EventC == 235){
                        Bill = Globals.NV10Object.Bill;
                        ResponseBE.Bill = Bill;
                        if ((FlagReading) & (Bill > 0)){
                            ResponseBE.StatusCode = 308;
                            ResponseBE.Message = "Billete apilado";   
                            FlagReading = false; 
                        }
                        else if (FlagReading == false){
                            ResponseBE.StatusCode = 310;
                            ResponseBE.Message = "Billetero OK. Billete apilado. No hay nueva informacion"; 
                        }
                        else {
                            ResponseBE.StatusCode = 509;
                            ResponseBE.Message = "Billete apilado pero no se sabe su valor";
                            FlagReading = false;
                        }
                    }
                    // Otro codigo de evento significa que hubo un error grave
                    else {
                        if(Length == 3){
                            Bill = Globals.NV10Object.Bill;
                            ResponseBE.Bill = Bill;
                        }
                        else {
                            ResponseBE.Bill = 0;
                        }
                        std::string BillStr = std::to_string(ResponseBE.Bill);
                        ResponseBE.StatusCode = 510;
                        ResponseBE.Message = "Error grave en el Billetero: " + Globals.NV10Object.EventOMsg; + "con  el billete: " + BillStr;
                    }
                }
                // Aca entra unicamente cuando la longitud es 0
                else {
                    ResponseBE.StatusCode = 505;
                    ResponseBE.Message = "Fallo en la respuesta. Longitud invalida";
                }
            }
            //Si el billetero envia un comando repetido [Poll = 2], quiere decir que no hay que volver a leer la respuesta
            else if (Poll == 2){
                ResponseBE.StatusCode = 301;
                ResponseBE.Message = "Billetero OK. Comando repetido, la respuesta ya fue vista anteriormente";
            }
            //Si Poll da un error de lectura, significa que el billetero no responde
            else {
                ResponseBE.StatusCode = 501;
                ResponseBE.Message = "Fallo con el billetero. No responde";
            }
        }
        else {
            ResponseBE.StatusCode = 503;
            ResponseBE.Message = "No se ha iniciado el lector (StartReader)";
        }

        if (LastResponseBE.StatusCode == ResponseBE.StatusCode){
            return ResponseBEdef;
        }
        else{
            if (ResponseBE.StatusCode == 303){
                std::cout<<"-----------------------------------------------------------------------------------------------------"<<std::endl;
            }
            LastResponseBE = ResponseBE;
            return ResponseBE;
        }   
    }
    
    Response_t NV10ControlClass::ModifyChannels(int InhibitMask1) {

        Inhibit = InhibitMask1;

        Response.StatusCode = 204;
        Response.Message = "Billetero OK. Canales inhibidos correctamente";

        return Response;
    }

    Response_t NV10ControlClass::StopReader() {
              
        //Deberia entrar aca desde el estado ST_POLLING

        int Check = -1;
        int Disable = -1;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_POLLING") == 0){
            //Cambio de estado: ST_POLLING ---> ST_CHECK
            Check = Globals.SMObject.StateMachineRun(NV10SMClass::EV_FINISH_POLL);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
            if (Check == 0){
                //Cambio de estado: ST_CHECK ---> ST_DISABLE
                Disable = Globals.SMObject.StateMachineRun(NV10SMClass::EV_LOOP);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                if (Disable == 0){
                    Response.StatusCode = 205;
                    Response.Message = "Billetero OK. StopReader corrio exitosamente";
                }
                else {
                    Response.StatusCode = 511;
                    Response.Message = "Fallo con el billetero. No se pudo desactivar";
                }
            }
            else {
                Response.StatusCode = 512;
                Response.Message = "Fallo con el billetero. No se pudo revisar";
            }
        }
        else {
            Response.StatusCode = 513;
            Response.Message = "No se puede detener el lector porque no se ha iniciado";
        }

        if (Response.StatusCode != 205) {
            //Cambio de estado: [Cualquiera] ---> ST_ERROR
            Globals.SMObject.StateMachineRun(NV10SMClass::EV_ERROR);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        }

        return Response;
    }

    Response_t NV10ControlClass::Reject() {

        int Reject = Globals.NV10Object.Reject();

        if (Reject == 0){
            Response.StatusCode = 206;
            Response.Message = "Billetero OK. Reject corrio exitosamente";
        }
        else {
            Response.StatusCode = 501;
            Response.Message = "Fallo con el billetero. No responde";
        }

        return Response;
    }

    TestStatus_t NV10ControlClass::TestStatus() {

        TestStatus_t Status;
        
        Status.Version = VERSION;
        Status.Device = 1;

        if (Globals.NV10Object.ErrorOCode != 240){
            Status.ErrorType = 0;
            Status.ErrorCode = Globals.NV10Object.ErrorOCode;
            Status.Message = Globals.NV10Object.ErrorOMsg;
            std::string EvC = std::to_string(Globals.NV10Object.EventOCode);
            Status.AditionalInfo = "LastEventCode: " + EvC + " LastEventMessage: " + Globals.NV10Object.EventOMsg;
            Status.Priority = 1;
        }
        else {
            Status.ErrorType = 1;
            Status.ErrorCode = Globals.NV10Object.EventOCode;
            Status.Message = Globals.NV10Object.EventOMsg;
            std::string LRC = std::to_string(Globals.NV10Object.LROCode);
            Status.AditionalInfo = "LastRejectCode: " + LRC + " LastRejectMessage: " + Globals.NV10Object.LROMsg;

            if ((Globals.NV10Object.EventOPriority == 1) | (Globals.NV10Object.LROPriority == 1)){
                Status.Priority = 1;
            }
            else {
                Status.Priority = 0;
            }
        }
        return Status;
    }
}

