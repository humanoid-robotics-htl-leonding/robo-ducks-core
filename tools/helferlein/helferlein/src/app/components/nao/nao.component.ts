import { Component, OnInit, Input, EventEmitter, Output, Injector } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';
import { NaoService } from 'src/app/service/nao.service';
import { DebugMessageType } from 'src/app/model/message-type.enum';
import { NaoConnector } from 'src/app/model/nao-connector';
import { ComponentPortal } from '@angular/cdk/portal';
import { NaoElementComponent } from '../elements/nao-element/nao-element.component';
import { NaoImageComponent } from '../elements/nao-image/nao-image.component';
import { NaoGridElement } from 'src/app/model/nao-grid-element';
import { NaoMapComponent } from '../elements/nao-map/nao-map.component';
import { NaoConfigComponent } from '../elements/nao-config/nao-config.component';
import { NaoTextComponent } from '../elements/nao-text/nao-text.component';

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
  messageType: DebugMessageType;
  message = '';
  elements: NaoGridElement[] = [];

  constructor(public dialog: MatDialog, private naoService: NaoService, private injector: Injector) {
  }

  ngOnInit() {
    console.log('Nao: ',this.id);
    this.connector = this.naoService.tabs.find(t => t.id == this.id).connector;
    this.elements.push(this.getGridElement(1, 1, 5, 7));
    this.elements.push(this.getGridElement(1, 7, 4, 11));
    this.elements.push(this.getGridElement(5, 1, 7, 7));
    this.elements.push(this.getGridElement(4, 7, 7, 11));
  }

  connectToNao() {
    this.connector.connect();
    this.connector.sendString(DebugMessageType.DM_REQUEST_LIST,'');
  }

  send() {
    this.connector.sendString(this.messageType, this.message);
  }

  addElement() {

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

}
