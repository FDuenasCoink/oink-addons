/**
 * @file StateMachine.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Codigo fuente de la maquina de estados
 * @version 1.1
 * @date 2023-05-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "StateMachine.hpp"

namespace StateMachine{

    using namespace Dispenser;

    DispenserClass *DispenserObject;

    SMClass::SMClass(Dispenser::DispenserClass *_DispenserClass_p){
        DispenserObject = _DispenserClass_p;
        SM.CurrState = SMClass::ST_IDLE;
    };

    struct StateFunctionRow_t{
        const char * name;
        int (DispenserClass::*func)(void);
    };

    static StateFunctionRow_t StateFunctionDispenser[] = {
            // NAME             // FUNC
        { "ST_IDLE",            &DispenserClass::StIdle },
        { "ST_CONNECT",         &DispenserClass::StConnect },      
        { "ST_INIT",            &DispenserClass::StInit },        
        { "ST_WAIT",            &DispenserClass::StWait },
        { "ST_MOVING_MOTOR",    &DispenserClass::StMovingMotor },    
        { "ST_HANDING_CARD",    &DispenserClass::StHandingCard },      
        { "ST_ERROR",           &DispenserClass::StError },
    };

    struct StateTransitionRow_t{
        SMClass::State_t CurrState;
        SMClass::Event_t Event;
        SMClass::State_t NextState;
    } ;
    
    static StateTransitionRow_t StateTransition[] = {
        // CURR STATE       // EVENT            // NEXT STATE
        { SMClass::ST_IDLE,          SMClass::EV_ANY,             SMClass::ST_CONNECT},

        { SMClass::ST_CONNECT,       SMClass::EV_SUCCESS_CONN,    SMClass::ST_INIT },
        { SMClass::ST_CONNECT,       SMClass::EV_ERROR,           SMClass::ST_ERROR },

        { SMClass::ST_INIT,          SMClass::EV_SUCCESS_INIT,    SMClass::ST_WAIT },
        { SMClass::ST_INIT,          SMClass::EV_ERROR,           SMClass::ST_ERROR },

        { SMClass::ST_WAIT,          SMClass::EV_CALL_DISPENSING, SMClass::ST_MOVING_MOTOR },
        { SMClass::ST_WAIT,          SMClass::EV_WAIT,            SMClass::ST_WAIT },
        { SMClass::ST_WAIT,          SMClass::EV_CARD_IN_GATE,    SMClass::ST_HANDING_CARD },
        { SMClass::ST_WAIT,          SMClass::EV_ERROR,           SMClass::ST_ERROR },

        { SMClass::ST_MOVING_MOTOR,  SMClass::EV_CARD_IN_GATE,    SMClass::ST_HANDING_CARD },
        { SMClass::ST_MOVING_MOTOR,  SMClass::EV_FINISH,          SMClass::ST_WAIT },
        { SMClass::ST_MOVING_MOTOR,  SMClass::EV_ERROR,           SMClass::ST_ERROR },

        { SMClass::ST_HANDING_CARD,  SMClass::EV_FINISH,          SMClass::ST_WAIT },
        { SMClass::ST_HANDING_CARD,  SMClass::EV_ERROR,           SMClass::ST_ERROR },

        { SMClass::ST_ERROR,         SMClass::EV_RESET,           SMClass::ST_IDLE },
    };

    void SMClass::InitStateMachine() {
        SM.CurrState = SMClass::ST_IDLE; 
        (DispenserObject[0].*(StateFunctionDispenser[SM.CurrState].func))();
    }

    int SMClass::RunCheck() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = SMClass::ST_WAIT; 
        Response = (DispenserObject[0].*(StateFunctionDispenser[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }
    
    int SMClass::StateMachineRun(Event_t Event) {
        int Response = -1;
        for (long unsigned int i = 0; i < sizeof(StateTransition)/sizeof(StateTransition[0]); i++) {
            if (StateTransition[i].CurrState == SM.CurrState) {
                if (StateTransition[i].Event == Event) {
                    SM.CurrState =  StateTransition[i].NextState;
                    Response = (DispenserObject[0].*(StateFunctionDispenser[SM.CurrState].func))();
                    return Response;
                    break;
                }
            }
        }
        return 0;
    }
    
    const char * SMClass::StateMachineGetStateName(State_t State) {
        return StateFunctionDispenser[State].name;
    }
    
};