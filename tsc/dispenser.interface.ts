import { CommandResponse, DeviceStatus } from "./interface"

export interface IDispenser {
  connect(): CommandResponse;
  checkDevice(): CommandResponse;
  dispenseCard(): CommandResponse;
  recycleCard(): CommandResponse;
  endProcess(): CommandResponse;
  getDispenserFlags(): DispenserFlags;
  testStatus(): DeviceStatus;
}

export interface DispenserOptions {
  maxInitAttempts: number;
  shortTime: number;
  longTime: number;
  maximumPorts: number;
  logPath: string;
  logLevel: string;
}

export interface DispenserFlags {
  rficCardInG: boolean;
  recyclingBoxF: boolean;
  cardInG: boolean;
  cardsInD: boolean;
  dispenserF: boolean;
}