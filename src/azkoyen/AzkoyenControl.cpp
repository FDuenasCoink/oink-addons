/**
 * @file AzkoyenControl.cpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Archivo principal que expone las funciones del validador Azkoyen
 * @version 1.1
 * @date 2023-05-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "AzkoyenControl.hpp"

namespace AzkoyenControl {

    using namespace StateMachineAzkoyen;
    using namespace ValidatorAzkoyen;

    // --------------- EXTERNAL VARIABLES --------------------//

    // READ ONLY
    int PortO;   

    // WRITE ONLY
    int WarnToCritical;
    int MaxCritical;
    std::string Path;
    int LogLvl;
    int MaximumPorts;
    
    // --------------- INTERNAL VARIABLES --------------------//
    
    std::string VERSION = "1.1";
    std::string DEFAULTERROR = "DeafaultError";
    int Remaining;
    bool FlagCritical;
    bool FlagCritical2;
    int DeckCounter;
    int WarnCounter;
    int CriticalCounter;
    int CoinEventPrev;

    Response_t Response;
    
    AzkoyenControlClass::AzkoyenControlClass(){
        PortO = 0;
        CoinEventPrev = 0;
        DeckCounter = 0;
        WarnCounter = 0;
        CriticalCounter = 0;
        Remaining = 0;
        FlagCritical = false;
        FlagCritical2 = false;
        WarnToCritical = 10;
        MaxCritical = 4;
        Path = "logs/Azkoyen.log";
        LogLvl = 1;
        MaximumPorts = 10;
        Response.StatusCode = 404;
        Response.Message = DEFAULTERROR;
    }

    AzkoyenControlClass::~AzkoyenControlClass(){}

    void AzkoyenControlClass::InitLog(){
        Globals.AzkoyenObject.LoggerLevel = LogLvl;
        Globals.AzkoyenObject.InitLogger(Path);
        Globals.AzkoyenObject.MaxPorts = MaximumPorts;
    }

    Response_t AzkoyenControlClass::Connect() {
        
        int Connection = -1;
        int Check = -1;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        //std::cout<<"Iniciando maquina de estados"<<std::endl;
        //Cambio de estado: [Cualquiera] ---> ST_IDLE
        Globals.SMObject.InitStateMachine();
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        //Cambio de estado: ST_IDLE ---> ST_CONNECT
        Connection = Globals.SMObject.StateMachineRun(SMClass::EV_ANY);
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if (Connection == 0){
            
            PortO = Globals.AzkoyenObject.PortO;
            //Cambio de estado: ST_CONNECT ---> ST_CHECK
            Check = Globals.SMObject.StateMachineRun(SMClass::EV_SUCCESS_CONN);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

            Response = CheckCodes(Check);
        }
        else {
            //Cambio de estado: ST_CONNECT ---> ST_ERROR
            Globals.SMObject.StateMachineRun(SMClass::EV_ERROR);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
            Response.StatusCode = 505;
            Response.Message = "Fallo en la conexion con el validador, puerto no encontrado";
        }
        return Response;
    }

    Response_t AzkoyenControlClass::CheckDevice() {

        int Check = -1;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        Check = Globals.SMObject.RunCheck();
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        
        Response = CheckCodes(Check);

        return Response;
    }

    Response_t AzkoyenControlClass::StartReader() {
        
        int Enable = -1;
        int Poll = -1;
        int Reset = -1;
        int Check = -1;

        bool FlagReady = true;
        bool FlagInit = false;

        DeckCounter = 0;
        WarnCounter = 0;
        CriticalCounter = 0;

        CoinEventPrev = 0;
        Remaining = 0;

        FlagCritical = false;
        FlagCritical2 = false;

        //Deberia entrar aca desde el estado ST_CHECK
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_CHECK") == 0){
            FlagInit = true;
        }
        else if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_POLLING") == 0){
            //Se revisa que el evento este reiniciado para poder comenzar a hacer polling correctamente
            if (Globals.AzkoyenObject.CoinEvent <= 1){
                Response.StatusCode = 300;
                Response.Message = "Start reader corrio nuevamente. Listo para iniciar";
            }
            //Se debe cambiar de estado a reset cuando el evento no esta reiniciado y por ultimo se cambia al estado check
            else {
                //Cambio de estado: ST_POLLING ---> ST_RESET
                Reset = Globals.SMObject.StateMachineRun(SMClass::EV_FINISH_POLL);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                if (Reset == 0){
                    //Cambio de estado: ST_RESET ---> ST_CHECK
                    Check = Globals.SMObject.StateMachineRun(SMClass::EV_LOOP);
                    //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                    Response = CheckCodes(Check);
                    if ((Response.StatusCode == 200) | (Response.StatusCode == 300)){
                        FlagInit = true;
                    }
                }
                else {
                    FlagReady = false;
                }
            }
        }
        else {
            //Para cualquier otro estado diferente a ST_CHECK o ST_POLLING se vuelve a reiniciar la maquina de estados, llevandola hasta ST_CHECK
            //Cambio de estado: [Cualquiera] ---> ST_CHECK
            Response = Connect();
            if ((Response.StatusCode == 200) | (Response.StatusCode == 300)){
                FlagInit = true;
            }    
        }

        if (FlagReady){
            if (FlagInit){
                //Si llega hasta este punto, debe estar en el estado ST_CHECK
                //Cambio de estado: ST_CHECK ---> ST_WAIT_POLLING
                Enable = Globals.SMObject.StateMachineRun(SMClass::EV_CALL_POLLING);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                if (Enable == 0){
                    //Cambio de estado: ST_WAIT_POLLING ---> ST_POLLING
                    Poll = Globals.SMObject.StateMachineRun(SMClass::EV_READY);
                    //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

                    if (((Poll == 0) | (Poll == -2)) & (Globals.AzkoyenObject.CoinEvent <= 1)){
                        Response.StatusCode = 201;
                        Response.Message = "Validador OK. Listo para iniciar a leer monedas";
                    }
                    else if (Globals.AzkoyenObject.CoinEvent > 1){
                        Response.StatusCode = 506;
                        Response.Message = "Fallo con el validador. Validador no reinicio aunque se intento reiniciar";
                    }
                    else {
                        Response.StatusCode = 503;
                        Response.Message = "Fallo con el validador. No responde";
                    }
                }
                else {
                    Response.StatusCode = 503;
                    Response.Message = "Fallo con el validador. No responde";
                }
            }
        }
        else {
            Response = CheckCodes(1);
        }
        return Response;
    }

    CoinError_t AzkoyenControlClass::GetCoin() {
        
        CoinError_t ResponseCE;

        ResponseCE.StatusCode = 404;
        ResponseCE.Event = 0;
        ResponseCE.Coin = 0;
        ResponseCE.Message = DEFAULTERROR;
        ResponseCE.Remaining = 0;

        int Poll = -1;

        //Deberia entrar aca desde el estado ST_POLLING
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_POLLING") == 0){
            
            Poll = Globals.SMObject.StateMachineRun(SMClass::EV_POLL);
            
            if (Globals.AzkoyenObject.CoinEvent != CoinEventPrev){
                //std::cout<<"[MAIN] Evento actual: "<<Globals.AzkoyenObject.CoinEvent<<" Evento previo: "<<CoinEventPrev<<std::endl;
                Remaining = Globals.AzkoyenObject.CoinEvent - CoinEventPrev;

                if (Poll == 0){

                    ResponseCE.StatusCode = 202;
                    ResponseCE.Event = Globals.AzkoyenObject.CoinEvent;
                    ResponseCE.Coin = Globals.AzkoyenObject.ActOCoin;
                    ResponseCE.Message = "Moneda detectada";

                    DeckCounter = 0;
                    WarnCounter = 0;
                    CriticalCounter = 0;
                }
                else if (Poll == -2){

                    if (Globals.AzkoyenObject.ErrorOCode == 1){

                        ResponseCE.StatusCode = 302;
                        ResponseCE.Event = Globals.AzkoyenObject.CoinEvent;
                        ResponseCE.Coin = 0;
                        ResponseCE.Message = "Moneda rechazada";
                        WarnCounter++;
                    }
                    else{

                        if (Globals.AzkoyenObject.ErrorOCritical == 1){
                            CriticalCounter++;
                        }
                        if ((Globals.AzkoyenObject.ErrorOCode == 5) | (Globals.AzkoyenObject.ErrorOCode == 254) |
                           (Globals.AzkoyenObject.ErrorOCode == 6) | (Globals.AzkoyenObject.ErrorOCode == 9) | 
                           (Globals.AzkoyenObject.ErrorOCode == 10) | (Globals.AzkoyenObject.ErrorOCode == 20) | 
                           (Globals.AzkoyenObject.ErrorOCode == 119)){
                            WarnCounter++;
                        }

                        if (Globals.AzkoyenObject.ErrorOCode == 254){
                            DeckCounter++;
                        }

                        if (CriticalCounter >= MaxCritical){
                            FlagCritical = true;
                        }

                        if ((WarnCounter >= WarnToCritical) | (DeckCounter >= WarnToCritical)){ 
                            FlagCritical2 = true;
                        }

                        std::string EC = std::to_string(Globals.AzkoyenObject.ErrorOCode);
                        std::string WarnCounterStr = std::to_string(WarnCounter);
                        std::string CriticalCounterStr = std::to_string(CriticalCounter);
                        
                        ResponseCE.Event = Globals.AzkoyenObject.CoinEvent;
                        ResponseCE.Coin = 0;

                        if (FlagCritical){
                            ResponseCE.StatusCode = 402;
                            ResponseCE.Message = "Codigo: " + EC + " Mensaje: " + Globals.AzkoyenObject.ErrorOMsg + " CC: Full WC: " + WarnCounterStr;
                            FlagCritical2 = true;
                        }
                        else if(FlagCritical2){
                            ResponseCE.StatusCode = 403;
                            ResponseCE.Message = "Codigo: " + EC + " Mensaje: " + Globals.AzkoyenObject.ErrorOMsg + " CC: " + CriticalCounterStr + " WC: Full";
                        }
                        else{
                            ResponseCE.StatusCode = 401;
                            ResponseCE.Message = "Codigo: " + EC + " Mensaje: " + Globals.AzkoyenObject.ErrorOMsg + " CC: " + CriticalCounterStr + " WC: " + WarnCounterStr;
                        }
                    }
                }
                else{
                    ResponseCE.StatusCode = 503;
                    ResponseCE.Event = Globals.AzkoyenObject.CoinEvent;
                    ResponseCE.Coin = 0;
                    ResponseCE.Message = "Fallo con el validador. No responde";
                    FlagCritical = true;
                }

                if (Remaining > 1){
                    ResponseCE.Remaining = Remaining;                            
                }

                CoinEventPrev = (Globals.AzkoyenObject.CoinEvent == 255) ? 0 : Globals.AzkoyenObject.CoinEvent;
                
            }
            else{
                ResponseCE.StatusCode = 303;
                ResponseCE.Event = CoinEventPrev;
                ResponseCE.Coin = 0;
                ResponseCE.Message = "No hay nueva informacion";
            }
        }
        else{
            ResponseCE.StatusCode = 507;
            ResponseCE.Message = "No se ha iniciado el lector (StartReader)";
        }
        return ResponseCE;
    }

    CoinLost_t AzkoyenControlClass::GetLostCoins() {

        CoinLost_t ResponseLC;

        ResponseLC.CoinCinc = 0;
        ResponseLC.CoinCien = 0;
        ResponseLC.CoinDosc = 0;
        ResponseLC.CoinQuin = 0;
        ResponseLC.CoinMil = 0;

        if(strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_POLLING") == 0){
            ResponseLC.CoinCinc = Globals.AzkoyenObject.CoinCinc;
            ResponseLC.CoinCien = Globals.AzkoyenObject.CoinCien;
            ResponseLC.CoinDosc = Globals.AzkoyenObject.CoinDosc;
            ResponseLC.CoinQuin = Globals.AzkoyenObject.CoinQuin;
            ResponseLC.CoinMil = Globals.AzkoyenObject.CoinMil;
        }

        return ResponseLC;
    }
    
    Response_t AzkoyenControlClass::ModifyChannels(int InhibitMask1,int InhibitMask2) {

        int Inhibit = Globals.AzkoyenObject.ChangeInhibitChannels(InhibitMask1,InhibitMask2);

        if (Inhibit == 0){
            Response.StatusCode = 203;
            Response.Message = "Validador OK. Canales inhibidos correctamente";
        }
        else{
            Response.StatusCode = 508;
            Response.Message = "Fallo con el validador. No se pudieron inhibir los canales";
        }

        return Response;
    }

    Response_t AzkoyenControlClass::StopReader() {
              
        //Deberia entrar aca desde el estado ST_POLLING

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if(strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_POLLING") == 0){
            if ((FlagCritical) | (FlagCritical2)) {
                //Cambio de estado: ST_POLLING ---> ST_ERROR
                Globals.SMObject.StateMachineRun(SMClass::EV_ERROR);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                Response.StatusCode = 509;
                Response.Message = "Fallo en el deposito. Hubo un error critico";
            }
            else {
                //Cambio de estado: ST_POLLING ---> ST_RESET
                int Reset = Globals.SMObject.StateMachineRun(SMClass::EV_FINISH_POLL);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                if (Reset == 0){
                    //Cambio de estado: ST_RESET ---> ST_CHECK
                    int Check = Globals.SMObject.StateMachineRun(SMClass::EV_FINISH_POLL);
                    //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                    Response = CheckCodes(Check);
                }
                else{
                    //Cambio de estado: ST_RESET ---> ST_ERROR
                    Globals.SMObject.StateMachineRun(SMClass::EV_ERROR);
                    //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
                    Response.StatusCode = 506;
                    Response.Message = "Fallo con el validador. Validador no reinicio aunque se intento reiniciar";
                }
            }
        }
        else{
            Response.StatusCode = 405;
            Response.Message = "No se puede detener el lector porque no se ha iniciado";
        }

        return Response;
    }

    Response_t AzkoyenControlClass::ResetDevice() {

        int Reset = -1;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        Reset = Globals.SMObject.RunReset();
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        
        if (Reset == 0){
            Response.StatusCode = 204;
            Response.Message = "Validador OK. Reset corrio exitosamente";
        }
        else {
            Response.StatusCode = 506;
            Response.Message = "Fallo con el validador. Validador no reinicio aunque se intento reiniciar";
        }

        return Response;
    }

    TestStatus_t AzkoyenControlClass::TestStatus() {

        TestStatus_t Status;
        
        Status.Version = VERSION;
        Status.Device = 1;

        if (Globals.AzkoyenObject.FaultOCode != 0){
            Status.ErrorType = 0;
            Status.ErrorCode = Globals.AzkoyenObject.FaultOCode;
            Status.Message = Globals.AzkoyenObject.FaultOMsg;
            std::string EC = std::to_string(Globals.AzkoyenObject.ErrorOCode);
            Status.AditionalInfo = "ErrorCode: " + EC + " ECMensaje: " + Globals.AzkoyenObject.ErrorOMsg;
            Status.Priority = 1;
        }
        else{
            Status.ErrorType = 1;
            Status.ErrorCode = Globals.AzkoyenObject.ErrorOCode; //Revisar fallas de los opto estados. Aparecen aca?
            Status.Message = Globals.AzkoyenObject.ErrorOMsg;
            Status.AditionalInfo = "FaultCode: OK";
            Status.Priority = Globals.AzkoyenObject.ErrorOCritical;
        }

        return Status;
    }

    Response_t AzkoyenControlClass::CheckCodes(int Check){

        if(Check == 0){
            Response.StatusCode = 200;
            Response.Message = "Validador OK. Todos los sensores reportan buen estado";
        }
        else if(Check == 2){
            if (Globals.AzkoyenObject.NoUsedBit){
                Response.StatusCode = 200;
                Response.Message = "Validador OK. Todos los sensores reportan buen estado";
            }
            else if(Globals.AzkoyenObject.COSAlert){
                Response.StatusCode = 301;
                Response.Message = "Validador OK. Validador reporta alerta de moneda en cuerda";
            }
            else{
                Response.StatusCode = 504;
                Response.Message = "Fallo en el codigo del validador. Revisar codigo en C";
            }
        }
        else{
            if (Globals.AzkoyenObject.MeasurePhotoBlocked){
                Response.StatusCode = 501;
                Response.Message = "Fallo con el validador. Sensor de medida esta bloqueado";
            }
            else if (Globals.AzkoyenObject.OutPhotoBlocked){
                Response.StatusCode = 502;
                Response.Message = "Fallo con el validador. Sensor de salida esta bloqueado";
            }
            else{
                Response.StatusCode = 503;
                Response.Message = "Fallo con el validador. No responde";
            }
        }
        return Response;
    }
}

