import { Socket } from 'net';
import { DebugMessage } from './debug-message';
import { DebugMessageType } from './message-type.enum';
import { Sink } from 'ts-binary';
import { NaoService } from '../service/nao.service';
import { ChangeDetectorRef, Output, EventEmitter } from '@angular/core';
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

  @Output() receivedData = new EventEmitter();
  @Output() connectionEnded = new EventEmitter();
  @Output() connected = new EventEmitter();
  @Output() connectionError = new EventEmitter();

  constructor(){}

  connect() {
    if (!this.client) {
      console.log('Connecting to ' + this.address + '...');
      this.status = this.address + ' [Connecting]';
      this.client = new net.Socket({});

      this.client.on('end', () => {
        this.onEnd();
        this.connectionEnded.emit();
      });
      this.client.on('data', (chunk) => {
        this.onData(chunk);
      });
      this.client.on('connect', () => {
        this.onConnect();
        this.connected.emit();
      });
      this.client.on('error', (error) => {
        this.onError(error);
        this.connectionError.emit();
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
        console.log('Sent: ',message);
        this.client.write(message);
      }
    }
  }

  sendString(type: DebugMessageType, message: string) {
    const debugMessage = new DebugMessage(type, message);
    const sink: Sink = debugMessage.toSink();
    this.send(new Uint8Array(sink.view.buffer));
  }

  /**
   * The default Callback when the Connector receives Data from a NAO.
   * Messages come in chunks (Uint8Arrays) and therefore, several chunks have to
   * be combined into one message
   * @param chunk The latest message from the NAO
   */
  defaultOnData(chunk: Uint8Array) {
    const oldLen = this.remainingChunk.length;
    this.remainingChunk = new Uint8Array(oldLen + chunk.length);  // Unused parts of the last chunk are concatinated with the new chunk
    this.remainingChunk.set(this.remainingChunk);
    this.remainingChunk.set(chunk, oldLen);
    let index = 0;
    while(index<this.remainingChunk.length) { //we want all of the chunk to be put in DebugMessages
      if (this.messages.length == 0 || this.messages[this.messages.length-1].isCompleted()) {
        if(this.messages.length>0){
          this.receivedData.emit();
        }
        this.messages.push(new DebugMessage());
      }
      const message = this.messages[this.messages.length-1];  //always put the chunk in the latest (and only incomplete) message
      if (message.headerIncomplete()) {
        if (this.remainingChunk.length - index < 16) {  //we want the whole header to be parsed at once
          let oldChunk = this.remainingChunk;
          this.remainingChunk = new Uint8Array(oldChunk.length - index);
          this.remainingChunk.set(oldChunk.slice(index));
          return; //it it isn't possible to parse a header we save the chunk and wait for the next one
        }
        message.parseHeader(this.remainingChunk.subarray(index, 16));
        index += 16;
      }
      index += message.appendMessage(this.remainingChunk.subarray(index));
    }
    this.remainingChunk = new Uint8Array(0);
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
