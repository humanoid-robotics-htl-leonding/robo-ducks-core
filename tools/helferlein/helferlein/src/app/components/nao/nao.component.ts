import { Component, OnInit, Input, EventEmitter, Output, Injector } from '@angular/core';
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
import { ComponentPortal } from '@angular/cdk/portal';
import { NaoElementComponent } from '../elements/nao-element/nao-element.component';
import { NaoImageComponent } from '../elements/nao-image/nao-image.component';
import { NaoGridElement } from 'src/app/model/nao-grid-element';
import { NaoMapComponent } from '../elements/nao-map/nao-map.component';
import { NaoConfigComponent } from '../elements/nao-config/nao-config.component';
import { NaoTextComponent } from '../elements/nao-text/nao-text.component';
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

  constructor(public dialog: MatDialog, private naoService: NaoService, private injector: Injector) {
  }

  ngOnInit() {
    this.elements.push(this.getGridElement(1,1,5,7));
    this.elements.push(this.getGridElement(1,7,4,11));
    this.elements.push(this.getGridElement(5,1,7,7));
    this.elements.push(this.getGridElement(4,7,7,11));
  }

  connectToNao() {
    this.connector.connect();
  }

  toggleFormField() {
    this.toggleForm = !this.toggleForm;
  }

  send() {
    this.connector.sendString(DebugMessageType.DM_REQUEST_LIST, this.message);
  }

  doAction(event) {
    console.log(event);
  }

  addElement(type: string) {

  }

  getComponentPortal(element: NaoGridElement) {
    switch (element.type) {
      case 'image': {
        return new ComponentPortal(NaoImageComponent);
      }
      case 'map': {
        return new ComponentPortal(NaoMapComponent);
      }
      case 'config': {
        return new ComponentPortal(NaoConfigComponent);
      }
      case 'text': {
        return new ComponentPortal(NaoTextComponent);
      }
    }
  }

  getGridElement(rowStart,colStart,rowEnd,colEnd): NaoGridElement{
    return {
      id: null,
      type: null,
      rowStart,
      colStart,
      rowEnd,
      colEnd,
      component: new NaoElementComponent(this.injector)
    };
  }

}
