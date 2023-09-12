/**
 * @file DispenserControl.cpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Archivo principal que expone las funciones del dispensador
 * @version 1.1
 * @date 2013-05-31
 * 
 * @copyright Copyright (c) 2013
 * 
 */

#include "DispenserControl.hpp"

namespace DispenserControl{

    using namespace DispenserStateMachine;
    using namespace Dispenser;

    // --------------- EXTERNAL VARIABLES --------------------//

    // READ ONLY
    int PortO;

    // WRITE ONLY
    std::string Path;
    int LogLvl;
    int MaximumPorts;
    int MaxInitAttempts;
    int ShortTime;
    int LongTime;
    
    // --------------- INTERNAL VARIABLES --------------------//
    
    std::string VERSION = "1.1";
    std::string DEFAULTERROR = "DefaultError";
    
    Response_t Response;

    bool FlagCanRecycle = false;
    
    DispenserControlClass::DispenserControlClass(){
        
        PortO = 0;

        Path = "logs/Dispenser.log";
        LogLvl = 1;             
        MaximumPorts = 10;
        MaxInitAttempts = 4;
        ShortTime = 0;
        LongTime = 3;

        Response.StatusCode = 404;
        Response.Message = DEFAULTERROR;
    }

    DispenserControlClass::~DispenserControlClass(){}

    void DispenserControlClass::InitLog(){
        Globals.DispenserObject.LoggerLevel = LogLvl;
        Globals.DispenserObject.InitLogger(Path);
        Globals.DispenserObject.MaxPorts = MaximumPorts;
        Globals.DispenserObject.MaxInitAttempts = MaxInitAttempts;
        Globals.DispenserObject.ShortTime = ShortTime;
        Globals.DispenserObject.LongTime = LongTime;
    }

    Response_t DispenserControlClass::Connect() {
        
        int Connection = -1;
        int Init = -1;
        int Check = -1;

        Response.StatusCode = 404;
        Response.Message = DEFAULTERROR;

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        //std::cout<<"Iniciando maquina de estados"<<std::endl;
        //Cambio de estado: [Cualquiera] ---> ST_IDLE
        Globals.SMObject.InitStateMachine();

        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        //Cambio de estado: ST_IDLE ---> ST_CONNECT
        Connection = Globals.SMObject.StateMachineRun(DispenserSMClass::EV_ANY);
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if (Connection == 0){
            
            PortO = Globals.DispenserObject.PortO;

            //Cambio de estado: ST_CONNECT ---> ST_INIT
            Init = Globals.SMObject.StateMachineRun(DispenserSMClass::EV_SUCCESS_CONN);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

            if (Init == 0){

                //Cambio de estado: ST_INIT ---> ST_WAIT
                Check = Globals.SMObject.StateMachineRun(DispenserSMClass::EV_SUCCESS_INIT);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

                if (Check == 0){
                    Response = CheckCodes();
                }
                else if (Check == 2){
                    Response.StatusCode = 500;
                    Response.Message = "Fallo con el dispensador. Codigo de fallo: "+ Globals.DispenserObject.ErrorOCode + " - Mensaje de fallo: " + Globals.DispenserObject.ErrorOMsg;
                }
                else {
                    Response.StatusCode = 501;
                    Response.Message = "Fallo con el dispensador. Conectó, inicializó, pero no se pudo revisar";
                }
            }
            else {
                Response.StatusCode = 502;
                Response.Message = "Fallo con el dispensador. Conectó, pero no se pudo inicializar";
            }
        }
        else {
            Response.StatusCode = 503;
            Response.Message = "Fallo en la conexion con el dispensador, puerto no encontrado";
        }
        
        if ((Response.StatusCode == 404) | (Response.StatusCode == 501) | (Response.StatusCode == 502) | (Response.StatusCode == 503)){
            //Cambio de estado: [Cualquiera] ---> ST_ERROR
            Globals.SMObject.StateMachineRun(DispenserSMClass::EV_ERROR);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        }

        return Response;
    }

    Response_t DispenserControlClass::CheckDevice() {

        int Check = -1;

        Response.StatusCode = 404;
        Response.Message = DEFAULTERROR;

        Check = Globals.SMObject.RunCheck();
        
        if (Check == 0){
            Response = CheckCodes();
        }
        else if (Check == 2){
            Response.StatusCode = 500;
            Response.Message = "Fallo con el dispensador. Codigo de fallo: "+ Globals.DispenserObject.ErrorOCode + " - Mensaje de fallo: " + Globals.DispenserObject.ErrorOMsg;
        }
        else {
            Response.StatusCode = 507;
            Response.Message = "Fallo con el dispensador. No responde";
        }

        if ((Response.StatusCode == 404) | (Response.StatusCode == 500) | (Response.StatusCode == 507)){
            //Cambio de estado: [Cualquiera] ---> ST_ERROR
            Globals.SMObject.StateMachineRun(DispenserSMClass::EV_ERROR);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        }

        return Response;
    }

