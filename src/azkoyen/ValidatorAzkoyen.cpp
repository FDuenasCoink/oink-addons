/**
 * @file ValidatorAzkoyen.cpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Codigo fuente de las funciones del validador Azkoyen
 * @version 1.1
 * @date 2023-05-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "ValidatorAzkoyen.hpp"

namespace ValidatorAzkoyen{

    // --------------- EXTERNAL VARIABLES --------------------//

    //READ ONLY
    
    int SerialPort;
    bool SuccessConnect;
    int PortO;
    
    int CoinEvent;
    int CoinEventPrev;

    int CoinCinc;
    int CoinCien;
    int CoinDosc;
    int CoinQuin;
    int CoinMil;

    bool ErrorHappened;
    bool CriticalError;

    int ErrorOCode;
    std::string ErrorOMsg;
    int ErrorOStatic;
    int ErrorOCritical;

    int FaultOCode;
    std::string FaultOMsg;

    bool NoUsedBit;
    bool MeasurePhotoBlocked;
    bool OutPhotoBlocked;
    bool COSAlert;

    int ActOCoin;
    int ActOChannel;

    //WRITE ONLY
    
    int LoggerLevel;
    std::string LogFilePath;
    int MaxPorts;

    // --------------- INTERNAL VARIABLES --------------------//

    bool Scanning;

    std::vector<unsigned char> CMDSIMPLEPOLL    = {0x02, 0x00, 0x01, 0xFE, 0xFF};
    std::vector<unsigned char> CMDSTARTPOLL     = {0x02, 0x00, 0x01, 0xE5, 0x18};
    std::vector<unsigned char> CMDRESETDEVICE   = {0x02, 0x00, 0x01, 0x01, 0xFC};
    std::vector<unsigned char> CMDREQUESTSTATUS = {0x02, 0x00, 0x01, 0xF8, 0x05};
    std::vector<unsigned char> CMDREADOPTOST    = {0x02, 0x00, 0x01, 0xEC, 0x11};
    std::vector<unsigned char> CMDSELFCHECK     = {0x02, 0x00, 0x01, 0xE8, 0x15};
    std::vector<unsigned char> CMDENABLE        = {0x02, 0x02, 0x01, 0xE7, 0xFF, 0xFF, 0x16};
    std::vector<unsigned char> CMDINHIBIT50     = {0x02, 0x02, 0x01, 0xE7, 0xF7, 0xFD, 0x20};

    std::string DEFAULTERROR = "Error por defecto";
    
    ErrorCodePolling_t ErrP;
    ErrorCodePolling_t ErrPPrev;

    FaultCode_t FaultC;

    CoinPolling_t ActCoin;

    std::shared_ptr<spdlog::logger> logger;
    
    static CoinPolling_t CoinPolling[] = {
        //Agregar los demás canales de moneda.
        {1,0},
        {2,0},
        {3,0},
        {4,50},
        {5,100},
        {6,200},
        {7,500},
        {8,0},
        {9,0},
        {10,50},
        {11,100},
        {12,200},
        {13,500},
        {14,1000},
        {15,500},
        {16,1000},
    };

    static ErrorCodePolling_t ErrorCodePolling[] = {
        {0,"Null event",0,0},
        {1,"Reject coin",1,3},
        {2,"Inhibited coin",1,0},
        {3,"Multiple window",1,3},
        {4,"Wake-up timeout",2,3},
        {5,"Validation timeout",2,3},
        {6,"Credit sensor timeout",2,2},
        {7,"Sorter opto timeout",0,3},
        {8,"2nd close coin error",1,3},
        {9,"Accept gate not ready",1,2},
        {10,"Credit sensor not ready",1,2},
        {11,"Sorter not ready",1,0},
        {12,"Reject coin not cleared",1,1},
        {13,"Validation sensor not ready",1,1},
        {14,"Credit sensor blocked",1,1},
        {15,"Sorter opto blocked",1,1},
        {16,"Credit sequence error",0,2},
        {17,"Coin going backwards",0,2},
        {18,"Coin too fast",0,0},
        {19,"Coin too slow",0,0},
        {20,"C.O.S. mechanism activated",0,2},
        {21,"DCE opto timeout",2,0},
        {22,"DCE opto not seen",1,0},
        {23,"Credit sensor reached too early",0,3},
        {24,"Reject coin",1,3},
        {25,"Reject slug",1,3},
        {26,"Reject sensor blocked",0,1},
        {27,"Games overload",0,3},
        {28,"Max. coin meter pulses exceeded",0,3},
        {29,"Accept gate open not closed",0,1},
        {30,"Accept gate closed not open",1,1},
        {31,"Manifold opto timeout",0,3},
        {32,"Manifold opto blocked",1,1},
        {33,"Manifold not ready",1,3},
        {34,"Security status changed",2,3},
        {35,"Motor exception",2,2},
        {36,"Swallowed coin",0,3},
        {37,"Coin too fast",1,0},
        {38,"Coin too slow",1,0},
        {39,"Coin incorrectly sorted",0,3},
        {40,"External light attack",0,2},
        {128,"Inhibited coin",1,0},
        {129,"Inhibited coin",1,0},
        {130,"Inhibited coin",1,0},
        {131,"Inhibited coin",1,0},
        {132,"Inhibited coin",1,0},
        {133,"Inhibited coin",1,0},
        {134,"Inhibited coin",1,0},
        {135,"Inhibited coin",1,0},
        {136,"Inhibited coin",1,0},
        {137,"Inhibited coin",1,0},
        {138,"Inhibited coin",1,0},
        {139,"Inhibited coin",1,0},
        {140,"Inhibited coin",1,0},
        {141,"Inhibited coin",1,0},
        {142,"Inhibited coin",1,0},
        {143,"Inhibited coin",1,0},
        {144,"Inhibited coin",1,0},
        {145,"Inhibited coin",1,0},
        {146,"Inhibited coin",1,0},
        {147,"Inhibited coin",1,0},
        {148,"Inhibited coin",1,0},
        {149,"Inhibited coin",1,0},
        {150,"Inhibited coin",1,0},
        {151,"Inhibited coin",1,0},
        {152,"Inhibited coin",1,0},
        {153,"Inhibited coin",1,0},
        {154,"Inhibited coin",1,0},
        {155,"Inhibited coin",1,0},
        {156,"Inhibited coin",1,0},
        {157,"Inhibited coin",1,0},
        {158,"Inhibited coin",1,0},
        {159,"Inhibited coin",1,0},
        {253,"Data block request",0,3},
        {254,"Coin return mechanism activated",0,3},
        {255,"Unspecified alarm code",0,2},
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

    static ErrorCodeExComm_t ErrorCodesExComm[] = {
        {-6,"[] Function EC/HR/HRP/HRI was not executed"},
        {-5,"[EC] Writting error"},
        {-4,"[EC] Writing successful but cannot read"},
        {-3,"[EC] Timeout, acceptor not responding"},
        {-2,"[HR] Acceptor is busy"},
        {-1,"[HR] ACK Negative"},
        { 0,"[] No news is good news"},
        { 1,"[HR] Unknown data in ACK position"},
        { 2,"[HR] Message was not received completely"},
        { 3,"[HRP] Polling data is incorrect, please reset validator"},
        { 4,"[HRP] Polling error detected"},
        { 5,"[EC] Reading lenth is too short, sleep time is too short"},
        { 6,"[EC] Command not recognized or adress is wrong"},
    };

    static FaultCode_t FaultCodeM[] = {
        {  0,"OK"},
        {  1,"Firmware checksum corrupted"},
        {  2,"Fault on electromagnetic sensors"},
        {  3,"Fault on credit sensors"},
        {  4,"Fault on sound sensor or piezoelectric"},
        {  6,"Fault on diameter sensor"},
        { 20,"Fault on COS mechanism (is open)"},
        { 28,"Sensor module not responding"},
        { 30,"Datablock checksum corrupted"},
        { 33,"Voltage of module sensor is wrong"},
        { 34,"Fault on temperature sensor"},
        { 35,"Fault on double-in sensor"},
        { 41,"Error in COS mechanism (open)"},
        {253,"Coin jam in measurement system"},
        {255,"No valid hardware test: Measuring a coin inside"},
    };

    // --------------- CONSTRUCTOR FUNCTIONS --------------------//

    AzkoyenClass::AzkoyenClass(){
        
        SerialPort = 0;
        SuccessConnect = false;
        PortO = 0;

        CoinEvent = 0;
        CoinEventPrev = 0;

        CoinCinc = 0;
        CoinCien = 0;
        CoinDosc = 0;
        CoinQuin = 0;
        CoinMil = 0;

        ErrorHappened = false;
        CriticalError = false;

        ErrorOCode = 0;
        ErrorOMsg = DEFAULTERROR;
        ErrorOStatic = 0;
        ErrorOCritical = 0;

        FaultOCode = 0;
        FaultOMsg = DEFAULTERROR;

        ActOCoin = 0;
        ActOChannel = 0;

        NoUsedBit = false;
        MeasurePhotoBlocked = false;
        OutPhotoBlocked = false;
        COSAlert = false;
    }

    AzkoyenClass::~AzkoyenClass(){}

    // --------------- LOGGER FUNCTIONS --------------------//

    SpdlogLevels_t AzkoyenClass::SearchSpdlogLevel(int Code){
        SpdlogLevels_t SPLvl;
        SPLvl.Message = "SpdlogLvl not found!!!"; 

        for(long unsigned int i = 0; i < sizeof(SpdlogLvl)/sizeof(SpdlogLvl[0]); i++) {
            if(SpdlogLvl[i].Code == Code) {
                SPLvl.Message = SpdlogLvl[i].Message;
                break;
            }
        }
        
        SPLvl.Code = Code;
        return SPLvl;
    }

    void AzkoyenClass::SetSpdlogLevel(){
        //spdlog::set_level(static_cast<spdlog::level::level_enum>(LoggerLevel)); // Set global log level
        logger->set_level(spdlog::level::debug);
    }

    // --------------- SEARCH FUNCTIONS --------------------//

    ErrorCodeExComm_t AzkoyenClass::SearchErrorCodeExComm (int Code){
        
        ErrorCodeExComm_t Code_msg;
        
        Code_msg.Message = "ErrorCode not found!!!";
        
        for(long unsigned int i = 0; i < sizeof(ErrorCodesExComm)/sizeof(ErrorCodesExComm[0]); i++) {
            if(ErrorCodesExComm[i].Code == Code) {
                Code_msg.Message = ErrorCodesExComm[i].Message;
                break;
            }
        }
        
        Code_msg.Code = Code;

        return Code_msg;
    }

    CoinPolling_t AzkoyenClass::SearchCoin (int Channel){
        
        CoinPolling_t ChannelCoin;
        
        ChannelCoin.Coin = 0;
        
        for(long unsigned int i = 0; i < sizeof(CoinPolling)/sizeof(CoinPolling[0]); i++) {
            if(CoinPolling[i].Channel == Channel) {
                ChannelCoin.Coin = CoinPolling[i].Coin;
                break;
            }
        }
        
        ChannelCoin.Channel = Channel;

        return ChannelCoin;
    }

    ErrorCodePolling_t AzkoyenClass::SearchErrorCodePolling (int Code){
        
        ErrorCodePolling_t CodeMsgRej;
        
        CodeMsgRej.Message = "ErrorCode not found!!!";
        CodeMsgRej.Static = 3;
        CodeMsgRej.Critical = 0;
        
        for(long unsigned int i = 0; i < sizeof(ErrorCodePolling)/sizeof(ErrorCodePolling[0]); i++) {
            if(ErrorCodePolling[i].Code == Code) {
                CodeMsgRej.Message = ErrorCodePolling[i].Message;
                CodeMsgRej.Static = ErrorCodePolling[i].Static;
                CodeMsgRej.Critical = ErrorCodePolling[i].Critical;
                break;
            }
        }
        
        CodeMsgRej.Code = Code;

        return CodeMsgRej;
    }

    FaultCode_t AzkoyenClass::SearchFaultCode (int Code){
        
        FaultCode_t FaultCodeMsg;
        
        FaultCodeMsg.Message = "FaultCode not found!!!";

        for(long unsigned int i = 0; i < sizeof(FaultCodeM)/sizeof(FaultCodeM[0]); i++) {
            if(FaultCodeM[i].Code == Code) {
                FaultCodeMsg.Message = FaultCodeM[i].Message;
                break;
            }
        }
        
        FaultCodeMsg.Code = Code;

        return FaultCodeMsg;
    }

    // --------------- STATES OF MACHINE STATE (FUNCTIONS) --------------------//

    int AzkoyenClass::StIdle() {

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

    int AzkoyenClass::StConnect() {

        logger->info("[E1:STCONNECT] Scanning ports");

        PortO = ScanPorts();
        if (PortO >= 0){
            logger->info("[E1:STCONNECT] Port was found in /dev/ttyUSB{0:d}",PortO);
            return 0;
        }
        else{
            logger->info("[E1:STCONNECT] Port was NOT found ......");
            return 1;
        }
    }

    int AzkoyenClass::StCheck() {

        int Response = 1;

        logger->info("[E2:STCHECK] Checking communication");

        Response = SimplePoll();

        if (Response != 0){
            logger->error("[E2:STCHECK] Bad communication");
            return 1;
        }
        
        logger->info("[E2:STCHECK] Checking fault code");

        Response = SelfCheck();

        //Response = 0;

        if (Response >= 1){
            logger->error("[E2:STCHECK] Fatal error code found");
            return 1;
        }
        else if(Response == -1){
            logger->error("[E2:STCHECK] Bad communication");
            return 1;
        }
        else if(Response == -2){
            logger->warn("[E2:STCHECK] Sending command again ...");
            Response = SelfCheck();
            if (Response != 0){
                logger->error("[E2:STCHECK] SelfCheck could not run");
                return 1;
            }
        }

        logger->debug("[E2:STCHECK] Fault code is: OK");

        logger->info("[E2:STCHECK] Checking opto states");

        Response = CheckOptoStates();

        if (Response == 1){
            logger->critical("[E2:STCHECK] Measure or out phototransistor is blocked!");
            return 1;
        }
        else if (Response == 2){
            logger->error("[E2:STCHECK] NoUsedBit change or COS alert is activated");
            return 2;
        }
        else if(Response == -1){
            logger->error("[E2:STCHECK] Bad communication");
            return 1;
        }
        else if(Response == -2){

            logger->warn("[E2:STCHECK] Sending command again ...");

            Response = CheckOptoStates();

            if(Response == 2){
                logger->error("[E2:STCHECK] NoUsedBit change or COS alert is activated");
                return 2;
            }
            else if (Response != 0){
                logger->error("[E2:STCHECK] CheckOptoStates could not run");
                return 1;
            }
        }

        logger->debug("[E2:STCHECK] 4 Optostates are OK");
        return 0;
    }

    int AzkoyenClass::StWaitPoll() {

        int Response = 1;

        logger->info("[E3:STWAITPOLL] Rebooting device");

        Response = ResetDevice();

        if (Response != 0){
            logger->error("[E3:STWAITPOLL] Acceptor could not reset");
            return 1;
        }

        logger->info("[E3:STWAITPOLL] Enabling channels");

        Response = EnableChannels();

        if (Response != 0){
            logger->error("[E3:STWAITPOLL] Acceptor could not enable channels");
            return 1;
        }

        logger->info("[E3:STWAITPOLL] Checking if event is reset");

        Response = CheckEventReset();

        if (Response != 0){
            logger->error("[E3:STWAITPOLL] Acceptor could not reset event");
            return 1;
        }

        CoinEventPrev = 0;

        return 0;
    }

    int AzkoyenClass::StPolling() {

        int Response = 2;

        logger->info("[E4:STPOLLING] Running CMDSTARTPOLL");

        Response = SendingCommand(CMDSTARTPOLL);

        return Response;
        
    }

    int AzkoyenClass::StReset() {

        int Response = 0;
        
        logger->info("[E6:STRESET] Rebooting device");

        Response = ResetDevice();

        if (Response != 0){
            logger->error("[E6:STRESET] Acceptor could not reset");
            return 1;
        }

        logger->info("[E6:STRESET] Checking if event is reset");

        Response = CheckEventReset();

        if (Response != 0){
            logger->error("[E6:STRESET] Acceptor could not reset event");
            return 1;
        }

        return 0;
    }

    int AzkoyenClass::StError() {

        int Response = -1;

        logger->info("[EE:STERROR] Checking communication");

        Response = SimplePoll();

        if (Response != 0){
            logger->error("[EE:STERROR] Bad communication");
            return 1;
        }

        return 0;
    }

    // --------------- MAIN FUNCTIONS --------------------//

    // Función para inicializar el logger
    void AzkoyenClass::InitLogger(const std::string& Path) {
        // Crear el daily_logger y asignarlo a la variable logger
        logger = spdlog::daily_logger_mt("ValidatorAzkoyen", Path, 23, 59);
    }

    //Connects to port /dev/ttyUSB% where % is the port number (Port)
    int AzkoyenClass::ConnectSerial(int Port){

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
                if(tcgetattr(SerialPort, &Tty) != 0) {
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
            else{
                logger->debug("[ConnectSerial] Could no connect to /dev/ttyUSB{0:d}",Port);
                return 4;
            }
        }
    }
    
    //Scan all /dev/ttyUSB ports from 0 to 98
    int AzkoyenClass::ScanPorts(){
        int Port = -1;
        int Response = -1;

        for (int i=1;i<MaxPorts;i++)
        {   
            logger->debug("[ScanPorts] Trying connection to /dev/ttyUSB{0:d}",i-1);
            Response = ConnectSerial(i-1);

            if (Response==0)
            {   
                logger->debug("[ScanPorts] Connection successfull");
                Response = -1;
                Scanning = true;
                logger->debug("[ScanPorts] Sending simple poll Command (Checking connection)");

                Response = SendingCommand(CMDSIMPLEPOLL);

                if (Response == 0){
                    Port = i-1;
                    logger->debug("[ScanPorts] Validator Azkoyen found in port /dev/ttyUSB{0:d}",Port);
                    i=MaxPorts;
                    Scanning = false;
                    return Port;
                }
                else{
                    logger->warn("[ScanPorts] Error in writing/reading or Validator Azkoyen is NOT connected to /dev/ttyUSB{0:d} port",i-1);
                }

                logger->debug("[ScanPorts] Clossing connection in /dev/ttyUSB{0:d}",i-1);
                close(SerialPort);            
            }
        }
        Scanning = false;
        logger->error("[ScanPorts] Acceptor was not found in any port!");
        return Port;
    }

    int AzkoyenClass::SendingCommand(std::vector<unsigned char> Comm){

        int Response = 2;
        int Res = 1;

        Response = ExecuteCommand(Comm);

        ErrorCodeExComm_t Err;
        Err = SearchErrorCodeExComm(Response);

        logger->debug("[SendingCommand] Execute command returns: {0:d} with message: {1}",Err.Code,Err.Message);

        if(Response == 0){
            //logger->trace("[SendingCommand] Everything is OK");
            Res = 0;
        }
        else if((Response == -5)|(Response == -4)|(Response == 1)){
            logger->debug("[SendingCommand] Fatal error with comand");
            Res = -1;
        }
        else if(Response == 4){
            logger->debug("[SendingCommand] Polling error");
            Res = -2;
        }
        else{
            logger->debug("[SendingCommand] Repeat command");
            Res = 1;
        }
        return Res;
    }

    int AzkoyenClass::ExecuteCommand(std::vector<unsigned char> Comm){

        int Wrlen = -1;
        int Rdlen = -1;
        int Res = -6;
        
        int Xlen = Comm.size();

        //logger->trace("[ExecuteCommand] Writting command");
        Wrlen = write(SerialPort, &Comm[0], Xlen);

        if(Wrlen!=Xlen){
            logger->warn("[ExecuteCommand] Writting error, length expect/received: {0:d}/{1:d} Error: {2}",Xlen,Wrlen,strerror(errno));
            Res = -5;
        }
        else{
            //logger->trace("[ExecuteCommand] Length expected is the same: {0:d}",Wrlen);
            
            std::vector<unsigned char> Buffer(100);
            
            usleep(200000);

            //logger->trace("[ExecuteCommand] Reading response");
            Rdlen = read(SerialPort,&Buffer[0],Buffer.size());

            if(Rdlen > 0){

                logger->debug("[ExecuteCommand] Reading length: {0:d}",Rdlen);

                if (Rdlen >= Xlen){
                    //logger->trace("[ExecuteCommand] Reading length greater or equal than {0}, handling response... ",Xlen);
                    Res = HandleResponse(Buffer,Rdlen,Xlen);
                }
                else if(Rdlen == Xlen-1){
                    logger->warn("[ExecuteCommand] Reading length equal than {0}, not recognized... ",Xlen);
                    Res = 6;   
                }
                else{
                    logger->warn("[ExecuteCommand] Reading length less than {0}, very little waiting time ",Xlen);
                    Res = 5;
                }
            }
            else if (Rdlen < 0){
                logger->warn("[ExecuteCommand] Reading error, length expect: {0:d} Error: {2}",Rdlen,strerror(errno));
                Res = -4;
            }
            else {
                logger->warn("[ExecuteCommand] Not responding, timeout!");
                Res = -3;
            }
        }

        if((Res!=0)&(Res!=4)){
            ioctl(SerialPort, TCIOFLUSH, 2);
        }

        return Res;
    }

    int AzkoyenClass::HandleResponse(std::vector<unsigned char> Response, int Rdlen, int Xlen){

        int Res = -6;
        int Header = 0;

        if (Rdlen >= (Xlen+4)){
            //logger->trace("[HandleResponse] Message seems to be complete");
            if (Response[Xlen + 3] == 0){
                //logger->trace("[HandleResponse] ACK Received!");
                Header = Response[3];
                if ((Rdlen >= Xlen+15)&((Header == 229))){
                    //logger->trace("[HandleResponse] Polling detected, searching response error");
                    Res = HandleResponsePolling(Response,Rdlen);
                }
                else if ((Rdlen < Xlen+15)&((Header == 229))){
                    logger->warn("[HandleResponse] Polling response incomplete!");
                    Res = 2;
                }
                else if((Header == 236)|(Header == 232)){
                    logger->trace("[HandleResponse] Self check or read opto states detected, searching more info");
                    Res = HandleResponseInfo(Response,Rdlen);
                }
                else if((Header == 231)|(Header == 254)|(Header == 1)){
                    logger->trace("[HandleResponse] No more information to check!");
                    Res = 0;
                }
                else{
                    logger->error("[HandleResponse] Error in header!");
                    Res = 6;
                }
            }
            else if(Response[Xlen + 3] == 5){
                logger->warn("[HandleResponse] Negative ACK Received...");  
                Res = -1;
            }
            else if(Response[Xlen + 3] == 6){
                logger->warn("[HandleResponse] Acceptor is BUSY!");  
                Res = -2;
            }
            else{
                logger->error("[HandleResponse] Uknown code in ACK position!");  
                Res = 1;
            }
        }
        else{
            logger->debug("[HandleResponse] Message is NOT complete!!!");
            Res = 2;
        }
        return Res;
    }

    int AzkoyenClass::HandleResponsePolling(std::vector<unsigned char> Response, int Rdlen){

        CriticalError = false;
        int Remaining = 0;

        int Res = -6;

        CoinCinc = 0;
        CoinCien = 0;
        CoinDosc = 0;
        CoinQuin = 0;
        CoinMil = 0;

        int Data = 0;
        int k=1;

        ErrorHappened = false;

        ErrorOCode = 0;
        ErrorOMsg = DEFAULTERROR;
        ErrorOStatic = 0;
        ErrorOCritical = 0;

        ActOCoin = 0;
        ActOChannel = 0;

        if (Response[6] == 11){

            logger->trace("[HandleResponsePolling] Data is correct!");
            
            CoinEvent = Response[9];

            if(CoinEvent != CoinEventPrev){
                
                Remaining = CoinEvent-CoinEventPrev;

                logger->debug("[HandleResponsePolling] CoinEvent: {0} CoinEventPrev: {1}",CoinEvent,CoinEventPrev);

                if (Remaining > 1){
                    logger->debug("[HandleResponsePolling] Remaining events: {0}",Remaining);
                    for (int i = 0; i<2*Remaining; i++){
                        logger->debug("[HandleResponsePolling] Counters, i:{0} k:{1}",i,k);
                        Data = Response[10+i];
                        logger->debug("[HandleResponsePolling] Data: {0}",Data);
                        if( (Data == 0) & (i== 2*(k-1) ) ){
                            ErrorHappened = true;
                        }
                        else if( ( ((Data >= 4) & (Data <= 7)) | ((Data >= 10) & (Data <= 16)) ) & (i== 2*(k-1)) ){
                            ActCoin = SearchCoin(Data);
                            logger->debug("[HandleResponsePolling] Coin: {0}",ActCoin.Coin);
                            if (ActCoin.Coin == 50){
                                CoinCinc++;
                            }
                            else if(ActCoin.Coin == 100){
                                CoinCien++;
                            }
                            else if(ActCoin.Coin == 200){
                                CoinDosc++;
                            }
                            else if(ActCoin.Coin == 500){
                                CoinQuin++;
                            }
                            else if(ActCoin.Coin == 1000){
                                CoinMil++;
                            }
                            k++;
                        }

                        if( (i == ((2*k)-1)) & (ErrorHappened) ){
                            ErrPPrev = SearchErrorCodePolling(Data);
                            k++;
                            if(ErrPPrev.Critical == 1){
                                CriticalError = true;
                                ErrorHappened = true;
                                ErrP = ErrPPrev;
                                //i = 10;
                            }
                            if(CriticalError == false){
                                ErrP = ErrPPrev;
                            }
                        }
                    }
                }
                else{

                    for (int i = 0; i<10; i++){
                        Data = Response[10+i];

                        if((Data == 0)&(i==0)){
                            ErrorHappened = true;
                        }
                        else if(i==0){
                            ActCoin = SearchCoin(Data);
                        }

                        if((i==1)&(ErrorHappened)&(CriticalError == false)){
                            ErrP = SearchErrorCodePolling(Data);
                        }
                        else if((i==1)&(Data == 0)){
                            ActCoin.Coin = 0;
                        }
                    } 
                }

                if(ErrorHappened|CriticalError){

                    ErrorOCode = ErrP.Code;
                    ErrorOMsg = ErrP.Message;
                    ErrorOStatic = ErrP.Static;
                    ErrorOCritical = ErrP.Critical;
 
                    logger->error("[HandleResponsePolling] ----------> Error happened!");
                    logger->error("[HandleResponsePolling] Error code: {0}",ErrP.Code);
                    logger->error("[HandleResponsePolling] Error message: {0}",ErrP.Message);
                    logger->trace("[HandleResponsePolling] Error rejected: {0}",ErrP.Static);
                    logger->trace("[HandleResponsePolling] Error critical: {0}",ErrP.Critical);
                    Res = 4;
                }
                else{

                    ActOCoin = ActCoin.Coin;
                    ActOChannel = ActCoin.Channel;

                    logger->trace("[HandleResponsePolling] ----------> Coin detected");
                    logger->debug("[HandleResponsePolling] Coin: {0}",ActCoin.Coin);
                    logger->trace("[HandleResponsePolling] Coin Channel: {0}",ActCoin.Channel);

                    Res = 0;
                }
                
                for (int i = 0; i<10; i++){
                    Data = Response[10+i];
                    logger->debug("[HandleResponsePolling] Data: {0}",Data);
                }

                CoinEventPrev = CoinEvent;
            }   
            else{
                logger->trace("[HandleResponsePolling] Actual coin event is identical to coin event prev");
                Res = 0;
            }
        }
        else{
            logger->debug("[HandleResponsePolling] Data is not correct!");
            Res = 3;
        }
        return Res;
    }

    int AzkoyenClass::HandleResponseInfo(std::vector<unsigned char> Response, int Rdlen){
        
        int Res = -6;
        int FaultCode = -1;

        if (Response[3] == 232){
            logger->trace("[HandleResponseInfo] Self check detected, checking fault code");

            FaultCode = Response[9];
            FaultC = SearchFaultCode(FaultCode);
            FaultOCode = FaultC.Code;
            FaultOMsg = FaultC.Message;

            logger->debug("[HandleResponseInfo] Fault code: {0}",FaultC.Code);
            logger->debug("[HandleResponseInfo] Fault message: {0}",FaultC.Message);
            logger->debug("[HandleResponseInfo] Fault code complementary: {0}",Response[10]);
            
            Res = 0;
        }
        else if(Response[3] == 236){
            logger->trace("[HandleResponseInfo] Read opto states detected, checking bit mask");
            int StateMask = 0;
            StateMask = Response[9];

            std::bitset<4> Bits(StateMask);

            NoUsedBit = Bits[0];
            MeasurePhotoBlocked = Bits[1];
            OutPhotoBlocked = Bits[2];
            COSAlert = Bits[3];

            if(NoUsedBit){
                logger->trace("[HandleResponseInfo] Error NoUsedBit change!");
            }
            else{
                logger->trace("[HandleResponseInfo] NoUsedBit is set OK");
            }

            if(MeasurePhotoBlocked){
                logger->trace("[HandleResponseInfo] Measue phototransistor is blocked!");
            }
            else{
                logger->trace("[HandleResponseInfo] Measue phototransistor is free");
            }

            if(OutPhotoBlocked){
                logger->trace("[HandleResponseInfo] Out phototransistor is blocked!");
            }
            else{
                logger->trace("[HandleResponseInfo] Out phototransistor is free");
            }

            if(COSAlert){
                logger->trace("[HandleResponseInfo] COS alert activated");
            }
            else{
                logger->trace("[HandleResponseInfo] COS alert deactivated");
            }

            Res = 0;
        }

        return Res;
    }
 
    int AzkoyenClass::CheckOptoStates(){

        int Response  = -2;

        logger->debug("[CheckOptoStates] Reading opto states");
        logger->trace("[CheckOptoStates] Running CMDREADOPTOST");
        
        Response = SendingCommand(CMDREADOPTOST);
        
        if (Response == 0){
            if ( NoUsedBit | COSAlert ){
                logger->warn("[CheckOptoStates] NoUsedBit change or COS alert is activated");
                Response = 2;
            }
            if ( MeasurePhotoBlocked | OutPhotoBlocked ){
                logger->critical("[CheckOptoStates] Measure or out phototransistor is blocked");
                Response = 1;
            }
        }
        else{
            if (Response != -1){
                logger->error("[CheckOptoStates] Repeat command, command was not run successfully");
                Response = -2;
            }
            else{
                logger->critical("[CheckOptoStates] Error sending command");
                Response = -1;
            }
        }

        logger->trace("[CheckOptoStates] Optostates return: Everything is OK");
        return Response;
    }

    int AzkoyenClass::SimplePoll(){

        int Response  = -1;

        logger->debug("[SimplePoll] Checking communication");
        logger->trace("[SimplePoll] Running CMDSIMPLEPOLL");
        
        Response = SendingCommand(CMDSIMPLEPOLL);

        if (Response == -1){
            logger->error("[SimplePoll] Acceptor does not return ACK!");
            return -1;
        }
        else if(Response >= 1){
            logger->warn("[SimplePoll] Running CMDSIMPLEPOLL again .......");
            Response = SendingCommand(CMDSIMPLEPOLL);
            if (Response != 0){
                logger->error("[SimplePoll] Acceptor does not return ACK!");
                return -1;
            }
        }

        logger->trace("[SimplePoll] Acceptor return ACK successfully");
        return 0;
    }

    int AzkoyenClass::SelfCheck(){
        
        int Response = -1;

        logger->debug("[SelfCheck] Running initial revision");
        logger->trace("[SelfCheck] Running CMDSELFCHECK");
        
        Response = SendingCommand(CMDSELFCHECK);

        if (Response == 0){
            if (FaultOCode == 253){
                logger->critical("[SelfCheck] Acceptor blocked, code: {0}",FaultOCode);
                return 1;
            }
            else if((FaultOCode == 1)|(FaultOCode == 30)|(FaultOCode == 255)){
                logger->critical("[SelfCheck] Software error, code: {0}",FaultOCode);
                return 3;
            }
            else if ((FaultOCode != 0) & (FaultOCode != 20) & (FaultOCode != 2)){
                logger->critical("[SelfCheck] Hardware error, code: {0}",FaultOCode);
                return 2;
            }
        }
        else{
            if (Response != -1){
                logger->error("[SelfCheck] Repeat command, command was not run successfully");
                return -2;
            }
            else{
                logger->critical("[SelfCheck] Error sending command");
                return -1;
            }
        }

        logger->trace("[SelfCheck] Acceptor return faultcode: OK");
        return 0;
    }

    int AzkoyenClass::EnableChannels(){
        
        int Response  = -1;

        logger->debug("[EnableChannels] Enabling coins");
        logger->trace("[EnableChannels] Running CMDENABLE");

        Response = SendingCommand(CMDENABLE);
        //Response = SendingCommand(CMDINHIBIT50);

        if (Response != 0){
            logger->error("[EnableChannels] Acceptor could not enable all coins!");
            return -1;
        }

        logger->trace("[EnableChannels] All coins enabled");
        return 0;
    }

    int AzkoyenClass::CheckEventReset(){
        
        int Response = 2;

        logger->debug("[CheckEventReset] Reading coin event");
        logger->trace("[CheckEventReset] Running CMDSTARTPOLL");

        Response = SendingCommand(CMDSTARTPOLL);

        if ((Response == 0)|(Response == -2)){
            if ((CoinEvent == 0)|(CoinEvent == 1)){
                logger->debug("[CheckEventReset] CoinEvent is OK: {0}",CoinEvent);
            }
            else{
                logger->error("[CheckEventReset] CoinEvent is not OK {0}",CoinEvent);
                return 1;
            }
        }
        else{
            if (Response != -1){
                logger->error("[CheckEventReset] Repeat command, command was not run successfully");
                return -2;
            }
            else{
                logger->critical("[CheckEventReset] Error sending command");
                return -1;
            }
        }

        logger->trace("[CheckEventReset] Coin event is ready");
        return 0;
        
    }

    int AzkoyenClass::ResetDevice(){

        int Response  = -1;

        logger->debug("[ResetDevice] Reset device running");
        logger->trace("[ResetDevice] Running CMDRESETDEVICE");

        Response = SendingCommand(CMDRESETDEVICE);

        if (Response !=0){
            logger->error("[ResetDevice] Running command Reset failed");
            return -1;
        }
        
        logger->trace("[ResetDevice] Device was rebooted");

        return 0;
    }

    std::vector<unsigned char> AzkoyenClass::BuildCmdModifyInhibit(int InhibitMask1, int InhibitMask2) {
    
        std::vector<unsigned char> command;

        // Direccion de destino
        command.push_back(0x02);
        // Cantidad de datos a enviar
        command.push_back(0x02);
        // Direccion fuente
        command.push_back(0x01);
        // Header inhibit status
        command.push_back(0xE7);

        // Parámetros InhibitMask1 e InhibitMask2
        command.push_back(static_cast<unsigned char>(InhibitMask1));
        command.push_back(static_cast<unsigned char>(InhibitMask2));

        // Calcular el checksum del mensaje (suma de todos los bytes hasta ahora)
        unsigned char checksum = 0;
        for (size_t i = 0; i < command.size(); i++) {
            logger->debug("Custom command {0}",command[i]);
            checksum += command[i];
        }
        checksum = 256-checksum;

        // Checksum al final del mensaje
        command.push_back(checksum);

        return command;
    }

    int AzkoyenClass::ChangeInhibitChannels(int InhibitMask1, int InhibitMask2){

        int Response  = -1;

        logger->debug("[ChangeInhibitChannels] Changing inhibit channels");
        logger->trace("[ChangeInhibitChannels] Running BuildCmdModifyInhibit");

        std::vector<unsigned char> command = BuildCmdModifyInhibit(InhibitMask1,InhibitMask2);

        Response = SendingCommand(command);

        if (Response !=0){
            logger->error("[ChangeInhibitChannels] Running command failed");
            return -1;
        }
        
        logger->trace("[ChangeInhibitChannels] Inhibited custom channels");

        return 0;
    }
}
