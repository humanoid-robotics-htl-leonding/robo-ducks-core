import { Socket } from 'net';
import { DebugMessage } from './debug-message';
import { DebugMessageType } from './message-type.enum';
import { Sink } from 'ts-binary';
import { NaoService } from '../service/nao.service';
declare var electron: any;
const net = electron.remote.require('net');

export class NaoConnector {

  checksum = 0;
  address = '';
  port = 12441;
  status = 'New';
  client: Socket;
  messages: DebugMessage[] = [];
  remainingChunk: Uint8Array = new Uint8Array();
  onData: (chunk) => void = this.defaultOnData;
  onEnd: () => void = this.defaultOnEnd;
  onConnect: () => void = this.defaultOnConnect;
  onError: (error) => void = this.defaultOnError;

  connect() {
    if (!this.client) {
      console.log('Connecting to ' + this.address + '...');
      this.status = this.address + ' [Connecting]';
      this.client = new net.Socket({});

      this.client.on('end', () => {
        this.onEnd();
      });
      this.client.on('data', (chunk) => {
        this.onData(chunk);
      });
      this.client.on('connect', () => {
        this.onConnect();
      });
      this.client.on('error', (error) => {
        this.onError(error);
      });
    }

    if (this.client.destroyed || !this.client.connecting) {
      this.client.connect(this.port, this.address);
    }

    this.client.setKeepAlive(true);
  }

  reset() {
    if (this.client && !this.client.destroyed) {
      this.client.destroy();
      this.status = this.address + ' [Destroyed]';
    }
  }

  disconnect() {
    if (this.client) {
      this.client.end();
    }
  }

  send(message: Uint8Array) {
    if (this.client && !this.client.destroyed) {
      if (message && message.length >= 16) {
        this.client.write(message);
      }
    }
  }

  sendString(type: DebugMessageType, message: string) {
    const debugMessage = new DebugMessage(type, message);
    const sink: Sink = debugMessage.toSink();
    this.send(new Uint8Array(sink.view.buffer));
  }

  defaultOnData(chunk: Uint8Array) {
    console.log('Start ',this.checksum);
    //console.log('Remaining');
    //console.log(this.remainingChunk);
    const oldLen = this.remainingChunk.length;
    this.remainingChunk = new Uint8Array(oldLen + chunk.length);
    this.remainingChunk.set(this.remainingChunk);
    this.remainingChunk.set(chunk, oldLen);
    //console.log(this.remainingChunk);
    let index = 0;
    while(index<this.remainingChunk.length) {
      if (this.messages.length == 0 || this.messages[this.messages.length-1].isCompleted) {
        // if(this.messages.length >= 2){
        //   console.log('loolig');
        //   console.log(this.messages);
        //   return;
        // }
        this.messages.push(new DebugMessage());
      }
      const message = this.messages[this.messages.length-1];
      if (message.headerIncomplete()) {
        if (this.remainingChunk.length - index < 16) {
          let oldChunk = this.remainingChunk;
          this.remainingChunk = new Uint8Array(oldChunk.length - index);
          this.remainingChunk.set(oldChunk.slice(index));
          return;
        }
        message.parseHeader(this.remainingChunk.subarray(index, 16));
        index += 16;
      }
      index += message.appendMessage(this.remainingChunk.subarray(index));
    }
    this.remainingChunk = new Uint8Array(0);
    //console.log(this);
    console.log('End ', this.checksum);
    this.checksum++;
  }

  defaultOnEnd() {
    console.log('Connection to ' + this.address + ' ended');
    this.status = this.address + ' [Disconnected]';
  }

  defaultOnConnect() {
    console.log('Connected to ' + this.address);
    console.log(this);
    this.status = this.address;
  }

  defaultOnClose() {
    console.log('Connection to ' + this.address + ' closed');
    this.status = this.address + ' [Disconnected]';
  }

  defaultOnError(error) {
    console.log('Error on ' + this.address, error);
    this.status = this.address + ' [Error]';
    this.client.destroy();
  }
}
