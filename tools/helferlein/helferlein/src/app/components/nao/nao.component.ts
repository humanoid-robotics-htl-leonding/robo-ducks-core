import { Component, OnInit, Input } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';
import { ActivatedRoute } from '@angular/router';
import { NaoService } from 'src/app/service/nao.service';
import { Tab } from 'src/app/model/tab';
import { DebugMessageHeader } from 'src/app/model/message';
import { Sink, write_str, write_u8, write_u16, write_u32 } from 'ts-binary';

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

  constructor(public dialog: MatDialog, private naoService: NaoService) {
    this.tab = this.naoService.tabs.find(t => t.id == this.id);
  }

  ngOnInit() {
  }

  connectToNao(){
    const clientfields = this.address.split(':');
    this.naoService.setUpClient(this.id, clientfields[0],clientfields[1]);
    this.naoService.connect(this.id);
  }

  toggleFormField(){
    this.toggleForm = !this.toggleForm;
  }

  send(){
    const buffer = new ArrayBuffer(0);
    let data: Sink = Sink(buffer);
    let dmsgh  = new DebugMessageHeader();
    // data = write_str(data, dmsgh.aheader);
    // data = write_u8(data, dmsgh.bversion);
    // data = write_u8(data, dmsgh.cmsgType);
    // data = write_u16(data, dmsgh.dpadding_);
    // data = write_u32(data, dmsgh.emsgLength);
    // data = write_u32(data, dmsgh.fpadding);
    // data = write_str(data, dmsgh.gmsg);

    const array = dmsgh.toIntArray();
    console.log(array);
    for(let i = 0; i < array.length; i++){
      data = write_u8(data, array[i]);
    }

    console.log("YEET");
    console.log(data);
    console.log('Start');
    for(let v = 0; v < data.view.byteLength; v++)
    {
      console.log(data.view.getUint8(v));
    }
    console.log('Fin');
    //console.log(new Uint8Array(data.buffer));

    this.naoService.send(this.id,data);
  }

}
