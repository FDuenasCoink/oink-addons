import { CommandResponse, DeviceStatus } from "./interface"

export interface NV10Obj {
  connect(): CommandResponse;
  checkDevice(): CommandResponse;
  startReader(): CommandResponse;
  getBill(): Bill;
  modifyChannels(inhibitMask: number): CommandResponse;
  stopReader(): CommandResponse;
  reject(): CommandResponse;
  testStatus(): DeviceStatus;
}

export interface NV10Options {
  maximumPorts: number;
  logPath: string;
  logLevel: number;
}

export interface Bill extends CommandResponse {
  bill: number;
}

