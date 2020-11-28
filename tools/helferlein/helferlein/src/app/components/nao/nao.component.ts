import { Component, OnInit, Input, EventEmitter, Output, Injector } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';
import { NaoService } from 'src/app/service/nao.service';
import { DebugMessageType } from 'src/app/model/message-type.enum';
import { NaoConnector } from 'src/app/model/nao-connector';
import { NaoElementComponent } from '../elements/nao-element/nao-element.component';
import { NaoGridElement } from 'src/app/model/nao-grid-element';
import { DebugMessage } from 'src/app/model/debug-message';
import { FormsModule } from '@angular/forms';

@Component({
  selector: 'app-nao',
  templateUrl: './nao.component.html',
  styleUrls: ['./nao.component.scss'],
})
export class NaoComponent implements OnInit {

  @Input() id: number;
  connector: NaoConnector;
  @Output() addTab = new EventEmitter();
  @Output() closeTab = new EventEmitter();
  toggleForm = false;
  messageType: DebugMessageType = DebugMessageType.DM_REQUEST_LIST;
  message = '';
  elements: NaoGridElement[] = [];
  addressSugestions = ['/tmp/simrobot/robot3/debug']

  constructor(public dialog: MatDialog, private naoService: NaoService, private injector: Injector) {
  }

  ngOnInit() {
    this.connector = this.naoService.tabs.find(t => t.id == this.id).connector;
    this.connector.receivedData.subscribe((event) => {this.onData(event)});
    this.elements.push(this.getGridElement(1, 1, 5, 7));
    this.elements.push(this.getGridElement(1, 7, 4, 11));
    this.elements.push(this.getGridElement(5, 1, 7, 7));
    this.elements.push(this.getGridElement(4, 7, 7, 11));
  }

  connectToNao() {
    this.connector.connect();
    this.connector.connected.subscribe(() => this.connector.sendString(DebugMessageType.DM_REQUEST_LIST,''));
  }

  send() {
    this.connector.sendString(this.messageType, this.message);
  }

  getGridElement(rowStart, colStart, rowEnd, colEnd): NaoGridElement {
    return {
      id: null,
      type: null,
      rowStart,
      colStart,
      rowEnd,
      colEnd,
      component: new NaoElementComponent(this.naoService, this.injector)
    };
  }

  onData(event: DebugMessage){
    console.log(event);
    if(event.msgType == DebugMessageType.DM_LIST){
      this.connector.keys = JSON.parse(event.msg).keys;
    }
  }
}
