import { Socket } from 'net';
import { DebugMessage } from './debug-message';
import { DebugMessageType } from './message-type.enum';
import { Sink } from 'ts-binary';
import { NaoService } from '../service/nao.service';
import { ChangeDetectorRef, Output, EventEmitter, Injectable } from '@angular/core';
declare var electron: any;
const net = electron.remote.require('net');

export class NaoConnector {

  address = '';
  port = 12441;
  status = 'New';
  client: Socket;
  message: DebugMessage;
  keys: any;
  onData: (chunk) => void = this.defaultOnData;
  onEnd: () => void = this.defaultOnEnd;
  onConnect: () => void = this.defaultOnConnect;
  onError: (error) => void = this.defaultOnError;

  @Output() receivedData = new EventEmitter();
  @Output() connectionEnded = new EventEmitter();
  @Output() connected = new EventEmitter();
  @Output() connectionError = new EventEmitter();

  constructor() {}

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
        console.log('Sent: ', message);
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
    if (!this.message || this.message.isCompleted()) {
      if (this.message) {
        console.log(this.message);
        this.receivedData.emit(this.message);
      }
      this.message = new DebugMessage();
    }
    if (this.message.headerIncomplete()) {
      this.message.parseHeader(chunk.subarray(0, 16));
      let saveChunk = chunk.subarray(16);
      chunk = new Uint8Array(chunk.length-16);
      chunk.set(saveChunk);
    }
    if (!this.message.isCompleted()) {
      this.message.appendMessage(chunk);
    }
    console.log('MsgLen',this.message.msg.length);
    console.log('Msg',this.message);
  }

  defaultOnEnd() {
    console.log('Connection to ' + this.address + ' ended');
    this.status = this.address + ' [Disconnected]';
  }

  defaultOnConnect() {
    console.log('Connected to ' + this.address);
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