    Response_t DispenserControlClass::CheckCodes(){

        if (Globals.DispenserObject.RFICCardInGate){
            Response.StatusCode = 504;
            Response.Message = "Dispensador atascado. Se reviso exitosamente pero se detecto una tarjeta atascada";
        }
        else if (Globals.DispenserObject.CardInGate){
            if (Globals.DispenserObject.RecyclingBoxFull == false){
                FlagCanRecycle = true;
            }
            Response.StatusCode = 301;
            Response.Message = "Dispensador con tarjeta en puerta. Esperar para entregar al usuario o reciclar";
        }
        else {
            if (Globals.DispenserObject.RecyclingBoxFull){
                FlagCanRecycle = false;
                if (Globals.DispenserObject.DispenserFull){
                    Response.StatusCode = 302;
                    Response.Message = "Dispensador con caja de reciclaje llena. Lleno de tarjetas disponibles";
                }
                else if (Globals.DispenserObject.CardsInDispenser){
                    Response.StatusCode = 303;
                    Response.Message = "Dispensador con caja de reciclaje llena. Con algunas tarjetas disponibles";
                }
                else {
                    Response.StatusCode = 505;
                    Response.Message = "Dispensador con caja de reciclaje llena. No hay tarjetas disponibles";
                }
            }
            else {
                FlagCanRecycle = true;
                if (Globals.DispenserObject.DispenserFull){
                    Response.StatusCode = 201;
                    Response.Message = "Dispensador OK. Lleno de tarjetas disponibles";
                }
                else if (Globals.DispenserObject.CardsInDispenser){
                    Response.StatusCode = 202;
                    Response.Message = "Dispensador OK. Con algunas tarjetas disponibles";
                }
                else {
                    Response.StatusCode = 506;
                    Response.Message = "Dispensador sin tarjetas. Caja de reciclaje aun no está llena";
                }
            }
        }

        return Response;
    }

    Response_t DispenserControlClass::DispenseCard(){

        bool FlagReady = false;
        bool FlagSaveCard = false;

        int Dispense = -1;

        Response.StatusCode = 404;
        Response.Message = DEFAULTERROR;

        //Deberia entrar aca desde el estado ST_WAIT
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_WAIT") == 0){
            FlagReady = true;
        }
        else {
            Response = Connect();
            if ((Response.StatusCode == 201) | (Response.StatusCode == 202) | (Response.StatusCode == 302) | (Response.StatusCode == 303)){
                FlagReady = true;
            }
        }

        if (FlagReady){
            
            Response = CheckDevice();
            
            if ((Response.StatusCode == 201) | (Response.StatusCode == 202) | (Response.StatusCode == 302) | (Response.StatusCode == 303)){

                //Cambio de estado: ST_WAIT ---> ST_MOVING_MOTOR
                Dispense = Globals.SMObject.StateMachineRun(DispenserSMClass::EV_CALL_DISPENSING);
                //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

                if (Dispense == 0){

                    FlagSaveCard = true;

                    if (Globals.DispenserObject.RFICCardInGate){
                        //std::cout<<"[MAIN] Hay una tarjeta atorada en el dispensador"<<std::endl;
                        FlagSaveCard = false;
                        Response.StatusCode = 508;
                        Response.Message = "Dispensador atascado. Se dispensó, pero la tarjeta quedo atascada";
                    }
                    else if (Globals.DispenserObject.CardInGate){
                        //std::cout<<"[MAIN] Hay una tarjeta en la puerta"<<std::endl;
                        Response.StatusCode = 203;
                        Response.Message = "Dispensador movio la tarjeta. Se dispensó y se detecto la tarjeta en puerta";
                    }
                    else {
                        Response.StatusCode = 304;
                        Response.Message = "Dispensador movio la tarjeta. Se dispensó pero ya se retiro la tarjeta";
                    }
                }
                else if (Dispense == 2){
                    Response.StatusCode = 500;
                    Response.Message = "Fallo con el dispensador. Codigo de fallo: "+ Globals.DispenserObject.ErrorOCode + " - Mensaje de fallo: " + Globals.DispenserObject.ErrorOMsg;
                }
                else if (Dispense == 3){
                    Response.StatusCode = 510;
                    Response.Message = "Dispensador sin tarjetas. No se puede dispensar la tarjeta";
                }
                else {
                    for (int i = 1; i <= MaxInitAttempts ; i++){

                        Response = CheckDevice();

                        if ((Response.StatusCode != 404) & (Response.StatusCode != 500) & (Response.StatusCode != 507)){
                            
                            if (Globals.DispenserObject.CardInGate){
                                FlagSaveCard = true;
                                Response.StatusCode = 305;
                                Response.Message = "Dispensador movio la tarjeta. Hubieron errores de comunicación pero se detecto la tarjeta en puerta";
                            }
                            else if (Globals.DispenserObject.RFICCardInGate){
                                Response.StatusCode = 508;
                                Response.Message = "Dispensador atascado. Se dispensó, pero la tarjeta quedo atascada";
                            }
                            else {
                                Response.StatusCode = 509;
                                Response.Message = "Fallo en el dispensador. No se pudo conocer el estado de la tarjeta";
                            }

                            i = MaxInitAttempts + 1;
                        }
                        else {
                            //std::cout<<"[MAIN] No se puede obtener informacion de la tarjeta en puerta"<<std::endl;
                            Response.StatusCode = 509;
                            Response.Message = "Fallo con el dispensador. No se pudo conocer el estado de la tarjeta";
                        }
                    }
                }
            }
            else if ((Response.StatusCode == 505) | (Response.StatusCode == 506)){
                Response.StatusCode = 510;
                Response.Message = "Dispensador sin tarjetas. No se puede dispensar la tarjeta";
            }
        }

