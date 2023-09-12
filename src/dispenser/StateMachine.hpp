/**
 * @file StateMachine.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Header de la maquina de estados
 * @version 1.1
 * @date 2023-05-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef STATEMACHINE_HPP_DISPENSER
#define STATEMACHINE_HPP_DISPENSER

#include "Dispenser.hpp"

namespace DispenserStateMachine{

    using namespace Dispenser;
    
    class DispenserSMClass{
        public:

            enum State_t{
                ST_IDLE,
                ST_CONNECT,
                ST_INIT,
                ST_WAIT,
                ST_MOVING_MOTOR,
                ST_HANDING_CARD,
                ST_ERROR               
            };

            struct StateMachine_t{
                State_t CurrState;
            };

            enum Event_t{
                EV_ANY,
                EV_SUCCESS_CONN,
                EV_SUCCESS_INIT,
                EV_CALL_DISPENSING,
                EV_WAIT,
                EV_CARD_IN_GATE,
                EV_FINISH,
                EV_RESET,
                EV_ERROR,
            };

            /**
             * @brief Construct a new SMClass object
             * @param _DispenserClass_p Apuntador a la clase del archivo Dispenser.cpp
             */
            DispenserSMClass(Dispenser::DispenserClass *_DispenserClass_p);

            /**
             * @brief Estado actual de la maquina de estados
             */
            StateMachine_t SM;

            /**
             * @brief Estado ultimo de la maquina de estados
             */
            StateMachine_t LS;

            /**
             * @brief Evento actual que cambia el estado de la maquina de estados
             */
            Event_t Evento;

            /**
             * @brief Estado inicial de la maquina de estados
             */
            State_t Estado;

            /**
             * @brief Inicia la maquina de estados y corre la funcion asociada al estado actual
             */
            void InitStateMachine();

            /**
             * @brief Corre la maquina de estados dependiendo del evento ingresado y del estado que tenga actualmente
             * @param Event Evento que ingresa para hacer el cambio de estado
             * @return int Si la funcion asociada hizo el cambio de estado con exito retorna un 1 / -1, de lo contrario retorna 0
             */
            int StateMachineRun(Event_t Event);

            /**
             * @brief Corre el estado ST_WAIT (revisa el estado del dispensador)
             * @return Retorna 0 si pudo revisar exitosamente el estado del dispensador
             * @return Retorna 1 si no pudo revisar el estado del dispensador
             */
            int RunCheck();

            /**
             * @brief Funcion para conocer el nombre del estado actual de la maquina de estados
             * @param State Se introduce un enum del estado actual
             * @return const char* Regresa una cadena de caracteres con el nombre dele stado actual
             */
            const char * StateMachineGetStateName(State_t State);
    };
};

#endif /* STATEMACHINE_HPP */

