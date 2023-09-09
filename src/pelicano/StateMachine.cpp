/**
 * @file StateMachine.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Codigo fuente de la maquina de estados
 * @version 1.1
 * @date 2023-05-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "StateMachine.hpp"

namespace StateMachine{

    using namespace ValidatorPelicano;

    PelicanoClass *PelicanoObject;

    SMClass::SMClass(ValidatorPelicano::PelicanoClass *_PelicanoClass_p){
        PelicanoObject = _PelicanoClass_p;
        SM.CurrState = SMClass::ST_IDLE;
    };

    struct StateFunctionRow_t{
        const char * name;
        int (PelicanoClass::*func)(void);
    };

    static StateFunctionRow_t StateFunctionValidatorPelicano[] = {
            // NAME         // FUNC
        { "ST_IDLE",       &PelicanoClass::StIdle },      
        { "ST_CONNECT",    &PelicanoClass::StConnect },      
        { "ST_CHECK",      &PelicanoClass::StCheck },     
        { "ST_ENABLE",     &PelicanoClass::StEnable },
        { "ST_POLLING",    &PelicanoClass::StPolling },
        { "ST_CLEANBOWL",  &PelicanoClass::StCleanBowl }, 
        { "ST_RESET",      &PelicanoClass::StReset }, 
        { "ST_ERROR",      &PelicanoClass::StError }, 
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

        { SMClass::ST_CHECK,         SMClass::EV_CALL_POLLING,    SMClass::ST_ENABLE},
        { SMClass::ST_CHECK,         SMClass::EV_CHECK,           SMClass::ST_CHECK},
        { SMClass::ST_CHECK,         SMClass::EV_TRASH,           SMClass::ST_CLEANBOWL},
        { SMClass::ST_CHECK,         SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_ENABLE,        SMClass::EV_READY,           SMClass::ST_POLLING},
        { SMClass::ST_ENABLE,        SMClass::EV_ERROR,           SMClass::ST_ERROR},
        
        { SMClass::ST_POLLING,       SMClass::EV_FINISH_POLL,     SMClass::ST_CLEANBOWL},
        { SMClass::ST_POLLING,       SMClass::EV_POLL,            SMClass::ST_POLLING},
        { SMClass::ST_POLLING,       SMClass::EV_ERROR,           SMClass::ST_ERROR},

        { SMClass::ST_CLEANBOWL,     SMClass::EV_EMPTY,           SMClass::ST_RESET},
        { SMClass::ST_CLEANBOWL,     SMClass::EV_ERROR,           SMClass::ST_ERROR},
        { SMClass::ST_CLEANBOWL,     SMClass::EV_ANY,             SMClass::ST_CHECK},
        { SMClass::ST_CLEANBOWL,     SMClass::EV_FINISH_POLL,     SMClass::ST_CLEANBOWL},

        { SMClass::ST_RESET,         SMClass::EV_LOOP,            SMClass::ST_CHECK},
        { SMClass::ST_RESET,         SMClass::EV_ANY,             SMClass::ST_RESET},
        { SMClass::ST_RESET,         SMClass::EV_ERROR,           SMClass::ST_ERROR},
        { SMClass::ST_ERROR,         SMClass::EV_ANY,             SMClass::ST_IDLE},
    };

    void SMClass::InitStateMachine() {
        SM.CurrState = SMClass::ST_IDLE; 
        (PelicanoObject[0].*(StateFunctionValidatorPelicano[SM.CurrState].func))();
    }

    int SMClass::RunCheck() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = SMClass::ST_CHECK; 
        Response = (PelicanoObject[0].*(StateFunctionValidatorPelicano[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }

    int SMClass::RunReset() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = SMClass::ST_RESET; 
        Response = (PelicanoObject[0].*(StateFunctionValidatorPelicano[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }

    int SMClass::RunClean() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = SMClass::ST_CLEANBOWL; 
        Response = (PelicanoObject[0].*(StateFunctionValidatorPelicano[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }
    
    
    int SMClass::StateMachineRun(Event_t Event) {
        int Response = -1;
        for(long unsigned int i = 0; i < sizeof(StateTransition)/sizeof(StateTransition[0]); i++) {
            if(StateTransition[i].CurrState == SM.CurrState) {
                if(StateTransition[i].Event == Event) {
                    SM.CurrState =  StateTransition[i].NextState;
                    Response = (PelicanoObject[0].*(StateFunctionValidatorPelicano[SM.CurrState].func))();
                    return Response;
                    break;
                }
            }
        }
        return Response;
    }
    
    const char * SMClass::StateMachineGetStateName(State_t State) {
        return StateFunctionValidatorPelicano[State].name;
    }
};