/**
 * @file StateMachine.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Header de la maquina de estados
 * @version 1.1
 * @date 2023-05-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include "ValidatorNV10.hpp"

namespace StateMachine{

    using namespace ValidatorNV10;
    
    class SMClass{
        public:

            enum State_t{
                ST_IDLE,
                ST_CONNECT,
                ST_DISABLE, 
                ST_ENABLE,
                ST_POLLING,
                ST_CHECK,
                ST_ERROR                
            };

            struct StateMachine_t{
                State_t CurrState;
            };

            enum Event_t{
                EV_ANY,
                EV_SUCCESS_CONN,
                EV_CALL_POLLING,
                EV_CHECK,
                EV_READY,
                EV_FINISH_POLL,
                EV_POLL,
                EV_LOOP,
                EV_RESET,
                EV_ERROR,
            };

            /**
             * @brief Construct a new SMClass object
             * @param _NV10Class_p Apuntador a la clase del archivo ValidatorNV10.cpp
             */
            SMClass(ValidatorNV10::NV10Class *_NV10Class_p);

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
            * @brief Quinto estado, Revisa el ultimo codigo de rechazo, hace un poll para revisar el ultimo billete y borrarlo
            * @return int - Retorna 0 si pudo correr las 2 anteriores funciones exitosamente
            * @return int - Retorna 1 si no se pudo correr alguna de las 2 funciones anteriores o si el evento no esta en cero
            */
            int RunCheck();

            /**
             * @brief Corre la maquina de estados dependiendo del evento ingresado y del estado que tenga actualmente
             * @param Event Evento que ingresa para hacer el cambio de estado
             * @return int Si la funcion asociada hizo el cambio de estado con exito retorna un 1 / -1, de lo contrario retorna 0
             */
            int StateMachineRun(Event_t Event);

            /**
             * @brief Funcion para conocer el nombre del estado actual de la maquina de estados
             * @param State Se introduce un enum del estado actual
             * @return const char* Regresa una cadena de caracteres con el nombre dele stado actual
             */
            const char * StateMachineGetStateName(State_t State);
    };
};

#endif /* STATEMACHINE_HPP */

