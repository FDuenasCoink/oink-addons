/**
 * @file ValidatorNV10.cpp
 * @author Oscar Pineda (o.pineda@coink.com)
 * @brief Codigo fuente de las funciones del validador NV10
 * @version 1.1
 * @date 2023-05-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "ValidatorNV10.hpp"

namespace ValidatorNV10{

    // --------------- EXTERNAL VARIABLES --------------------//

    //READ ONLY

    int SerialPort;
    bool SuccessConnect;
    int PortO;

    int PrevResponseSeq;
    int LengthData;

    int ErrorOCode;
    std::string ErrorOMsg;
    int ErrorOPriority;

    int EventOCode;
    std::string EventOMsg;
    int EventOPriority;

    int LROCode;
    std::string LROMsg;
    int LROPriority;

    int AdEventOCode;
    std::string AdEventOMsg;
    int AdEventOPriority;

    int Bill;
    int Channel;

    //WRITE ONLY

    int LoggerLevel;
    std::string LogFilePath;
    int MaxPorts;

    // --------------- INTERNAL VARIABLES --------------------//

    bool Scanning;

    std::vector<unsigned char> RESET                = {0x01};
    std::vector<unsigned char> SET_CHANNELS_ENABLE  = {0x02, 0xFF, 0xFF, 0xFF};
    std::vector<unsigned char> DISPLAY_ON           = {0x03};
    std::vector<unsigned char> DISPLAY_OFF          = {0x04};
    std::vector<unsigned char> POLL                 = {0x07};
    std::vector<unsigned char> REJECT               = {0x08};
    std::vector<unsigned char> DISABLE              = {0x09};
    std::vector<unsigned char> ENABLE               = {0x0A};
    std::vector<unsigned char> SYNC                 = {0x11};
    std::vector<unsigned char> LAST_REJECT          = {0x17};
    std::vector<unsigned char> HOLD                 = {0x18};

    std::string DEFAULTERROR = "Codigo de error no encontrado";

    bool ActSequence = false;
    bool LastRejectFlag = false;

    ErrorCodes_t ErrorC;
    ErrorCodes_t EventC;
    ErrorCodes_t AdEventC;
    ErrorCodes_t LRCode;
    Bills_t BillC;

    std::shared_ptr<spdlog::logger> logger;
    
    static Bills_t Bills[] = {
        {0,0},
        {1,1000},
        {2,2000},
        {3,5000},
        {4,10000},
        {5,20000},
        {6,50000},
        {7,100000},
    };

    static ErrorCodes_t ErrorCodes[] = {
        {240,"OK",0},
        {242,"COMMAND NOT KNOWN",1},
        {243,"WRONG NO PARAMETERS",1},
        {244,"PARAMETER OUT OF RANGE",1},
        {245,"COMMAND CANNOT BE PROCESSED",2},
        {246,"SOFTWARE ERROR",1},
        {248,"FAIL",1},
        {250,"KEY NOT SET",1},
    };

    static ErrorCodes_t EventCodes[] = {
        //No additional params
        {240,"OK",0},
        //1 additional params
        {181,"CHANNELS_DISABLED",1},
        {182,"INITIALIZING",1},
        {204,"STACKING",0},
        {231,"STACKER_FULL",1},
        {232,"DISABLED",2},
        {233,"UNSAFE JAM",1},
        {234,"SAFE JAM",1},
        {235,"STACKED",0},
        {236,"REJECTED",0},
        {237,"REJECTING",0},
        {241,"SLAVE_RESET",1},
        //2 additional params
        {225,"CLEARED_FROM_FRONT",1},
        {226,"CLEARED_TO_CASH_BOX",1},
        {230,"FRAUD ATTEMPT",1},
        {239,"READ",0},
        //3 additional params
        {238,"CREDIT",0},
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

    static ErrorCodes_t ErrorCodesExComm[] = {
        {-5,"[EC] Timeout, acceptor not responding",2},
        {-4,"[EC] Writing successful but cannot read",1},
        {-3,"[EC] Writting error",1},
        {-2,"[] Function EC/HR/HC was not executed",2},
        {-1,"[HR] Length of data is wrong",1},
        { 0,"[HC] No news is good news",0},
        { 1,"[HC] Response code is different of OK",1},
        { 2,"[HR] Response was reviewed previosly",3},
        { 3,"[HR] Response was received shifted",2},
        { 4,"[EC] Reading length is too short, sleep time is too short",2},        
    };

    static ErrorCodes_t LastRejectCodes[] = {
        {0,"Note accepted",0},
        {1,"Note length incorrect",3},
        {2,"Reject reason 2",3},
        {3,"Reject reason 3",3},
        {4,"Reject reason 4",3},
        {5,"Reject reason 5",3},
        {6,"Channel inhibited",3},
        {7,"Second note inserted",2},
        {8,"Reject reason 8",3},
        {9,"Note recognised in more than one channel",1},
        {10,"Reject reason 10",3},
        {11,"Note too long",3},
        {12,"Reject reason 12",3},
        {13,"Mechanism slow/stalled",1},
        {14,"Strimming attempt detected",1},
        {15,"Fraud channel reject",1},
        {16,"No notes inserted",3},
        {17,"Peak detect fail",2},
        {18,"Twisted note detected",2},
        {19,"Escrow time-out",3},
        {20,"Bar code scan fail",2},
        {21,"Rear sensor 2 fail",1},
        {22,"Slot fail 1",1},
        {23,"Slot fail 2",1},
        {24,"Lens over-sample",2},
        {25,"Width detect fail",2},
        {26,"Short note detected",2},
        {27,"Note payout",3},
        {28,"Unable to stack note",1},
    };

    // --------------- CONSTRUCTOR FUNCTIONS --------------------//

    NV10Class::NV10Class(){
        
        SerialPort = 0;
        SuccessConnect = false;
        PortO = 0;
        
        PrevResponseSeq = 0;
        LengthData = 0;

        ErrorOCode = 0;
        ErrorOMsg = DEFAULTERROR;
        ErrorOPriority = 0;

        EventOCode = 0;
        EventOMsg = DEFAULTERROR;
        EventOPriority = 0;

        AdEventOCode = 0;
        AdEventOMsg = DEFAULTERROR;
        AdEventOPriority = 0;

        LROCode = 0;
        LROMsg = DEFAULTERROR;
        LROPriority = 0;

        Bill = 0;
        Channel = 0;
    }

    NV10Class::~NV10Class(){}

    // --------------- LOGGER FUNCTIONS --------------------//

    SpdlogLevels_t NV10Class::SearchSpdlogLevel(int Code){
        SpdlogLevels_t SPLvl;
        SPLvl.Message = "SpdlogLvl not found!!!"; 

        for (long unsigned int i = 0; i < sizeof(SpdlogLvl)/sizeof(SpdlogLvl[0]); i++) {
            if(SpdlogLvl[i].Code == Code) {
                SPLvl.Message = SpdlogLvl[i].Message;
                break;
            }
        }
        
        SPLvl.Code = Code;
        return SPLvl;
    }

    void NV10Class::SetSpdlogLevel(){
        spdlog::set_level(static_cast<spdlog::level::level_enum>(LoggerLevel)); // Set global log level
        //logger->set_level(spdlog::level::debug);
    }

    // --------------- SEARCH FUNCTIONS --------------------//

    ErrorCodes_t NV10Class::SearchErrorCodeExComm (int Code){
        
        ErrorCodes_t CodeMsg;
        
        CodeMsg.Message = "ErrorCode not found!!!";
        CodeMsg.Priority = 1;
        
        for (long unsigned int i = 0; i < sizeof(ErrorCodesExComm)/sizeof(ErrorCodesExComm[0]); i++) {
            if(ErrorCodesExComm[i].Code == Code) {
                CodeMsg.Message = ErrorCodesExComm[i].Message;
                CodeMsg.Priority = ErrorCodesExComm[i].Priority;
                break;
            }
        }
        
        CodeMsg.Code = Code;

        return CodeMsg;
    }

    Bills_t NV10Class::SearchBill (int Channel){
        
        Bills_t ChannelBill;
        
        ChannelBill.Bill = 0;
        
        for (long unsigned int i = 0; i < sizeof(Bills)/sizeof(Bills[0]); i++) {
            if(Bills[i].Channel == Channel) {
                ChannelBill.Bill = Bills[i].Bill;
                break;
            }
        }
        
        ChannelBill.Channel = Channel;

        return ChannelBill;
    }

    ErrorCodes_t NV10Class::SearchErrorCodes (int Code){
        
        ErrorCodes_t CodeMsg;
        
        CodeMsg.Message = "ErrorCode not found!!!";
        CodeMsg.Priority = 1;
        
        for (long unsigned int i = 0; i < sizeof(ErrorCodes)/sizeof(ErrorCodes[0]); i++) {
            if(ErrorCodes[i].Code == Code) {
                CodeMsg.Message = ErrorCodes[i].Message;
                CodeMsg.Priority = ErrorCodes[i].Priority;
                break;
            }
        }
        
        CodeMsg.Code = Code;

        return CodeMsg;
    }

    ErrorCodes_t NV10Class::SearchEventCodes (int Code){
        
        ErrorCodes_t CodeMsg;
        
        CodeMsg.Message = "EventCode not found!!!";
        CodeMsg.Priority = 1;
        
        for (long unsigned int i = 0; i < sizeof(EventCodes)/sizeof(EventCodes[0]); i++) {
            if(EventCodes[i].Code == Code) {
                CodeMsg.Message = EventCodes[i].Message;
                CodeMsg.Priority = EventCodes[i].Priority;
                break;
            }
        }
        
        CodeMsg.Code = Code;

        return CodeMsg;
    }

    ErrorCodes_t NV10Class::SearchLastReject (int Code){
        
        ErrorCodes_t CodeMsg;
        
        CodeMsg.Message = "LRC not found!!!";
        CodeMsg.Priority = 1;
        
        for (long unsigned int i = 0; i < sizeof(LastRejectCodes)/sizeof(LastRejectCodes[0]); i++) {
            if(LastRejectCodes[i].Code == Code) {
                CodeMsg.Message = LastRejectCodes[i].Message;
                CodeMsg.Priority = LastRejectCodes[i].Priority;
                break;
            }
        }
        
        CodeMsg.Code = Code;

        return CodeMsg;
    }

    // --------------- STATES OF MACHINE STATE (FUNCTIONS) --------------------//

    int NV10Class::StIdle() {

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

    int NV10Class::StConnect() {

        logger->info("[E1:STCONNECT] Scanning ports");

        PortO = ScanPorts();
        if (PortO >= 0){
            logger->info("[E1:STCONNECT] Port was found in /dev/ttyACM{0:d}",PortO);
            return 0;
        }
        else {
            logger->info("[E1:STCONNECT] Port was NOT found ......");
            return 1;
        }
    }

    int NV10Class::StDisable() {

        int Response = 1;

        logger->debug("[E2:STDISABLE] Running Disable");

        Response = Disable();

        if ((Response != 0)&(Response != 2)){
            logger->error("[E2:STDISABLE] Failed to run Disable");
            return 1;
        }

        logger->debug("[E2:STDISABLE] Running DisplayOff");

        Response = DisplayOff();

        if (Response != 0){
            logger->error("[E2:STDISABLE] Failed to run DisplayOff");
            return 1;
        }

        return 0;
    }

    int NV10Class::StEnable() {

        int Response = 1;
        
        Bill = 0;
        Channel = 0;

        logger->debug("[E3:STENABLE] Running Sync");

        Response = Sync();

        if (Response != 0){
            logger->error("[E3:STENABLE] Failed to run Sync");
            return 1;
        }

        logger->debug("[E3:STENABLE] Running DisplayOn");

        Response = DisplayOn();

        if (Response != 0){
            logger->error("[E3:STENABLE] Failed to run DisplayOn");
            return 1;
        }

        logger->debug("[E3:STENABLE] Running SetChannels");

        Response = SetChannels();

        if (Response != 0){
            logger->error("[E3:STENABLE] Failed to run SetChannels");
            return 1;
        }

        logger->debug("[E3:STENABLE] Running Enable");

        Response = Enable();

        if (Response != 0){
            logger->error("[E3:STENABLE] Failed to run Enable");
            return 1;
        }

        logger->debug("[E3:STENABLE] Running LastReject");

        Response = LastReject();

        if (Response != 0){
            logger->error("[E3:STENABLE] Failed to run LastReject");
            return 1;
        }

        return 0;
    }

    int NV10Class::StPolling() {

        int Response = 1;

        logger->debug("[E4:STPOLLING] Running Poll");

        Response = Poll();

        if (Response == 1){
            logger->error("[E4:STPOLLING] Failed to run Poll");
            return 1;
        }
        else if (Response == 2){
            logger->debug("[E4:STPOLLING] Response was viewed before");
            return 2;
        }

        return 0;
    }

    int NV10Class::StCheck() {
        
        int Response = 1;

        logger->debug("[E5:STCHECK] Running LastReject");

        Response = LastReject();

        if (Response != 0){
            logger->error("[E5:STCHECK] Failed to run LastReject");
            return 1;
        }

        logger->debug("[E5:STCHECK] Running Poll");

        Response = Poll();

        if ((Response != 0)&(Response != 2)){
            logger->error("[E5:STCHECK] Failed to run Poll");
            return 1;
        }

        return 0;
    }

    int NV10Class::StError() {

        int Response = 1;

        Response = Sync();

        if (Response != 0){
            logger->error("[EE:STERROR] Failed to run Sync");
            return 1;
        }

        return 0;
    }

    // --------------- MAIN FUNCTIONS --------------------//

    // Función para inicializar el logger
    void NV10Class::InitLogger(const std::string& Path) {
        // Crear el daily_logger y asignarlo a la variable logger
        logger = spdlog::daily_logger_mt("ValidatorNV10", Path, 23, 59);
    }

    //Connects to port /dev/ttyACM% where % is the port number (Port)
    int NV10Class::ConnectSerial(int Port){

        if (Port<0){
            logger->error("[ConnectSerial] Invalid port!");
            return 1;
        }
        else {
            logger->debug("[ConnectSerial] Connecting to /dev/ttyACM{0:d} port",Port);
            char DeviceName [50];
            sprintf (DeviceName,"/dev/ttyACM%d",Port);
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

                logger->debug("[ConnectSerial] Successfully connected to /dev/ttyACM{0:d}",Port);
                SuccessConnect = true;
                return 0;
            }
            else {
                logger->debug("[ConnectSerial] Could no connect to /dev/ttyACM{0:d}",Port);
                return 4;
            }
        }
    }
    
    //Scan all /dev/ttyACM ports from 0 to 98
    int NV10Class::ScanPorts(){
        int Port = -1;
        int Response = -1;

        for (int i = 1; i < MaxPorts; i++)
        {   
            logger->debug("[ScanPorts] Trying connection to /dev/ttyACM{0:d}",i-1);
            Response = ConnectSerial(i-1);

            if (Response==0)
            {   
                logger->debug("[ScanPorts] Connection successfull");
                Response = -1;
                Scanning = true;
                logger->debug("[ScanPorts] Sending simple poll Command (Checking connection)");

                Response = Sync();

                if (Response == 0){
                    Port = i-1;
                    logger->debug("[ScanPorts] Validator NV10 found in port /dev/ttyACM{0:d}",Port);
                    i = MaxPorts;
                    Scanning = false;
                    return Port;
                }
                else {
                    logger->warn("[ScanPorts] Error in writing/reading or Validator NV10 is NOT connected to /dev/ttyACM{0:d} port",i-1);
                }

                logger->debug("[ScanPorts] Clossing connection in /dev/ttyACM{0:d}",i-1);
                close(SerialPort);            
            }
        }
        Scanning = false;
        logger->error("[ScanPorts] Bill validator was not found in any port!");
        return Port;
    }

    int NV10Class::GetSeq() {

        ActSequence = !ActSequence;
        unsigned int Seq = 0x00;

        if (ActSequence){
            Seq = 0x80;
        }
        return Seq;
    }

    std::vector<unsigned char> NV10Class::CalcCRC(std::vector<unsigned char> Comm) {

        unsigned int Seed = 0xffff;
        unsigned int Poly = 0x8005;
        unsigned int Crc = Seed;

        for(auto Cmd: Comm){
            Crc ^= static_cast<unsigned int>(Cmd) << 8;
            for(int j = 0; j < 8; j++){
                if(Crc & 0x8000){
                    Crc = ((Crc << 1) & 0xffff) ^ Poly;
                }else{
                    Crc <<= 1;
                }
            }
        }

        std::vector<unsigned char> CrcResult(2);
        CrcResult[0] = static_cast<unsigned char>(Crc & 0xff);
        CrcResult[1] = static_cast<unsigned char>((Crc >> 8) & 0xff);

        return CrcResult;
    }

    std::vector<unsigned char> NV10Class::BuildCmd(std::vector<unsigned char> Comm){

        const unsigned char BYTE_START = 0x7F;
        std::vector<unsigned char> Cmd;

        Cmd.push_back(BYTE_START);
        int seq = GetSeq();
        Cmd.push_back(seq);
        Cmd.push_back(Comm.size());
        Cmd.insert(Cmd.end(), Comm.begin(), Comm.end());
        
        std::vector<unsigned char> CmdCrc = Cmd;
        CmdCrc.erase(CmdCrc.begin());

        std::vector<unsigned char> Crc = CalcCRC(CmdCrc);
        Cmd.insert(Cmd.end(), Crc.begin(), Crc.end());
        return Cmd;
    }

    int NV10Class::SendingCommand(std::vector<unsigned char> Comm){

        int Response = 2;
        int Res = 1;

        std::vector<unsigned char> Cmd = BuildCmd(Comm);
        Response = ExecuteCommand(Cmd);
        
        ErrorCodes_t Err;
        Err = SearchErrorCodeExComm(Response);

        if (Err.Priority == 0){
            //logger->trace("[SendingCommand] Everything is OK");
            Res = 0;
        }
        else if (Err.Priority == 1){
            logger->debug("[SendingCommand] Fatal error with comand");
            Res = -1;
        }
        else if (Err.Priority == 3){
            logger->debug("[SendingCommand] Command was sent before");
            Res = -2;
        }
        else {
            logger->debug("[SendingCommand] Repeat command");
            Res = 1;
        }

        if (Res != 0){
            logger->debug("[SendingCommand] Execute command returns: {0:d} with message: {1}",Err.Code,Err.Message);
        }
        return Res;
    }

    int NV10Class::ExecuteCommand(std::vector<unsigned char> Comm){
        int Wrlen = -1;
        int Rdlen = -1;
        int Res = -2;
        
        int Xlen = Comm.size();

        //logger->trace("[ExecuteCommand] Writting command");
        Wrlen = write(SerialPort, &Comm[0], Xlen);

        if (Wrlen!=Xlen){
            logger->warn("[ExecuteCommand] Writting error, length expect/received: {0:d}/{1:d} Error: {2}",Xlen,Wrlen,strerror(errno));
            Res = -3;
        }
        else {
            //logger->trace("[ExecuteCommand] Length expected is the same: {0:d}",Wrlen);
            
            std::vector<unsigned char> Buffer(30);
            
            usleep(200000);

            //logger->trace("[ExecuteCommand] Reading response");
            Rdlen = read(SerialPort,&Buffer[0],Buffer.size());

            if (Rdlen > 0){

                if (Rdlen >= 6){
                    //logger->trace("[ExecuteCommand] Reading length greater or equal than {0}, handling response... ");
                    Res = HandleResponse(Buffer);
                }
                else {
                    logger->debug("[ExecuteCommand] Reading partial length: {0:d}",Rdlen);
                    int i = 0;
                    for(auto t: Buffer){
                        if(i<Buffer.size()){
                            logger->error("[ExecuteCommand] Partial data{0}: {1}",i,t);
                        }
                        i++;
                    }
                    logger->warn("[ExecuteCommand] Reading length less than 6, very little waiting time ");
                    Res = 4;
                }
            }
            else if (Rdlen < 0){
                logger->warn("[ExecuteCommand] Reading error, length expect: {0:d} Error: {2}",Rdlen,strerror(errno));
                Res = -4;
            }
            else {
                logger->warn("[ExecuteCommand] Not responding, timeout!");
                Res = -5;
            }
        }

        if ((Res!=0)&(Res!=1)){
            ioctl(SerialPort, TCIOFLUSH, 2);
        }

        return Res;
    }

    int NV10Class::HandleResponse(std::vector<unsigned char> Response){

        int Res = -2;
        int StartOfTrame = Response[0];
        int ResponseSequence = Response[1];

        LengthData = 0;
        
        if ((StartOfTrame == 127) & (PrevResponseSeq != ResponseSequence)){
            PrevResponseSeq = ResponseSequence;
            LengthData = Response[2];
            if (LengthData >= 1){
                Res = HandleCode(Response);
                if(LengthData >= 2){
                    if(LastRejectFlag){
                        logger->trace("[HandleResponse] HandleLRC detected");
                        HandleLRC(Response);
                    }
                    else{
                        logger->trace("[HandleResponse] HandleEvent detected");
                        HandleEvent(Response);
                    }
                    if(LengthData >= 3){
                        logger->trace("[HandleResponse] HandleChannel detected");
                        HandleChannel(Response);
                    }
                }
            }
            else {
                logger->error("[HandleResponse] Length of data is 0 or is wrong: {0}",LengthData);
                Res = -1;
            }
        }
        else if (StartOfTrame == 127){
            logger->warn("[HandleResponse] Response decodificated previosly");
            Res = 2;
        }
        else {
            logger->error("[HandleResponse] Bad response");
            int i = 0;
            for(auto t: Response){
                if(i<Response.size()){
                    logger->error("[HandleResponse] Data{0}: {1}",i,t);
                }
                i++;
            }
            Res = 3;
        }
        return Res;
    }

    int NV10Class::HandleCode(std::vector<unsigned char> Response){
        
        int Res = -2;
        int Code = Response[3];

        ErrorC = SearchErrorCodes(Code);
        ErrorOCode = ErrorC.Code;
        ErrorOMsg = ErrorC.Message;
        ErrorOPriority = ErrorC.Priority;

        if (Code == 240){
            Res = 0;
        }
        else {
            logger->debug("[HandleCode] Code: {0} message: {1}",ErrorC.Code,ErrorC.Message);
            logger->warn("[HandleCode] Response code is not OK");
            Res = 1;
        }
        return Res;
    }

    int NV10Class::HandleEvent(std::vector<unsigned char> Response){

        int Res = -2;

        int Event = Response[4];
        EventC = SearchEventCodes(Event);
        EventOCode = EventC.Code;
        EventOMsg = EventC.Message;
        EventOPriority = EventC.Priority;
        logger->debug("[HandleEvent] Event code: {0} message: {1}",EventC.Code,EventC.Message);

        if (LengthData == 4){
            int AdEvent = Response[6];
            AdEventC = SearchEventCodes(AdEvent);
            AdEventOCode = AdEventC.Code;
            AdEventOMsg = AdEventC.Message;
            AdEventOPriority  = AdEventC.Priority;
            logger->debug("[HandleEvent] Additional event code: {0} message: {1}",AdEventC.Code,AdEventC.Message);
        }      

        if (EventC.Message == "EventCode not found!!!"){
            logger->error("[HandleEvent] Event code not found");
            Res = 1;
        }
        else {
            Res = 0;
        }

        return Res;
    }

    int NV10Class::HandleLRC(std::vector<unsigned char> Response){

        int Res = -2;
        int LRC = Response[4];

        LRCode = SearchLastReject(LRC);
        LROCode = LRCode.Code;
        LROMsg = LRCode.Message;

        logger->debug("[HandleLRC] Last reject code: {0} message: {1}",LRCode.Code,LRCode.Message); 

        if (LRCode.Message == "LRC not found!!!"){
            logger->error("[HandleLRC] LRC not found");
            Res = 1;
        }
        else {
            Res = 0;
        }

        return Res;
    }

    int NV10Class::HandleChannel(std::vector<unsigned char> Response){

        int Res = -2;
        Channel = Response[5];

        BillC = SearchBill(Channel);

        Bill = BillC.Bill;

        logger->debug("[HandleChannel] Bill detected: {0} Channel: {1}",BillC.Bill,BillC.Channel);

        if (BillC.Bill == 0){
            logger->debug("[HandleChannel] Bill not found");
            Res = 1;
        }
        else {
            Res = 0;
        }

        return Res;
    } 

    int NV10Class::DisplayOn(){
        int Response  = -1;

        logger->debug("[DisplayOn] Enabling display");
        Response = SendingCommand(DISPLAY_ON);

        if ((Response != 0)&(Response != -2)){
            logger->error("[DisplayOn] Diplay could not be turned on");
            return 1;
        }

        return 0;
    }

    int NV10Class::DisplayOff(){
        int Response  = -1;

        logger->debug("[DisplayOff] Disabling display");
        Response = SendingCommand(DISPLAY_OFF);

        if ((Response != 0)&(Response != -2)){
            logger->error("[DisplayOff] Diplay could not be turned off");
            return 1;
        }

        return 0;
    }

    int NV10Class::SetChannels(){
        int Response  = -1;
        
        logger->debug("[SetChannels] Setting internal channels");
        Response = SendingCommand(SET_CHANNELS_ENABLE);

        if ((Response != 0)&(Response != -2)){
            logger->error("[SetChannels] Channels could not be setted");
            return 1;
        }

        return 0;
    }

    int NV10Class::Enable(){
        int Response  = -1;
        
        logger->debug("[Enable] Enabling selected channels");
        Response = SendingCommand(ENABLE);

        if ((Response != 0)&(Response != -2)){
            logger->error("[Enable] Selected channels could not be enabled");
            return 1;
        }

        return 0;
    }

    int NV10Class::Disable(){
        int Response  = -1;
        
        logger->debug("[Enable] Disabling selected channels");
        Response = SendingCommand(DISABLE);

        if ((Response != 0)&(Response != -2)){
            logger->error("[Enable] Selected channels could not be disabled");
            return 1;
        }

        return 0;
    }

    int NV10Class::Poll(){
        int Response  = -1;
        
        //logger->debug("[Poll] Polling");
        Response = SendingCommand(POLL);

        if (Response == -2){
            logger->debug("[Poll] Response was viewed before");
            return 2;
        }
        else if (Response != 0){
            logger->error("[Poll] Failed to poll");
            return 1;
        }

        return 0;
    }

    int NV10Class::LastReject(){
        int Response  = -1;
        LastRejectFlag = true;
        logger->debug("[LastReject] Checking last reject code");
        Response = SendingCommand(LAST_REJECT);
        
        if ((Response != 0)&(Response != -2)){
            logger->error("[LastReject] Could not verify last reject code");
            return 1;
        }
        LastRejectFlag = false;
        return 0;
    }

    int NV10Class::Sync(){

        int Response  = -1;
        ActSequence = false;
        logger->debug("[Sync] Synchronizing with the bill acceptor");
        Response = SendingCommand(SYNC);

        if ((Response != 0)&(Response != -2)){
            logger->error("[Sync] Failed to sync");
            return 1;
        }

        return 0;
    }

    int NV10Class::Hold(){
        int Response  = -1;
        
        logger->debug("[Hold] Holding the last bill");
        Response = SendingCommand(HOLD);

        if ((Response != 0)&(Response != -2)){
            logger->error("[Hold] Failed to hold");
            return 1;
        }

        return 0;
    }

    int NV10Class::Reject(){
        int Response  = -1;
        
        logger->debug("[Reject] Rejecting the last bill");
        Response = SendingCommand(REJECT);

        if ((Response != 0)&(Response != -2)){
            logger->error("[Reject] Failed to reject");
            return 1;
        }

        return 0;
    }
}