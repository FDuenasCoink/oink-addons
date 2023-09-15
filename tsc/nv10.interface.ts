import { CommandResponse, DeviceStatus, UnsubscribeFunc } from "./interface"

export interface INV10 {
  connect(): CommandResponse;
  checkDevice(): CommandResponse;
  startReader(): CommandResponse;
  getBill(): Bill;
  modifyChannels(inhibitMask: number): CommandResponse;
  stopReader(): CommandResponse;
  reject(): CommandResponse;
  testStatus(): DeviceStatus;
  onBill(callback: (bill: Bill) => void): UnsubscribeFunc;
}

export interface NV10Options {
  maximumPorts: number;
  logPath: string;
  logLevel: number;
}

export interface Bill extends CommandResponse {
  bill: number;
}

