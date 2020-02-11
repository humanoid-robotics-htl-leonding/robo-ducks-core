import { Component, OnInit, Input } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';
import { ActivatedRoute } from '@angular/router';
import { NaoService } from 'src/app/service/nao.service';
import { Tab } from 'src/app/model/tab';
import { DebugMessage } from 'src/app/model/debug-message';
import { Sink, write_str, write_u8, write_u16, write_u32 } from 'ts-binary';
import { DebugMessageType } from 'src/app/model/message-type.enum';
import { NaoConnector } from 'src/app/model/nao-connector';

@Component({
  selector: 'app-nao',
  templateUrl: './nao.component.html',
  styleUrls: ['./nao.component.scss']
})
export class NaoComponent implements OnInit {

  @Input() connector: NaoConnector;
  toggleForm = false;
  message = '';
  messages: DebugMessage[] = [];

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

}
