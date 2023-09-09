/**
 * @file AzkoyenControl.cpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Header del archivo principal que expone las funciones del validador Azkoyen
 * @version 1.1
 * @date 2023-05-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef AZKOYENCONTROL
#define AZKOYENCONTROL

#include <stdio.h>
#include <string>
#include <iostream>
#include "StateMachine.hpp"
#include "ValidatorAzkoyen.hpp"

namespace AzkoyenControl{

    using namespace StateMachine;
    using namespace ValidatorAzkoyen;

    struct Response_t{
        int StatusCode;
        std::string Message;
    };

    struct CoinError_t{
        int StatusCode;
        int Event;
        int Coin;
        std::string Message;
        int Remaining;
    };

    struct CoinLost_t{
        int CoinCinc;
        int CoinCien;
        int CoinDosc;
        int CoinQuin;
        int CoinMil;
    };

    struct TestStatus_t{
        std::string Version;
        int Device;
        int ErrorType; 
        int ErrorCode;
        std::string Message; 
        std::string AditionalInfo;
        int Priority; 
    };

    class GlobalVariables {
        public:
            AzkoyenClass AzkoyenObject;
            AzkoyenClass* AzkoyenPointer;
            SMClass SMObject;
            GlobalVariables() : AzkoyenObject(), AzkoyenPointer(&AzkoyenObject), SMObject(AzkoyenPointer) {}
    };

    class AzkoyenControlClass{
        public:
        
            // READ ONLY
            int PortO;

            // WRITE ONLY
            int WarnToCritical;
            int MaxCritical;
            std::string Path;
            int LogLvl;
            int MaximumPorts;

            GlobalVariables Globals;

            AzkoyenControlClass();
            ~AzkoyenControlClass();
            void InitLog();
            Response_t Connect();
            Response_t CheckDevice();
            Response_t StartReader();
            CoinError_t GetCoin();
            CoinLost_t GetLostCoins();
            Response_t ModifyChannels(int InhibitMask1,int InhibitMask2);
            Response_t StopReader();
            Response_t ResetDevice();
            TestStatus_t TestStatus();
            Response_t CheckCodes(int Check);
    };
}

#endif /* AZKOYENCONTROL */