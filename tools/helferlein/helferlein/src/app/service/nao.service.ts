import { Injectable } from '@angular/core';
import { Tab } from '../model/tab';
import { BinaryBuffer } from 'ts-binary-serializer/dist/src/BinaryBuffer';
declare var electron: any;
const net = electron.remote.require('net');
const MAX_BUFF_LENGTH = 0xfffffff;

@Injectable({
  providedIn: 'root'
})
export class NaoService {

  tabs: Tab[] = [];

  client = null;
  buff = [];
  buffLen = 0;
  dataHandler;

  constructor() {
    this.dataHandler = () => {
      console.log('buffer', this.buff);
      console.log('buffer length', this.buffLen);
    };
  }

  addClient() {
    this.tabs.push({
      id: this.tabs.length,
      address: '',
      port: null,
      status: 'New',
      client: null,
      onReceiveData: null
    });
    console.log('Added Client');
    console.log(this.tabs);
  }

  setUpClient(id: number, addr, port, onReceiveDataCallback = this.onReceiveData) {
    console.log(onReceiveDataCallback.toString());
    if (this.tabs.some(t => t.id == id)) {
      const index = this.tabs.findIndex(t => t.id == id);
      if (this.tabs[index].client && !this.tabs[index].client.destroyed) {
        this.tabs[index].client.destroy();
      }
      this.tabs[index].address = addr;
      this.tabs[index].port = port;
      this.tabs[index].onReceiveData = onReceiveDataCallback;
    } else {
      this.tabs.push({
        id,
        address: addr,
        port,
        status: 'New',
        client: null,
        onReceiveData: onReceiveDataCallback
      });
      console.log(onReceiveDataCallback.toString())
    }
    console.log('Setup');
    console.log(this.tabs);
  }

  onReceiveData(buff) {
    try {
      console.log('Old OnReceiveData');
      if (this.buffLen + buff.length > MAX_BUFF_LENGTH) {
        this.buff.shift();
      }
      this.buff.push(buff);
      this.buffLen += buff.length;

      if (this.dataHandler) {
        this.dataHandler(this.buff, this.buffLen);
      }
    } catch (e) {
      this.buff = [];
      this.buffLen = 0;
      console.log(`Error on onReceiveData ${e.message}`);
    }
  }

  connect(id: number) {
    console.log('Connect');
    const i = this.tabs.findIndex(t => t.id == id);
    if (!this.tabs[i].client) {
      this.tabs[i].client = new net.Socket({});
      console.log(this.tabs[i].client);
      console.log(this.tabs);
      this.tabs[i].client.on('end', () => {
        console.log('Connection ended');
        this.tabs[i].status = 'Disconnected';
      });
      this.tabs[i].client.on('data', (chunk) => {
        try {
          this.tabs[i].onReceiveData(chunk);
        } catch (e) {
          console.log('onDataError', e.message);
        }
      });
      if (!this.tabs[i].client.connecting) {
        console.log('Connecting...');
        this.tabs[i].client.connect(this.tabs[i].port, this.tabs[i].address);
      }
    }

    if (this.tabs[i].client.destroyed) {
      this.tabs[i].client.connect(this.tabs[i].port, this.tabs[i].address);
    }
    this.tabs[i].client.setKeepAlive(true);
    console.log('Connected ');
    console.log(this.tabs[i].client.remoteAddress);
  }

  send(id: number, requestObj) {
    console.log('RequestObj');
    console.log(requestObj);
    console.log(this.tabs);
    const client = this.tabs.find(t => t.id==id).client;
    console.log(client);
    console.log('client-destroyed: ' + client.destroyed);
    if (!client || client.destroyed) {
      console.log('send->!this-client');
      this.connect(id);
    }

    if (requestObj && requestObj.length > 0) {
      console.log('sending...');
      console.log('connecting: ' + client.connecting);
      console.log('destroyed: ' + client.destroyed);
      console.log(requestObj);
      console.log(client.write(requestObj));
    }
  }

  destroyClient(id: number) {
    if (this.tabs.some(t => t.id == id)) {
      const client = this.tabs.find(t => t.id == id).client;
      if(client && !client.destroyed){
        client.end();
      }
      this.tabs.splice(this.tabs.findIndex(t => t.id == id), 1);
    }
  }
}
