import { CommandResponse, DeviceStatus, UnsubscribeFunc } from "./interface";
import { CoinResult, LostCoins } from "./interface";

export interface IAzkoyen {
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
  onCoin(callback: (coin: CoinResult) => void): UnsubscribeFunc;
}

export interface AzkoyenOptions {
  warnToCritical: number;
  maxCritical: number;
  maximumPorts: number;
  logLevel: number;
  logPath: string;
}