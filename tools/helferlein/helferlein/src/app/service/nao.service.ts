import { Injectable } from '@angular/core';
import { Tab } from '../model/tab';
import { BinaryBuffer } from 'ts-binary-serializer/dist/src/BinaryBuffer';
import { NaoConnector } from '../model/nao-connector';
declare var electron: any;
const net = electron.remote.require('net');
const MAX_BUFF_LENGTH = 0xfffffff;

@Injectable({
  providedIn: 'root'
})
export class NaoService {

  tabs: Tab[] = [];

  addTab(){
    this.tabs.push({
      id: this.tabs.length,
      connector: new NaoConnector()
    });
  }

  removeTab(id: number){
    this.tabs[this.tabs.findIndex(t => t.id == id)].connector.disconnect();
    this.tabs.splice(this.tabs.findIndex(t => t.id == id),1);
  }

}
