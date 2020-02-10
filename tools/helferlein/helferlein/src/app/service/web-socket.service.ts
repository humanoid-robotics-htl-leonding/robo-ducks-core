import { Injectable } from '@angular/core';
import { Socket } from 'socket.io';
const net = require("net");

const MAX_BUFF_LENGTH = 0xfffffff;

@Injectable({
  providedIn: 'root'
})
export class WebSocketService {

  private created: boolean = false;
  client = null;
  buff = [];
  buffLen = 0;
  dataHandler;
  addr = '';
  port = null;

  constructor() {
    this.dataHandler = () => {
      console.log('buffer',this.buff);
      console.log('buffer length', this.buffLen);
    };
  }

  setupClient(addr, port){
    if(this.client && !this.client.destroyed){
      this.client.destroy();
    }
    this.addr = addr;
    this.port = port;

  }

  onReceiveData(buff){
    try{
      if(this.buffLen + buff.length > MAX_BUFF_LENGTH){
        this.buff.shift();
      }
      this.buff.push(buff);
      this.buffLen += buff.length;

      if(this.dataHandler){
        this.dataHandler(this.buff, this.buffLen);
      }
    } catch(e){
      this.buff = [];
      this.buffLen = 0;
      console.log(`Error on onReceiveData ${e.message}`);
    }
  }

  connect(){
    if(!this.client){
      this.client = new net.Socket();
      this.client.on('data', (chunk) => {
        try{
          this.onReceiveData(chunk);
        } catch(e){
          console.log('onDataError',e.message);
        }
      });
      if(!this.client.connecting) {
        this.client.connect(this.port,this.addr);
      }
    }

    if(this.client.destroyed){
      this.client.connect(this.port, this.addr);
    }
  }

  send(requestObj){
    if(!this.client || this.client.destroyed){
      this.connect();
    }

    if(requestObj && requestObj.length > 0){
      this.client.write(requestObj);
    }
  }
}
