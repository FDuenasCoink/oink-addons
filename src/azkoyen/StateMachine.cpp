/**
 * @file StateMachine.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Codigo fuente de la maquina de estados
 * @version 1.1
 * @date 2023-05-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "StateMachine.hpp"

namespace StateMachine {

    using namespace ValidatorAzkoyen;

    AzkoyenClass *AzkoyenObject;

    SMClass::SMClass(ValidatorAzkoyen::AzkoyenClass *_AzkoyenClass_p){
        AzkoyenObject = _AzkoyenClass_p;
        SM.CurrState = SMClass::ST_IDLE;
    };

    struct StateFunctionRow_t{
        const char * name;
        int (AzkoyenClass::*func)(void);
    };

    static StateFunctionRow_t StateFunctionValidatorAzkoyen[] = {
            // NAME         // FUNC
        { "ST_IDLE",       &AzkoyenClass::StIdle },      
        { "ST_CONNECT",    &AzkoyenClass::StConnect }, 
        { "ST_CHECK",      &AzkoyenClass::StCheck },      
        { "ST_WAIT_POLL",  &AzkoyenClass::StWaitPoll },     
        { "ST_POLLING",    &AzkoyenClass::StPolling },
        { "ST_RESET",      &AzkoyenClass::StReset },
        { "ST_ERROR",      &AzkoyenClass::StError }, 
    };

    struct StateTransitionRow_t{
        SMClass::State_t CurrState;
        SMClass::Event_t Event;
        SMClass::State_t NextState;
    } ;
    
    static StateTransitionRow_t StateTransition[] = {
        // CURR STATE       // EVENT            // NEXT STATE
        { SMClass::ST_IDLE,          SMClass::EV_ANY,             SMClass::ST_CONNECT},

        { SMClass::ST_CONNECT,       SMClass::EV_SUCCESS_CONN,    SMClass::ST_CHECK},
        { SMClass::ST_CONNECT,       SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_CHECK,         SMClass::EV_CALL_POLLING,    SMClass::ST_WAIT_POLL},
        { SMClass::ST_CHECK,         SMClass::EV_CHECK,           SMClass::ST_CHECK},
        { SMClass::ST_CHECK,         SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_WAIT_POLL,     SMClass::EV_READY,           SMClass::ST_POLLING},
        { SMClass::ST_WAIT_POLL,     SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_POLLING,       SMClass::EV_FINISH_POLL,     SMClass::ST_RESET},
        { SMClass::ST_POLLING,       SMClass::EV_POLL,            SMClass::ST_POLLING},
        { SMClass::ST_POLLING,       SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_RESET,         SMClass::EV_LOOP,            SMClass::ST_CHECK},
        { SMClass::ST_RESET,         SMClass::EV_ANY,             SMClass::ST_RESET},
        { SMClass::ST_RESET,         SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_ERROR,         SMClass::EV_ANY,             SMClass::ST_IDLE},

    };

    void SMClass::InitStateMachine() {
        SM.CurrState = SMClass::ST_IDLE; 
        (AzkoyenObject[0].*(StateFunctionValidatorAzkoyen[SM.CurrState].func))();
    }

    int SMClass::RunCheck() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = SMClass::ST_CHECK; 
        Response = (AzkoyenObject[0].*(StateFunctionValidatorAzkoyen[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }

    int SMClass::RunReset() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = SMClass::ST_RESET; 
        Response = (AzkoyenObject[0].*(StateFunctionValidatorAzkoyen[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }
    
    int SMClass::StateMachineRun(Event_t Event) {
        int Response = -1;
        for(long unsigned int i = 0; i < sizeof(StateTransition)/sizeof(StateTransition[0]); i++) {
            if(StateTransition[i].CurrState == SM.CurrState) {
                if(StateTransition[i].Event == Event) {
                    SM.CurrState =  StateTransition[i].NextState;
                    Response = (AzkoyenObject[0].*(StateFunctionValidatorAzkoyen[SM.CurrState].func))();
                    return Response;
                    break;
                }
            }
        }
        return 0;
    }
    
    const char * SMClass::StateMachineGetStateName(State_t State) {
        return StateFunctionValidatorAzkoyen[State].name;
    }
};