        if (Response.StatusCode == 404){
            Response.StatusCode = 507;
            Response.Message = "Fallo con el dispensador. No responde";
        }
                
        if ((Response.StatusCode == 500) | (Response.StatusCode == 504) | (Response.StatusCode == 507) |
            (Response.StatusCode == 508) | (Response.StatusCode == 509)){
            //Cambio de estado: [Cualquiera] ---> ST_ERROR
            Globals.SMObject.StateMachineRun(DispenserSMClass::EV_ERROR);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        }

        return Response;
    }

    Response_t DispenserControlClass::RecycleCard(){

        bool FlagReady = false;
        int Recycle = -1;

        Response.StatusCode = 404;
        Response.Message = DEFAULTERROR;

        //Deberia entrar aca desde el estado ST_MOVING_MOTOR
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_MOVING_MOTOR") == 0){
            FlagReady = true;
        }
        else {
            Response = Connect();
            if ((Response.StatusCode == 201) | (Response.StatusCode == 202) | (Response.StatusCode == 301) | (Response.StatusCode == 506)){
                FlagReady = true;
            }
        }

        if (FlagReady){

            Response = CheckDevice();

            if ((Response.StatusCode != 404) & (Response.StatusCode != 500) & (Response.StatusCode != 507)){
                if (Globals.DispenserObject.RFICCardInGate){
                    Response.StatusCode = 504;
                    Response.Message = "Dispensador atascado. Se reviso exitosamente pero se detecto una tarjeta atascada";
                }
                else if ((Globals.DispenserObject.CardInGate) & (Globals.DispenserObject.RecyclingBoxFull == false)){

                    //Cambio de estado: ST_WAIT/ST_MOVING_MOTOR ---> ST_HANDING_CARD
                    Recycle = Globals.SMObject.StateMachineRun(DispenserSMClass::EV_CARD_IN_GATE);
                    //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

                    if (Recycle == 0){
                        if (Globals.DispenserObject.RFICCardInGate){
                            Response.StatusCode = 511;
                            Response.Message = "Dispensador atascado. Se intento reciclar, pero la tarjeta quedo atascada";
                        }
                        else if (Globals.DispenserObject.CardInGate){
                            Response.StatusCode = 512;
                            Response.Message = "Dispensador con tarjeta en puerta. Se intento reciclar, pero la tarjeta sigue en la puerta";
                        }
                        else {
                            Response.StatusCode = 204;
                            Response.Message = "Dispensador recicló la tarjeta. Se movió exitosamente a la caja de reciclaje";
                        }
                    }
                    else if (Recycle == 3){
                        Response.StatusCode = 513;
                        Response.Message = "No se detecto tarjeta en puerta y por ello no se intenta reciclar la tarjeta";
                    }
                    else if (Recycle == 2){
                        Response.StatusCode = 515;
                        Response.Message = "Hubo un error reciclando la tarjeta. Codigo de fallo: "+ Globals.DispenserObject.ErrorOCode + " - Mensaje de fallo: " + Globals.DispenserObject.ErrorOMsg;
                    }
                    else {
                        for (int i = 1; i <= MaxInitAttempts ; i++){

                            Response = CheckDevice();

                            if ((Response.StatusCode != 404) & (Response.StatusCode != 500) & (Response.StatusCode != 507)){
                            
                                if (Globals.DispenserObject.CardInGate){
                                    Response.StatusCode = 512;
                                    Response.Message = "Dispensador con tarjeta en puerta. Se intento reciclar, pero la tarjeta sigue en la puerta";
                                }
                                else if (Globals.DispenserObject.RFICCardInGate){
                                    Response.StatusCode = 511;
                                    Response.Message = "Dispensador atascado. Se intento reciclar, pero la tarjeta quedo atascada";
                                }
                                else {
                                    Response.StatusCode = 516;
                                    Response.Message = "Fallo con el dispensador. Se intenta reciclar pero no se pudo conocer el estado de la tarjeta";
                                }

                                i = MaxInitAttempts + 1;
                            }
                            else {
                                //std::cout<<"[MAIN] No se puede obtener informacion de la tarjeta en puerta"<<std::endl;
                                Response.StatusCode = 516;
                                Response.Message = "Fallo con el dispensador. Se intenta reciclar pero no se pudo conocer el estado de la tarjeta";
                            }
                        }
                    }
                }
                else {
                    if (Globals.DispenserObject.RecyclingBoxFull){
                        Response.StatusCode = 514;
                        Response.Message = "Dispensador con caja de reciclaje llena. No se puede reciclar la tarjeta";
                    }
                    else {
                        Response.StatusCode = 513;
                        Response.Message = "No se detecto tarjeta en puerta y por ello no se intenta reciclar la tarjeta";
                    }
                }
            }
        }
        
        if ((Response.StatusCode == 404) | (Response.StatusCode == 501) | (Response.StatusCode == 502) |
            (Response.StatusCode == 503) | (Response.StatusCode == 507)){

            Response.StatusCode = 507;
            Response.Message = "Fallo con el dispensador. No responde";

            Globals.SMObject.StateMachineRun(DispenserSMClass::EV_ERROR);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;
        }

        if ((Response.StatusCode == 302) | (Response.StatusCode == 303) | (Response.StatusCode == 505)){
            Response.StatusCode = 514;
            Response.Message = "Dispensador con caja de reciclaje llena. No se puede reciclar la tarjeta";
        }


        return Response;
    }

    Response_t DispenserControlClass::EndProcess(){

        int Check = -1;
        bool FlagReady = false;

        Response.StatusCode = 404;
        Response.Message = DEFAULTERROR;

        //Deberia entrar aca desde el estado ST_MOVING_MOTOR o ST_HANDING_CARD
        //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

        if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_MOVING_MOTOR") == 0){
            FlagReady = true;
        }
        else if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_HANDING_CARD") == 0){
            FlagReady = true;
        }

        if (FlagReady){

            //Cambio de estado: ST_HANDING_CARD/ST_MOVING_MOTOR ---> ST_WAIT
            Check = Globals.SMObject.StateMachineRun(DispenserSMClass::EV_FINISH);
            //std::cout<<"[MAIN] Estado actual: "<<Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState)<<std::endl;

            if (Check == 0){
                Response = CheckCodes();
            }
            else if (Check == 2){
                Response.StatusCode = 500;
                Response.Message = "Fallo con el dispensador. Codigo de fallo: "+ Globals.DispenserObject.ErrorOCode + " - Mensaje de fallo: " + Globals.DispenserObject.ErrorOMsg;
            }
            else {
                Response.StatusCode = 507;
                Response.Message = "Fallo con el dispensador. No responde";
            }
        }
        else {
            Response = Connect();
        }
        
        return Response;
    }

    Flags_t DispenserControlClass::GetDispenserFlags() {

        Flags_t DispenserFlags;

        DispenserFlags.RFICCardInG = Globals.DispenserObject.RFICCardInGate;
        DispenserFlags.RecyclingBoxF = Globals.DispenserObject.RecyclingBoxFull;
        DispenserFlags.CardInG = Globals.DispenserObject.CardInGate;
        DispenserFlags.CardsInD = Globals.DispenserObject.CardsInDispenser;
        DispenserFlags.DispenserF = Globals.DispenserObject.DispenserFull;

        return DispenserFlags;
    }

    TestStatus_t DispenserControlClass::TestStatus() {

        TestStatus_t Status;

        Status.Version = VERSION;
        Status.Device = 10;

        Response = CheckDevice();

        Status.ErrorCode = Response.StatusCode;
        Status.Message = Response.Message;
        Status.AditionalInfo = "LastErrorCode: " + Globals.DispenserObject.ErrorOCode + " - LastErrorMsg: "+ Globals.DispenserObject.ErrorOMsg;

        if ((Response.StatusCode == 404) | (Response.StatusCode == 500) | (Response.StatusCode == 504) | (Response.StatusCode == 507)){
            Status.ErrorType = 0;
            Status.Priority = 1;
        }
        else {
            Status.ErrorType = 1;
            if ((Response.StatusCode == 201) | (Response.StatusCode == 202)){
                if (strcmp(Globals.SMObject.StateMachineGetStateName(Globals.SMObject.SM.CurrState), "ST_ERROR") != 0){
                    Status.Priority = 0;
                }
                else {
                    Status.Priority = 1;
                }

            }
            else {
                Status.Priority = 1;
            }
            
        }
        
        return Status;
    }
}

