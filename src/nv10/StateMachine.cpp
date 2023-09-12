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

namespace NV10StateMachine{

    using namespace ValidatorNV10;

    NV10Class *NV10Object;

    NV10SMClass::NV10SMClass(ValidatorNV10::NV10Class *_NV10Class_p){
        NV10Object = _NV10Class_p;
        SM.CurrState = NV10SMClass::ST_IDLE;
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
        NV10SMClass::State_t CurrState;
        NV10SMClass::Event_t Event;
        NV10SMClass::State_t NextState;
    } ;
    
    static StateTransitionRow_t StateTransition[] = {
        // CURR STATE       // EVENT            // NEXT STATE
        { NV10SMClass::ST_IDLE,          NV10SMClass::EV_ANY,             NV10SMClass::ST_CONNECT},

        { NV10SMClass::ST_CONNECT,       NV10SMClass::EV_SUCCESS_CONN,    NV10SMClass::ST_DISABLE},
        { NV10SMClass::ST_CONNECT,       NV10SMClass::EV_ERROR,           NV10SMClass::ST_ERROR},

        { NV10SMClass::ST_DISABLE,       NV10SMClass::EV_READY,           NV10SMClass::ST_ENABLE},
        { NV10SMClass::ST_DISABLE,       NV10SMClass::EV_ERROR,           NV10SMClass::ST_ERROR},

        { NV10SMClass::ST_ENABLE,        NV10SMClass::EV_CALL_POLLING,    NV10SMClass::ST_POLLING},
        { NV10SMClass::ST_ENABLE,        NV10SMClass::EV_ERROR,           NV10SMClass::ST_ERROR},

        { NV10SMClass::ST_POLLING,       NV10SMClass::EV_FINISH_POLL,     NV10SMClass::ST_CHECK},
        { NV10SMClass::ST_POLLING,       NV10SMClass::EV_POLL,            NV10SMClass::ST_POLLING},
        { NV10SMClass::ST_POLLING,       NV10SMClass::EV_ERROR,           NV10SMClass::ST_ERROR},

        { NV10SMClass::ST_CHECK,         NV10SMClass::EV_LOOP,            NV10SMClass::ST_DISABLE},
        { NV10SMClass::ST_CHECK,         NV10SMClass::EV_ERROR,           NV10SMClass::ST_ERROR},

        { NV10SMClass::ST_ERROR,         NV10SMClass::EV_RESET,           NV10SMClass::ST_IDLE},

    };

    void NV10SMClass::InitStateMachine() {
        SM.CurrState = NV10SMClass::ST_IDLE; 
        (NV10Object[0].*(StateFunctionValidatorNV10[SM.CurrState].func))();
    }
    
    int NV10SMClass::RunCheck() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = NV10SMClass::ST_CHECK; 
        Response = (NV10Object[0].*(StateFunctionValidatorNV10[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }

    int NV10SMClass::StateMachineRun(Event_t Event) {
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
    
    const char * NV10SMClass::StateMachineGetStateName(State_t State) {
        return StateFunctionValidatorNV10[State].name;
    }
};