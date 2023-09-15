export interface CommandResponse {
  statusCode: number;
  message: string;
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

export type UnsubscribeFunc = () => void;