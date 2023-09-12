/**
 * @file StateMachine.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Codigo fuente de la maquina de estados
 * @version 1.1
 * @date 2023-05-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "ValidatorNV10.hpp"
#include "StateMachine.hpp"

namespace StateMachine{

    using namespace ValidatorNV10;

    NV10Class *NV10Object;

    SMClass::SMClass(ValidatorNV10::NV10Class *_NV10Class_p){
        NV10Object = _NV10Class_p;
        SM.CurrState = SMClass::ST_IDLE;
    };

    struct StateFunctionRow_t{
        const char * name;
        int (NV10Class::*func)(void);
    };

    static StateFunctionRow_t StateFunctionValidatorNV10[] = {
            // NAME         // FUNC
        { "ST_IDLE",       &NV10Class::StIdle },      
        { "ST_CONNECT",    &NV10Class::StConnect }, 
        { "ST_DISABLE",    &NV10Class::StDisable },      
        { "ST_ENABLE",     &NV10Class::StEnable },     
        { "ST_POLLING",    &NV10Class::StPolling },
        { "ST_CHECK",      &NV10Class::StCheck },
        { "ST_ERROR",      &NV10Class::StError }, 
    };

    struct StateTransitionRow_t{
        SMClass::State_t CurrState;
        SMClass::Event_t Event;
        SMClass::State_t NextState;
    } ;
    
    static StateTransitionRow_t StateTransition[] = {
        // CURR STATE       // EVENT            // NEXT STATE
        { SMClass::ST_IDLE,          SMClass::EV_ANY,             SMClass::ST_CONNECT},

        { SMClass::ST_CONNECT,       SMClass::EV_SUCCESS_CONN,    SMClass::ST_DISABLE},
        { SMClass::ST_CONNECT,       SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_DISABLE,       SMClass::EV_READY,           SMClass::ST_ENABLE},
        { SMClass::ST_DISABLE,       SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_ENABLE,        SMClass::EV_CALL_POLLING,    SMClass::ST_POLLING},
        { SMClass::ST_ENABLE,        SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_POLLING,       SMClass::EV_FINISH_POLL,     SMClass::ST_CHECK},
        { SMClass::ST_POLLING,       SMClass::EV_POLL,            SMClass::ST_POLLING},
        { SMClass::ST_POLLING,       SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_CHECK,         SMClass::EV_LOOP,            SMClass::ST_DISABLE},
        { SMClass::ST_CHECK,         SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_ERROR,         SMClass::EV_RESET,           SMClass::ST_IDLE},

    };

    void SMClass::InitStateMachine() {
        SM.CurrState = SMClass::ST_IDLE; 
        (NV10Object[0].*(StateFunctionValidatorNV10[SM.CurrState].func))();
    }
    
    int SMClass::RunCheck() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = SMClass::ST_CHECK; 
        Response = (NV10Object[0].*(StateFunctionValidatorNV10[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }

    int SMClass::StateMachineRun(Event_t Event) {
        int Response = -1;
        for(long unsigned int i = 0; i < sizeof(StateTransition)/sizeof(StateTransition[0]); i++) {
            if(StateTransition[i].CurrState == SM.CurrState) {
                if(StateTransition[i].Event == Event) {
                    SM.CurrState =  StateTransition[i].NextState;
                    Response = (NV10Object[0].*(StateFunctionValidatorNV10[SM.CurrState].func))();
                    return Response;
                    break;
                }
            }
        }
        return 0;
    }
    
    const char * SMClass::StateMachineGetStateName(State_t State) {
        return StateFunctionValidatorNV10[State].name;
    }
};