/**
 * @file NV10Control.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Header del archivo principal que expone las funciones del validador NV10
 * @version 1.1
 * @date 2023-05-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef NV10CONTROL
#define NV10CONTROL

#include <stdio.h>
#include <string>
#include <iostream>
#include <bitset> //To use bitset in GetBill()
#include "StateMachine.hpp"
#include "ValidatorNV10.hpp"

namespace NV10Control{

    using namespace NV10StateMachine;
    using namespace ValidatorNV10;

    struct Response_t{
        int StatusCode;
        std::string Message;
    };

    struct BillError_t{
        int StatusCode;
        int Bill;
        std::string Message;
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
            NV10Class NV10Object;
            NV10Class* NV10Pointer;
            NV10SMClass SMObject;
            GlobalVariables() : NV10Object(), NV10Pointer(&NV10Object), SMObject(NV10Pointer) {}
    };

    class NV10ControlClass{
        public:
            
            //READ ONLY
            int PortO;

            //WRITE ONLY
            std::string Path;
            int LogLvl;
            int MaximumPorts;

            GlobalVariables Globals;
            
            NV10ControlClass();
            ~NV10ControlClass();
            void InitLog();
            Response_t Connect();
            Response_t CheckDevice();
            Response_t StartReader();
            BillError_t GetBill();
            Response_t ModifyChannels(int InhibitMask1);
            Response_t StopReader();
            Response_t Reject();
            TestStatus_t TestStatus();
    };
}

#endif /* NV10CONTROL */