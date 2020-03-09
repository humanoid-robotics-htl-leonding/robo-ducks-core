import { Socket } from 'net';
import { NaoConnector } from './nao-connector';

export interface Tab {
  id: number;
  connector: NaoConnector;
}
