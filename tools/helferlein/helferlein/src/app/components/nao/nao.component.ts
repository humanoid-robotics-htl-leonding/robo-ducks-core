import { Component, OnInit, Input } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';
import { ActivatedRoute } from '@angular/router';
import { NaoService } from 'src/app/service/nao.service';
import { Tab } from 'src/app/model/tab';
import { DebugMessageHeader } from 'src/app/model/message';
import { Sink, write_str, write_u8, write_u16, write_u32 } from 'ts-binary';
import { DebugMessageType } from 'src/app/model/message-type.enum';

@Component({
  selector: 'app-nao',
  templateUrl: './nao.component.html',
  styleUrls: ['./nao.component.scss']
})
export class NaoComponent implements OnInit {

  @Input() id: number;
  tab: Tab;
  toggleForm = false;
  address = '';
  message = '';
  messages: DebugMessageHeader[] = [];

  constructor(public dialog: MatDialog, private naoService: NaoService) {
    this.tab = this.naoService.tabs.find(t => t.id == this.id);
  }

  ngOnInit() {
    this.tab = this.naoService.tabs.find(t => t.id == this.id);
  }

  connectToNao(){
    const clientfields = this.address.split(':');
    this.naoService.setUpClient(this.id, clientfields[0],clientfields[1],(buff) => {
      // if(this.messages.length==0 || this.messages[this.messages.length-1].isCompleted()){
      //   if(this.buff.length)
      // }

//TODO implement automated Message-Management, new message starts when old has reached length

      let header = new DebugMessageHeader(null,'',buff);
      console.log(header);
    });
    this.naoService.connect(this.id);
  }

  toggleFormField(){
    this.toggleForm = !this.toggleForm;
  }

  send(){
    let dmsgh  = new DebugMessageHeader(DebugMessageType.DM_REQUEST_LIST,'');
    let data: Sink = dmsgh.toSink();

    console.log(data.view);
    this.naoService.send(this.id,new Uint8Array(data.view.buffer));
  }

}
