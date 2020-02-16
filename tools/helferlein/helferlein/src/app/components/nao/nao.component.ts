import { Component, OnInit, Input, EventEmitter, Output } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';
import { ActivatedRoute } from '@angular/router';
import { NaoService } from 'src/app/service/nao.service';
import { Tab } from 'src/app/model/tab';
import { DebugMessage } from 'src/app/model/debug-message';
import { Sink, write_str, write_u8, write_u16, write_u32 } from 'ts-binary';
import { DebugMessageType } from 'src/app/model/message-type.enum';
import { NaoConnector } from 'src/app/model/nao-connector';
import { RawCardComponent } from '../elements/raw-card/raw-card.component';
import { state } from '@angular/animations';
import { NaoElement } from 'src/app/model/nao-element';
import { ComponentPortal } from '@angular/cdk/portal';
import { NaoElementComponent } from '../elements/nao-element/nao-element.component';
import { NaoImageComponent } from '../elements/nao-image/nao-image.component';
import { NaoGridElement } from 'src/app/model/nao-grid-element';
// const menu = require('electron').remote.Menu;

@Component({
  selector: 'app-nao',
  templateUrl: './nao.component.html',
  styleUrls: ['./nao.component.scss'],
})
export class NaoComponent implements OnInit {

  @Input() connector: NaoConnector;
  @Output() addTab = new EventEmitter();
  @Output() closeTab = new EventEmitter();
  toggleForm = false;
  message = '';
  elements: NaoGridElement[] = [];

  constructor(public dialog: MatDialog, private naoService: NaoService) {
  }

  ngOnInit() {
  }

  connectToNao(){
    this.connector.connect();
  }

  toggleFormField(){
    this.toggleForm = !this.toggleForm;
  }

  send(){
    this.connector.sendString(DebugMessageType.DM_REQUEST_LIST, this.message);
  }

  doAction(event){
    console.log(event);
  }

  addElement(type: string){

  }

  getComponentPortal(element: NaoElement){
    switch(element.type){
      case 'image': {
        return new ComponentPortal(NaoImageComponent);
      }
    }
  }

}
