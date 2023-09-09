export interface ValidatorObj {
  connect(): CommandResponse;
  checkDevice(): CommandResponse;
  startReader(): CommandResponse;
  getCoin(): CoinResult;
  getLostCoins(): LostCoins;
  modifyChannels(inhibitMask1: number, inhibitMask2: number): CommandResponse;
  stopReader(): CommandResponse;
  resetDevice(): CommandResponse;
  testStatus(): DeviceStatus;
  cleanDevice(): CommandResponse;
}

export interface ValidatorOptions {
  warnToCritical: number;
  maxCritical: number;
  maximumPorts: number;
  logLevel: number;
  logPath: string;
}

export interface CommandResponse {
  statusCode: number;
  message: string;
}

export interface CoinResult extends CommandResponse {
  event: number;
  coin: number;
  remaining: number;
}

export interface LostCoins {
  "50": number;
  "100": number;
  "200": number;
  "500": number;
  "1000": number;
}

export interface DeviceStatus {
  version: string;
  device: number;
  errorType: number;
  errorCode: number;
  message: string;
  aditionalInfo: string;
  priority: number;
}