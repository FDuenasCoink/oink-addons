/**
 * @file DispenserControl.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Header del archivo principal que expone las funciones del dispensador
 * @version 1.1
 * @date 2023-05-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef DISPENSERCONTROL
#define DISPENSERCONTROL

#include <stdio.h>
#include <string>
#include <iostream>

#include "StateMachine.hpp"
#include "Dispenser.hpp"

namespace DispenserControl{

    using namespace StateMachine;
    using namespace Dispenser;

    struct Response_t{
        int StatusCode;
        std::string Message;
    };

    struct Flags_t{
        bool RFICCardInG;
        bool RecyclingBoxF;
        bool CardInG;
        bool CardsInD;
        bool DispenserF;
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
            DispenserClass DispenserObject;
            DispenserClass* DispenserPointer;
            SMClass SMObject;
            GlobalVariables() : DispenserObject(), DispenserPointer(&DispenserObject), SMObject(DispenserPointer) {}
    };

    class DispenserControlClass{
        public:
            
            //READ ONLY
            int PortO;

            //WRITE ONLY
            std::string Path;
            int LogLvl;
            int MaximumPorts;
            int MaxInitAttempts;
            int ShortTime;
            int LongTime;

            GlobalVariables Globals;
            
            DispenserControlClass();
            ~DispenserControlClass();

            void InitLog();
            Response_t Connect();
            Response_t CheckDevice();
            Response_t CheckCodes();
            Response_t DispenseCard();
            Response_t RecycleCard();
            Response_t EndProcess();
            Flags_t GetDispenserFlags();
            TestStatus_t TestStatus();
    };
}

#endif /* DISPENSERCONTROL */