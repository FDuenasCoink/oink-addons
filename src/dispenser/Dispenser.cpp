/**
 * @file Dispenser.cpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Codigo fuente de las funciones del dispensador
 * @version 1.1
 * @date 2023-05-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "Dispenser.hpp"

namespace Dispenser{

    // --------------- EXTERNAL VARIABLES --------------------//
    
    //READ ONLY

    int SerialPort;
    bool SuccessConnect;
    bool Initialized;
    int PortO;

    bool CardsInDispenser;
    bool CardInGate;
    bool RFICCardInGate;
    bool DispenserFull;
    bool RecyclingBoxFull;

    std::string ErrorOCode;
    std::string ErrorOMsg;
    int ErrorOPriority;

    //WRITE ONLY

    int LoggerLevel;
    std::string LogFilePath;
    int MaxPorts;
    int MaxInitAttempts;
    int ShortTime;
    int LongTime;

    // --------------- INTERNAL VARIABLES --------------------//

    bool Scanning;

    std::vector<unsigned char> MSGINIT          = {0xF2, 0x00, 0x00, 0x03, 0x43, 0x30, 0x33, 0x03, 0xB2};
    std::vector<unsigned char> MSGDISPENSECARD  = {0xF2, 0x00, 0x00, 0x03, 0x43, 0x32, 0x30, 0x03, 0xB3};
    std::vector<unsigned char> MSGGETSTATUS     = {0xF2, 0x00, 0x00, 0x03, 0x43, 0x31, 0x30, 0x03, 0xB0};
    std::vector<unsigned char> MSGRETURNCARD    = {0xF2, 0x00, 0x00, 0x03, 0x43, 0x32, 0x33, 0x03, 0xB0};
    std::vector<unsigned char> ACK              = {0x06};


    std::string DEFAULTERROR = "Error por defecto";
    
    ErrorCodesRow_t ErrO;
    StatusCodesRow_t GateState;
    StatusCodesRow_t BoxState;
    StatusCodesRow_t RecyclingBoxState;
    
    std::shared_ptr<spdlog::logger> logger;

    // --------------- STRUCTS --------------------//

    static ErrorCodesRow_t ErrorCodesDispenser[] = {
        //CODE//MSG               //FOUND
        { "00","Undefined command",1},
        { "01","Errors in command parameters",1},
        { "02","Error in the command execution order",1},
        { "03","Hardware does not support commands",1},
        { "04","Command data error (error in communication packets DATA)",1},
        { "05","IC card is contacted but not released",1},
        { "06","IC card is contacted but not released",1},
        { "07","IC card is contacted but not released",1},
        { "08","IC card is contacted but not released",1},
        { "09","IC card is contacted but not released",1},
        { "10","Clogged card",1},
        { "11","Code not found, may be code is Clogged card",1},
        { "12","Sensor error",1},
        { "13","Long card error",1},
        { "14","Short card error",1},
        { "40","The card has been pulled away when recycling card",1},
        { "41","IC card electromagnet error",1},
        { "42","IC card electromagnet error",1},
        { "43","Card cannot be moved from IC card slot",1},
        { "44","Card cannot be moved from IC card slot",1},
        { "45","Cards are artificially moved",1},
        { "46","Cards are artificially moved",1},
        { "47","Cards are artificially moved",1},
        { "48","Cards are artificially moved",1},
        { "49","Cards are artificially moved",1},
        { "50","Recycled cards‟ counter overflows",1},
        { "51","Motor error",1},
        { "52","Motor error",1},
        { "53","Motor error",1},
        { "54","Motor error",1},
        { "55","Motor error",1},
        { "56","Motor error",1},
        { "57","Motor error",1},
        { "58","Motor error",1},
        { "59","Motor error",1},
        { "60","IC card power supply is short-circuited",1},
        { "61","IC card activation failed",1},
        { "62","IC card does not support the current command",1},
        { "63","IC card does not support the current command",1},
        { "64","IC card does not support the current command",1},
        { "65","IC card is not activated",1},
        { "66","The current IC card does not support the command",1},
        { "67","Transmission IC card data error",1},
        { "68","Transmission IC card data timeout",1},
        { "69","CPU / SAM card does not comply with EMV standard",1},
        { "A0","Card dispensing stack (box) is empty, there is no card in card stack",1},
        { "A1","Card collection box is full",2},
        { "A2","Card collection box is full",2},
        { "A3","Card collection box is full",2},
        { "A4","Card collection box is full",2},
        { "A5","Card collection box is full",2},
        { "A6","Card collection box is full",2},
        { "A7","Card collection box is full",2},
        { "A8","Card collection box is full",2},
        { "A9","Card collection box is full",2},
        { "B0","Card dispenser is not reset",3},
    };

    static StatusCodesRow_t Status0CodesDispenser[] = {
        { "0","There is no card in gate",0},
        { "1","There is a card at exit slot of card dispenser channel",0},
        { "2","There is a card at RF / IC card slot of card dispenser channel",1},
    };
    
    static StatusCodesRow_t Status1CodesDispenser[] = {
        { "0","There are no cards in dispenser",1},
        { "1","There are few cards in card dispensing box",0},
        { "2","There are enough cards in card dispensing box",0},
    };
    
    static StatusCodesRow_t Status2CodesDispenser[] = {
        { "0","Recycling box is not full of cards",0},
        { "1","Recycling bin is full of cards",1},
    };
    
    static ErrorCodeExComm_t ErrorCodesExComm[] = {
        {-6,"Function ExComm was not executed",2},
        {-5,"Writing successful but cannot read",1},
        {-4,"Writing length different, writing error",1},
        {-3,"Response could not be identified",1},
        {-2,"Recongnized fail command but ACK was not sent",3},
        {-1,"Recongnized success command but ACK was not sent",0},
        { 0,"Success command identified",0},
        { 1,"Fail command identified",3},
        { 2,"Uknown response code",1},
        { 3,"Datastart was not found, response is corrupted",1},
        { 4,"Timeout, dispenser not responding",1},
        { 5,"Device does not return ACK",1},
    };

    static SpdlogLevels_t SpdlogLvl[] = {
        {0,"trace"},
        {1,"debug"},
        {2,"info"},
        {3,"warn"},
        {4,"error"},
        {5,"critical"},
        {6,"off"},
    };
    
    // --------------- CONSTRUCTOR FUNCTIONS --------------------//

    DispenserClass::DispenserClass(){
        
        SerialPort = 0;
        SuccessConnect = false;
        Initialized = false;
        PortO = 0;

        CardsInDispenser = false;
        CardInGate = false;
        RFICCardInGate = false;
        DispenserFull = false;
        RecyclingBoxFull = false;

        ErrorOCode = DEFAULTERROR;
        ErrorOMsg = DEFAULTERROR;
        ErrorOPriority = 0;
    }

    DispenserClass::~DispenserClass(){}

    // --------------- LOGGER FUNCTIONS --------------------//

    SpdlogLevels_t DispenserClass::SearchSpdlogLevel(int Code){
        SpdlogLevels_t SPLvl;
        SPLvl.Message = "SpdlogLvl not found!!!"; 

        for (long unsigned int i = 0; i < sizeof(SpdlogLvl)/sizeof(SpdlogLvl[0]); i++) {
            if (SpdlogLvl[i].Code == Code) {
                SPLvl.Message = SpdlogLvl[i].Message;
                break;
            }
        }
        
        SPLvl.Code = Code;
        return SPLvl;
    }

    void DispenserClass::SetSpdlogLevel(){
        spdlog::set_level(static_cast<spdlog::level::level_enum>(LoggerLevel)); // Set global log level
    }

    // --------------- SEARCH FUNCTIONS --------------------//

    StatusCodesRow_t DispenserClass::SearchSuccessCode0 (std::string SCode){
        
        StatusCodesRow_t SucC;
        
        SucC.Message = "Code not found!!!";
        SucC.Priority = 1;

        for (long unsigned int i = 0; i < sizeof(Status0CodesDispenser)/sizeof(Status0CodesDispenser[0]); i++) {
            if (Status0CodesDispenser[i].Status == SCode) {
                SucC.Message = Status0CodesDispenser[i].Message;
                SucC.Priority = Status0CodesDispenser[i].Priority;

                if (SCode == "0"){
                    CardInGate = false;
                    RFICCardInGate = false;
                }
                else if (SCode == "1"){
                    RFICCardInGate = false;
                    CardInGate = true;
                }
                else if (SCode == "2"){
                    CardInGate = false;
                    RFICCardInGate = true;
                }
                
                break;
            }
        }
        
        SucC.Status = SCode;

        return SucC;

    }

    StatusCodesRow_t DispenserClass::SearchSuccessCode1 (std::string SCode){
        
        StatusCodesRow_t SucC;
        
        SucC.Message = "Code not found!!!";
        SucC.Priority = 1;

        for (long unsigned int i = 0; i < sizeof(Status1CodesDispenser)/sizeof(Status1CodesDispenser[0]); i++) {
            if (Status1CodesDispenser[i].Status == SCode) {
                SucC.Message = Status1CodesDispenser[i].Message;
                SucC.Priority = Status1CodesDispenser[i].Priority;

                if (SCode == "0"){
                    CardsInDispenser = false;
                    DispenserFull = false;
                }
                else if (SCode == "1"){
                    DispenserFull = false;
                    CardsInDispenser = true;
                }
                else if (SCode == "2"){
                    CardsInDispenser = true;
                    DispenserFull = true;
                }    
                
                break;
            }
        }
        
        SucC.Status = SCode;

        return SucC;

    }

    StatusCodesRow_t DispenserClass::SearchSuccessCode2 (std::string SCode){
        
        StatusCodesRow_t SucC;
        
        SucC.Message = "Code not found!!!";
        SucC.Priority = 1;

        for (long unsigned int i = 0; i < sizeof(Status2CodesDispenser)/sizeof(Status2CodesDispenser[0]); i++) {
            if (Status2CodesDispenser[i].Status == SCode) {
                SucC.Message = Status2CodesDispenser[i].Message;
                SucC.Priority = Status2CodesDispenser[i].Priority;

                if (SCode == "0"){
                    RecyclingBoxFull = false;
                }
                else if (SCode == "1"){
                    RecyclingBoxFull = true;
                }
                
                break;
            }
        }
        
        SucC.Status = SCode;

        return SucC;

    }
    
    ErrorCodesRow_t DispenserClass::SearchErrorCode (std::string ErrorC){
        
        ErrorCodesRow_t Error;
        
        Error.Message = "ErrorCode not found!!!";
        Error.Priority = 1;
        
        for (long unsigned int i = 0; i < sizeof(ErrorCodesDispenser)/sizeof(ErrorCodesDispenser[0]); i++) {
            if (ErrorCodesDispenser[i].ErrorCode == ErrorC) {
                Error.Message = ErrorCodesDispenser[i].Message;
                Error.Priority = ErrorCodesDispenser[i].Priority;
                break;
            }
        }
        
        Error.ErrorCode = ErrorC;

        return Error;
    }

    ErrorCodeExComm_t DispenserClass::SearchErrorCodeExComm (int Code){
        
        ErrorCodeExComm_t CodeMsg;
        
        CodeMsg.Message = "ErrorCode not found!!!";
        CodeMsg.Priority = 1;

        for (long unsigned int i = 0; i < sizeof(ErrorCodesExComm)/sizeof(ErrorCodesExComm[0]); i++) {
            if (ErrorCodesExComm[i].Code == Code) {
                CodeMsg.Message = ErrorCodesExComm[i].Message;
                CodeMsg.Priority = ErrorCodesExComm[i].Priority;
                break;
            }
        }
        
        CodeMsg.Code = Code;

        return CodeMsg;
    }    

    // --------------- STATES OF MACHINE STATE (FUNCTIONS) --------------------//
    
    int DispenserClass::StIdle() {

        SpdlogLevels_t SpdlogLvl;
        SpdlogLvl = SearchSpdlogLevel(LoggerLevel);
        SetSpdlogLevel();
        
        logger->critical("[E0:STIDLE] Setting spdlog level in {}",SpdlogLvl.Message);

        logger->trace("   [E0:STIDLE] --------------------------------------------------------------------------");
        logger->debug("   [E0:STIDLE] --------------------------------------------------------------------------");
        logger->info("    [E0:STIDLE] --------------------------------------------------------------------------");
        logger->warn(" [E0:STIDLE] --------------------------------------------------------------------------");
        logger->error("   [E0:STIDLE] --------------------------------------------------------------------------");
        logger->critical("[E0:STIDLE] --------------------------------------------------------------------------");

        return 0;
    }

    int DispenserClass::StConnect() {

        logger->info("[E1:STCONNECT] Scanning ports");

        PortO = ScanPorts();

        if (PortO >= 0){
            logger->debug("[E1:STCONNECT] Port was found in /dev/ttyUSB{0:d}",PortO);
            return 0;
        }
        else {
            logger->debug("[E1:STCONNECT] Port was NOT found ......");
            return 1;
        }
    }

    int DispenserClass::StInit() {

        int Response = -1;

        if (Initialized){
            logger->debug("[E2:STINIT] Dispenser was initialized");
            Response = 0;
        }
        else {
            logger->debug("[E2:STINIT] Running InitDispenser");
            Response = InitDispenser();
        }
        return Response;
    }
    
    int DispenserClass::StWait() {

        int Response = -1;

         logger->debug("[E3:STWAIT] Running CheckStatus");

        Response = CheckStatus();

        if (Response == 1){
            logger->error("[E3:STWAIT] Failed to run CheckStatus");
        }
        else if (Response == 0){
            PrintCheck();
        }
        
        return Response;
    }

    int DispenserClass::StMovingMotor() {

        int ResponseDisp = -1;
        int ResponseCheck = -1;

        int Response = -1;
        
        if ( CardsInDispenser | DispenserFull ){

            logger->debug("[E4:STMOVINGMOTOR] Running DispenseCard");

            ResponseDisp = DispenseCard();

            if (ResponseDisp == 0){
                if (CardInGate){
                    logger->debug("[E4:STMOVINGMOTOR] Card is in gate");
                    Response = 0;
                }
                else if (RFICCardInGate){
                    logger->warn("[E4:STMOVINGMOTOR] There is a card stuck in the dispenser");
                    Response = 2;
                }
                else {
                    logger->warn("[E4:STMOVINGMOTOR] Card is NOT in gate");
                    Response = 0;
                }
            }
            else if (ResponseDisp == 2){
                logger->error("[E4:STMOVINGMOTOR] Card was not dispensed, something happened");
                Response = 2;
            }
            else {
                logger->debug("[E5:STHANDINGCARD] Running CheckStatus");

                ResponseCheck = CheckStatus();

                if ((ResponseCheck == 0) & (CardInGate)){
                    PrintCheck();
                    Response = 0;
                }
                else {
                    Response = ResponseCheck;
                }
            }
        }
        else {
            logger->warn("[E4:STMOVINGMOTOR] There are no cards in dispenser");
            Response = 3;
        }
        
        return Response;
    }
    
    int DispenserClass::StHandingCard() {
        
        int Response = -1;
        int Res = -1;

        if (CardInGate){

            logger->debug("[E5:STHANDINGCARD] Running ReturnCardToBox");

            Response = ReturnCardToBox();

            if (Response == 0){
                logger->debug("[E5:STHANDINGCARD] Card is in recycling box");
                Res = 0;
            }
            else if (Response == 2){
                logger->debug("[E5:STHANDINGCARD] There was an error running ReturnCardToBox, check ErrorCode");
                Res = 2;
            }
            else {
                logger->debug("[E5:STHANDINGCARD] Running CheckStatus");

                Response = CheckStatus();

                if ((CardInGate == false) & (Response == 0) & (RFICCardInGate == false)){
                    logger->debug("[E5:STHANDINGCARD] Problem solved!");
                    Res = 0;
                }
                else {
                    logger->warn("[E5:STHANDINGCARD] Check log, problem could not solved!");
                    Res = Response;
                }
            }
        }
        else if (RFICCardInGate){
            logger->error("[E5:STHANDINGCARD] There is a card stuck in the dispenser");
            Res = 2;
        }
        else {
            logger->debug("[E5:STHANDINGCARD] There is not a card in gate");
            Res = 3;
        }

        return Res;
    }
    
    int DispenserClass::StError() {
        
        int Response = -1;

        logger->error("[EE:STERROR] In error state :(");
        logger->error("[EE:STERROR] Checking last status.....");

        logger->debug("[EE:STERROR] Running CheckStatus");

        Response = CheckStatus();

        if (Response == 1){
            logger->error("[E3:STWAIT] Failed to run CheckStatus");
        }
        else if (Response == 0){
            PrintCheck();
        }
        
        return Response;
    }
    
    // --------------- MAIN FUNCTIONS --------------------//

    // Función para inicializar el logger
    void DispenserClass::InitLogger(const std::string& Path) {
        // Crear el daily_logger y asignarlo a la variable logger
        logger = spdlog::daily_logger_mt("ValidatorDispenser", Path, 23, 59);
    }
    
    //Connects to port /dev/ttyUSB% where % is the port number (Port)
    int DispenserClass::ConnectSerial(int Port){

        if (Port<0){
            logger->error("[ConnectSerial] Invalid port!");
            return 1;
        }
        else {
            logger->debug("[ConnectSerial] Connecting to /dev/ttyUSB{0:d} port",Port);
            char DeviceName [50];
            sprintf (DeviceName,"/dev/ttyUSB%d",Port);
            SerialPort = open(DeviceName, O_RDWR | O_NOCTTY );

            if (SerialPort > 0){

                struct termios Tty;
                memset(&Tty, 0, sizeof(Tty));

                //Read existing settings, and handle any error
                if (tcgetattr(SerialPort, &Tty) != 0) {
                    logger->error("[ConnectSerial] Error reading actual settings in this port. Error: {0:d}, from tcgetattr: {1}",errno,strerror(errno));
                    SuccessConnect = false;
                    return 2;
                }

                Tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
                Tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
                Tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
                Tty.c_cflag |= CS8; // 8 bits per byte (most common)
                Tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
                Tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)

                Tty.c_iflag &= ~ICRNL;
                Tty.c_iflag &= ~IXON;

                Tty.c_oflag &= ~OPOST;
                Tty.c_oflag &= ~ONLCR;

                Tty.c_lflag &= ~ISIG;
                Tty.c_lflag &= ~ICANON;
                Tty.c_lflag &= ~ECHO;
                Tty.c_lflag &= ~ECHOE;
                Tty.c_lflag &= ~ECHOK;

                Tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
                Tty.c_cc[VMIN] = 0; // Wait from time 0

                cfsetispeed(&Tty, B9600); //Set IN baud rate in 9600
                cfsetospeed(&Tty, B9600); //Set OUT baud rate in 9600

                //Save existing settings, and handle any error
                if (tcsetattr(SerialPort, TCSANOW, &Tty) != 0) {
                    logger->error("[ConnectSerial] Error writing new settings in this port. Error: {0:d}, from tcgetattr: {1}",errno,strerror(errno));
                    SuccessConnect = false;
                    return 3;
                }

                logger->debug("[ConnectSerial] Successfully connected to /dev/ttyUSB{0:d}",Port);
                SuccessConnect = true;
                return 0;
            }
            else {
                logger->debug("[ConnectSerial] Could no connect to /dev/ttyUSB{0:d}",Port);
                return 4;
            }
        }
    }
    
    //Scan all /dev/ttyUSB ports from 0 to 98
    int DispenserClass::ScanPorts(){
        int Port = -1;
        int Response = -1;

        for (int i = 1; i < MaxPorts ; i++)
        {   
            logger->debug("[ScanPorts] Trying connection to /dev/ttyUSB{0:d}",i-1);
            Response = ConnectSerial(i-1);

            if (Response == 0){   

                logger->debug("[ScanPorts] Connection successfull");
                Response = -1;
                Scanning = true;
                logger->debug("[ScanPorts] Sending Init Command");

                Response = InitDispenser();
                
                if (Response == 0){
                    Port = i-1;
                    logger->debug("[ScanPorts] Dispenser found in port /dev/ttyUSB{0:d}",Port);
                    i=MaxPorts;
                    Scanning = false;
                    return Port;
                }
                else {
                    logger->warn("[ScanPorts] Error in writing/reading or dispenser is NOT connected to /dev/ttyUSB{0:d} port",i-1);
                }

                logger->info("[ScanPorts] Clossing connection in /dev/ttyUSB{0:d}",i-1);
                close(SerialPort);            
            }
        }
        Scanning = false;
        logger->error("[ScanPorts] Dispenser was not found in any port!");
        return Port;
    }

    int DispenserClass::SendingCommand(std::vector<unsigned char> Comm, int AdTime){

        int Response = 2;
        int Res = 1;

        Response = ExecuteCommand(Comm,AdTime);

        ErrorCodeExComm_t Err;
        Err = SearchErrorCodeExComm(Response);

        logger->debug("[SendingCommand] Execute command returns: {0:d} with message: {1}",Err.Code,Err.Message);

        if (Err.Priority == 0){
            logger->trace("[SendingCommand] Everything is OK");
            Res = 0;
        }
        else if (Err.Priority == 1){
            logger->debug("[SendingCommand] Error with comand");
            Res = -1;
        }
        else if (Err.Priority == 2){
            logger->debug("[SendingCommand] Repeat command");
            Res = 1;
        }
        else {
            logger->debug("[SendingCommand] Fail response detected");
            Res = 2;
        }

        return Res;
    }
       
    int DispenserClass::ExecuteCommand(std::vector<unsigned char> Comm, int AdTime){
    
        int Wrlen = -1;
        int Rdlen = -1;
        int Res = -6;

        int Cm = 0;
        int Pm = 0;

        int ShortTime = 0;
        int LongTime = 0;

        int PrevRdlen = 0;

        int Xlen = Comm.size();

        logger->trace("[ExecuteCommand] Writting command");
        Wrlen = write(SerialPort, &Comm[0], Xlen);

        if (Wrlen!=Xlen){
            logger->warn("[ExecuteCommand] Writting error, length expect/received: {0:d}/{1:d} Error: {2}",Xlen,Wrlen,strerror(errno));
            Res = -4;
        }
        else {
            logger->debug("[ExecuteCommand] Length expected is the same: {0:d}",Wrlen);
            
            std::vector<unsigned char> Response(100);
            std::vector<unsigned char> Buffer(100);
            std::vector<unsigned char> BufferAditional(100);

            usleep(300000);

            if (AdTime > 10){
                ShortTime = AdTime;
                usleep(ShortTime);
            }
            else {
                LongTime = AdTime;
                sleep(LongTime);
            }

            logger->debug("[ExecuteCommand] Reading response");

            for (int counter = 0; counter < MaxInitAttempts ; counter++){
                
                Rdlen = read(SerialPort, &Buffer[0], Buffer.size());

                logger->debug("[ExecuteCommand] Reading length: {0:d}",Rdlen);

                if ( ((Rdlen >= 10) & (PrevRdlen == 0)) | (PrevRdlen >= 10) ){
                    
                    if (Rdlen >= 10){
                        //std::copy(Buffer.begin(), Buffer.end(), Response.begin());
                        Response = Buffer;
                    }
                    else {
                        //std::copy(BufferAditional.begin(), BufferAditional.end(), Response.begin());
                        Response = BufferAditional;

                        for (int i = 0; i < PrevRdlen ; i++){
                            logger->warn("[ExecuteCommand] Data BufferAditional: {0}",Response[i]);
                        }
                    }

                    if (Response[0] == 6){

                        Cm = Comm[5];
                        Pm = Comm[6];

                        logger->debug("[ExecuteCommand] Reading length greater than 10 with ACK, handling response... ");

                        Res = HandleResponse(Response,Cm,Pm);

                    }
                    else if (Response[0] == 21){
                        logger->warn("[ExecuteCommand] NAK Received");
                        Res = 5;
                    }
                    else if (Response[0] == 4){
                        logger->warn("[ExecuteCommand] EOT Received");
                        Res = 5;
                    }
                    else {

                        for (int i = 0; i < Rdlen ; i++){
                            logger->error("[ExecuteCommand] Data: {0}",Response[i]);
                        }

                        logger->error("[ExecuteCommand] Message is not complete!");
                        Res = 5;
                    }

                    counter = MaxInitAttempts;
                    break;
                }
                else if (Rdlen < 0){

                    logger->error("[ExecuteCommand] Reading error, length expect: {0:d} Error: {2}",Rdlen,strerror(errno));
                    Res = -5;

                    counter = MaxInitAttempts;
                    break;
                }
                else if ( ((Rdlen > 0) & (Rdlen < 10)) | (PrevRdlen > 0) ){
                    
                    if (counter == 0){
                        std::fill(BufferAditional.begin(), BufferAditional.end(), 0);
                    }

                    std::copy(Buffer.begin(), Buffer.begin() + Rdlen, BufferAditional.begin() + PrevRdlen);
                    PrevRdlen = PrevRdlen + Rdlen;

                    for (int i = 0; i < Rdlen ; i++){
                        logger->error("[ExecuteCommand] Data: {0}",Buffer[i]);
                    }

                    logger->error("[ExecuteCommand] Message is not complete!");
                    Res = 3;

                    if (PrevRdlen < 10){
                        sleep(LongTime);
                    }
                }
                else {
                    logger->warn("[ExecuteCommand] Not responding, timeout!");

                    if (Res != 4){
                        sleep(LongTime);
                    }
                    
                    Res = 4;
                }
            }
        }

        if ((Res!=0)&(Res!=1)&(Res!=5)){
            ioctl(SerialPort, TCIOFLUSH, 2);
        }

        return Res;
    }

    int DispenserClass::HandleResponse(std::vector<unsigned char> Response, int Cm, int Pm){

        int Res = -6;
        int Ack = -6;

        int DataStart = Response[1];
        int CodeError = Response[5];

        bool FlagSameCmd = false;

        if ((Cm == Response[6]) & (Pm == Response[7])){
            FlagSameCmd = true;
        }

        if ((DataStart == 242) & (CodeError == 80)){
            logger->debug("[HandleResponse] DataStart correct and dispenser returns success code");
            if (FlagSameCmd){
                Res = HandleResponseSuccess(Response);
                if (Res == 0){
                    Ack = WriteAck();
                    if (Ack != 0){
                        logger->error("[HandleResponse] Error writting ACK");
                        Res = -1;
                    }
                }
            }
            else {
                logger->error("[HandleResponse] Response CM/PM does not is the same to the original command");
                Res = -3;
            }
        }
        else if (DataStart == 242){
            logger->warn("[HandleResponse] Dispenser returns fail code: {0}",CodeError);
            if (FlagSameCmd){
                Res = HandleResponseError(Response);
                if (Res == 0){
                    Ack = WriteAck();
                    if (Ack != 0){
                        logger->error("[HandleResponse] Error writting ACK");
                        Res = -2;
                    }
                }
            }
            else {
                logger->error("[HandleResponse] Response CM/PM does not is the same to the original command");
                Res = -3;
            }
        }
        else {
            logger->error("[HandleResponse] Response is corrupt, fail code: {0} and DataStart: {1}",CodeError,DataStart);
            Res = 3;
        }

        return Res;
    }

    int DispenserClass::WriteAck(){

        int Wrlen = -1;
        int Res = -6;
        
        std::vector<unsigned char> Comm = ACK;
        
        logger->trace("[WriteAck] Writting ACK");
        Wrlen = write(SerialPort, &Comm[0], 1);

        if (Wrlen != 1){
            logger->error("[WriteAck] Writting error, length expect/received: 1/{0:d} Error: {1}",Wrlen,strerror(errno));
            Res = -1;
        }
        else {
            Res = 0;
        }

        return Res;
    }

    int DispenserClass::HandleResponseSuccess(std::vector<unsigned char> Response){

        int Res = -6;
        std::string DefaultMessage = "Code not found!!!";

        std::string Status0 = std::string(1, static_cast<char>(Response[8]));
        std::string Status1 = std::string(1, static_cast<char>(Response[9]));
        std::string Status2 = std::string(1, static_cast<char>(Response[10]));

        StatusCodesRow_t St0 = SearchSuccessCode0(Status0);
        StatusCodesRow_t St1 = SearchSuccessCode1(Status1);
        StatusCodesRow_t St2 = SearchSuccessCode2(Status2);

        logger->trace("[HandleResponseSuccess] Status code 0: {0} Message: {1}",St0.Status,St0.Message);
        logger->trace("[HandleResponseSuccess] Status code 1: {0} Message: {1}",St1.Status,St1.Message);
        logger->trace("[HandleResponseSuccess] Status code 2: {0} Message: {1}",St2.Status,St2.Message);

        if ((St0.Message != DefaultMessage) & (St1.Message != DefaultMessage) & (St2.Message != DefaultMessage)){
            logger->trace("[HandleResponseSuccess] Print the next flags (1-true / 0-false) -> CardInGate {0:b} DispenserFull: {1:b} RecyclingBoxFull: {2:b}",CardInGate,DispenserFull,RecyclingBoxFull);
            Res = 0;
        }
        else {
            logger->error("[HandleResponseSuccess] Status code not found, response is corupted");
            Res = 2;
        }

        return Res;
    } 
    
    int DispenserClass::HandleResponseError(std::vector<unsigned char> Response){
        
        int Res = -6;
        std::string DefaultMessage = "ErrorCode not found!!!";
        
        std::string ErrorCode1 = std::string(1, static_cast<char>(Response[8]));
        std::string ErrorCode0 = std::string(1, static_cast<char>(Response[9]));

        std::string ErrorCode = ErrorCode1 + ErrorCode0;

        ErrO = SearchErrorCode(ErrorCode);

        ErrorOCode = ErrO.ErrorCode;
        ErrorOMsg = ErrO.Message;
        ErrorOPriority = ErrO.Priority;

        logger->trace("[HandleResponseError] Code: {0} Message: {1}",ErrO.ErrorCode,ErrO.Message);

        if (ErrO.Message != DefaultMessage){
            Res = 1;
        }
        else {
            logger->error("[HandleResponseError] Error code not found, response is corupted");
            Res = 2;
        }

        return Res;
    }

    int DispenserClass::InitDispenser(){
    
        int Response = -5;
        int Res = -1;

        int Attempt;
        
        if (Scanning){
            Attempt = MaxInitAttempts;
        }
        else {
            Attempt = 1;
        }
        
        for (Attempt = Attempt; Attempt <= MaxInitAttempts; Attempt++){
            
            logger->info("[InitDispenser] Executing command MSGINIT");
            Response = SendingCommand(MSGINIT,ShortTime);
            
            if (Response == 0){
                if (Scanning){
                    logger->info("[InitDispenser] Dispenser successfully initialized with 1 attempt");
                }
                else {
                    logger->info("[InitDispenser] Dispenser successfully initialized with {0:d} attempts",Attempt);
                }
                
                Attempt = MaxInitAttempts + 1;
                Initialized = true;
                Res = 0;
                break;
            }
            else if (Response == -1){
                logger->error("[InitDispenser] Writing/Reading error !!!");
                Attempt = MaxInitAttempts + 1;
                Res = 1;
                break;
            }
            else {
                if (Attempt<3){
                    logger->warn("[InitDispenser] Running command again with {0:d} attempts",Attempt);
                }
                Res = 1;
            }
        }

        if (Initialized == false){
            logger->error("[InitDispenser] Dispenser could not be initialized after {0:d} attempts",MaxInitAttempts);
        }
        return Res;
    }
    
    int DispenserClass::DispenseCard(){
        
        int Res = -1;

        logger->info("[DispenseCard] Executing command MSGDISPENSECARD");
        Res = SendingCommand(MSGDISPENSECARD,LongTime);

        if ((Res == 1) | (Res == -1)) {
            logger->error("[CheckStatus] Command MSGGETSTATUS cold not run");
            logger->error("[DispenseCard] Writing/Reading error !!!");
            Res = 1;
        }
        else if (Res == 0){
            logger->info("[DispenseCard] Dispenser successfully move the card to gate");
        }
        else {
            logger->warn("[DispenseCard] Fail code detected");
        }

        return Res;
    }
    
    int DispenserClass::CheckStatus(){
        
        int Res = -6;
        
        logger->info("[CheckStatus] Executing command MSGGETSTATUS");
        Res = SendingCommand(MSGGETSTATUS,ShortTime);
        
        if ((Res == 1) | (Res == -1)) {
            logger->error("[CheckStatus] Command MSGGETSTATUS cold not run");
            Res = 1;
        }

        return Res;
    }
    
    int DispenserClass::ReturnCardToBox(){
        
        int Res = -6;
        
        logger->info("[ReturnCardToBox] Executing command MSGRETURNCARD");
        Res = SendingCommand(MSGRETURNCARD,LongTime);
        
        if ((Res == 1) | (Res == -1)){
            logger->error("[ReturnCardToBox] Command MSGRETURNCARD cold not run");
            Res = 1;
        }

        return Res;
    }
    
    void DispenserClass::PrintCheck(){
        logger->trace("[PrintCheck] Print the next 5 flags (1-true / 0-false)");
        logger->trace("[PrintCheck] Card in gate: {0:b}",CardInGate);
        logger->trace("[PrintCheck] RF/IC Card in gate: {0:b}",RFICCardInGate);
        logger->trace("[PrintCheck] Cards in dispenser: {0:b}",CardsInDispenser);
        logger->trace("[PrintCheck] Dispenser full: {0:b}",DispenserFull);
        logger->trace("[PrintCheck] Recycling box full: {0:b}",RecyclingBoxFull);
    }
}