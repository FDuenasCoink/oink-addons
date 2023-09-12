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

namespace AzkoyenStateMachine {

    using namespace ValidatorAzkoyen;

    AzkoyenClass *AzkoyenObject;

    AzkoyenSMClass::AzkoyenSMClass(ValidatorAzkoyen::AzkoyenClass *_AzkoyenClass_p){
        AzkoyenObject = _AzkoyenClass_p;
        SM.CurrState = AzkoyenSMClass::ST_IDLE;
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
        AzkoyenSMClass::State_t CurrState;
        AzkoyenSMClass::Event_t Event;
        AzkoyenSMClass::State_t NextState;
    } ;
    
    static StateTransitionRow_t StateTransition[] = {
        // CURR STATE       // EVENT            // NEXT STATE
        { AzkoyenSMClass::ST_IDLE,          AzkoyenSMClass::EV_ANY,             AzkoyenSMClass::ST_CONNECT},

        { AzkoyenSMClass::ST_CONNECT,       AzkoyenSMClass::EV_SUCCESS_CONN,    AzkoyenSMClass::ST_CHECK},
        { AzkoyenSMClass::ST_CONNECT,       AzkoyenSMClass::EV_ERROR,           AzkoyenSMClass::ST_ERROR},

        { AzkoyenSMClass::ST_CHECK,         AzkoyenSMClass::EV_CALL_POLLING,    AzkoyenSMClass::ST_WAIT_POLL},
        { AzkoyenSMClass::ST_CHECK,         AzkoyenSMClass::EV_CHECK,           AzkoyenSMClass::ST_CHECK},
        { AzkoyenSMClass::ST_CHECK,         AzkoyenSMClass::EV_ERROR,           AzkoyenSMClass::ST_ERROR},

        { AzkoyenSMClass::ST_WAIT_POLL,     AzkoyenSMClass::EV_READY,           AzkoyenSMClass::ST_POLLING},
        { AzkoyenSMClass::ST_WAIT_POLL,     AzkoyenSMClass::EV_ERROR,           AzkoyenSMClass::ST_ERROR},

        { AzkoyenSMClass::ST_POLLING,       AzkoyenSMClass::EV_FINISH_POLL,     AzkoyenSMClass::ST_RESET},
        { AzkoyenSMClass::ST_POLLING,       AzkoyenSMClass::EV_POLL,            AzkoyenSMClass::ST_POLLING},
        { AzkoyenSMClass::ST_POLLING,       AzkoyenSMClass::EV_ERROR,           AzkoyenSMClass::ST_ERROR},

        { AzkoyenSMClass::ST_RESET,         AzkoyenSMClass::EV_LOOP,            AzkoyenSMClass::ST_CHECK},
        { AzkoyenSMClass::ST_RESET,         AzkoyenSMClass::EV_ANY,             AzkoyenSMClass::ST_RESET},
        { AzkoyenSMClass::ST_RESET,         AzkoyenSMClass::EV_ERROR,           AzkoyenSMClass::ST_ERROR},

        { AzkoyenSMClass::ST_ERROR,         AzkoyenSMClass::EV_ANY,             AzkoyenSMClass::ST_IDLE},

    };

    void AzkoyenSMClass::InitStateMachine() {
        SM.CurrState = AzkoyenSMClass::ST_IDLE; 
        (AzkoyenObject[0].*(StateFunctionValidatorAzkoyen[SM.CurrState].func))();
    }

    int AzkoyenSMClass::RunCheck() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = AzkoyenSMClass::ST_CHECK; 
        Response = (AzkoyenObject[0].*(StateFunctionValidatorAzkoyen[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }

    int AzkoyenSMClass::RunReset() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = AzkoyenSMClass::ST_RESET; 
        Response = (AzkoyenObject[0].*(StateFunctionValidatorAzkoyen[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }
    
    int AzkoyenSMClass::StateMachineRun(Event_t Event) {
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
    
    const char * AzkoyenSMClass::StateMachineGetStateName(State_t State) {
        return StateFunctionValidatorAzkoyen[State].name;
    }
};