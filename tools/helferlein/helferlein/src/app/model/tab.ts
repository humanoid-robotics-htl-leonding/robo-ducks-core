import { Socket } from 'net';

export interface Tab {
  id: number;
  address: string;
  port: number;
  status: string;
  client: Socket;
  onReceiveData: (buff) => void;
}
