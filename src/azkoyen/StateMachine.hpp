/**
 * @file StateMachine.hpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Header de la maquina de estados
 * @version 1.1
 * @date 2023-05-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef STATEMACHINE_HPP_AZKOYEN
#define STATEMACHINE_HPP_AZKOYEN

#include "ValidatorAzkoyen.hpp"

namespace AzkoyenStateMachine {

    using namespace ValidatorAzkoyen;
    
    class AzkoyenSMClass{
        public:

            enum State_t{
                ST_IDLE,
                ST_CONNECT,
                ST_CHECK, 
                ST_WAIT_POLL,
                ST_POLLING,
                ST_RESET,
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
                EV_ERROR,
            };

            /**
             * @brief Construct a new SMClass object
             * @param _AzkoyenClass_p Apuntador a la clase del archivo ValidatorAzkoyen.cpp
             */
            AzkoyenSMClass(ValidatorAzkoyen::AzkoyenClass *_AzkoyenClass_p);

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
             * @brief Corre el estado CHECK
            * @return int - Retorna 0 si pudo correr las 3 anteriores funciones exitosamente
            * @return int - Retorna 1 si no se pudo correr alguna de las 3 funciones anteriores o si hubo algun codigo de falla grave
            * @return int - Retorna 2 si el mecanismo COS fue activado o si hubo un cambio del bit 0 (NotUsed)
             */
            int RunCheck();

            /**
             * @brief Corre el estado RESET
            * @return int - Retorna 0 si pudo correr las 2 anteriores funciones exitosamente
            * @return int - Retorna 1 si no se pudo correr alguna de las 2 funciones anteriores o si el evento no esta en cero
             */
            int RunReset();

            /**
             * @brief Funcion para conocer el nombre del estado actual de la maquina de estados
             * @param State Se introduce un enum del estado actual
             * @return const char* Regresa una cadena de caracteres con el nombre dele stado actual
             */
            const char * StateMachineGetStateName(State_t State);
    };
};

#endif /* STATEMACHINE_HPP */

