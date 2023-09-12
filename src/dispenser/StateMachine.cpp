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

namespace DispenserStateMachine{

    using namespace Dispenser;

    DispenserClass *DispenserObject;

    DispenserSMClass::DispenserSMClass(Dispenser::DispenserClass *_DispenserClass_p){
        DispenserObject = _DispenserClass_p;
        SM.CurrState = DispenserSMClass::ST_IDLE;
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
        DispenserSMClass::State_t CurrState;
        DispenserSMClass::Event_t Event;
        DispenserSMClass::State_t NextState;
    } ;
    
    static StateTransitionRow_t StateTransition[] = {
        // CURR STATE       // EVENT            // NEXT STATE
        { DispenserSMClass::ST_IDLE,          DispenserSMClass::EV_ANY,             DispenserSMClass::ST_CONNECT},

        { DispenserSMClass::ST_CONNECT,       DispenserSMClass::EV_SUCCESS_CONN,    DispenserSMClass::ST_INIT },
        { DispenserSMClass::ST_CONNECT,       DispenserSMClass::EV_ERROR,           DispenserSMClass::ST_ERROR },

        { DispenserSMClass::ST_INIT,          DispenserSMClass::EV_SUCCESS_INIT,    DispenserSMClass::ST_WAIT },
        { DispenserSMClass::ST_INIT,          DispenserSMClass::EV_ERROR,           DispenserSMClass::ST_ERROR },

        { DispenserSMClass::ST_WAIT,          DispenserSMClass::EV_CALL_DISPENSING, DispenserSMClass::ST_MOVING_MOTOR },
        { DispenserSMClass::ST_WAIT,          DispenserSMClass::EV_WAIT,            DispenserSMClass::ST_WAIT },
        { DispenserSMClass::ST_WAIT,          DispenserSMClass::EV_CARD_IN_GATE,    DispenserSMClass::ST_HANDING_CARD },
        { DispenserSMClass::ST_WAIT,          DispenserSMClass::EV_ERROR,           DispenserSMClass::ST_ERROR },

        { DispenserSMClass::ST_MOVING_MOTOR,  DispenserSMClass::EV_CARD_IN_GATE,    DispenserSMClass::ST_HANDING_CARD },
        { DispenserSMClass::ST_MOVING_MOTOR,  DispenserSMClass::EV_FINISH,          DispenserSMClass::ST_WAIT },
        { DispenserSMClass::ST_MOVING_MOTOR,  DispenserSMClass::EV_ERROR,           DispenserSMClass::ST_ERROR },

        { DispenserSMClass::ST_HANDING_CARD,  DispenserSMClass::EV_FINISH,          DispenserSMClass::ST_WAIT },
        { DispenserSMClass::ST_HANDING_CARD,  DispenserSMClass::EV_ERROR,           DispenserSMClass::ST_ERROR },

        { DispenserSMClass::ST_ERROR,         DispenserSMClass::EV_RESET,           DispenserSMClass::ST_IDLE },
    };

    void DispenserSMClass::InitStateMachine() {
        SM.CurrState = DispenserSMClass::ST_IDLE; 
        (DispenserObject[0].*(StateFunctionDispenser[SM.CurrState].func))();
    }

    int DispenserSMClass::RunCheck() {
        int Response = -1;
        LS.CurrState = SM.CurrState; 
        SM.CurrState = DispenserSMClass::ST_WAIT; 
        Response = (DispenserObject[0].*(StateFunctionDispenser[SM.CurrState].func))();
        SM.CurrState = LS.CurrState;
        return Response;
    }
    
    int DispenserSMClass::StateMachineRun(Event_t Event) {
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
    
    const char * DispenserSMClass::StateMachineGetStateName(State_t State) {
        return StateFunctionDispenser[State].name;
    }
    
};