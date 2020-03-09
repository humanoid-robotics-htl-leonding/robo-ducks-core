import { Component, OnInit, Inject } from '@angular/core';
import { NaoConnector } from 'src/app/model/nao-connector';
import { CONTAINER_DATA } from 'src/app/app.component';
import { NaoService } from 'src/app/service/nao.service';

@Component({
  selector: 'app-nao-text',
  templateUrl: './nao-text.component.html',
  styleUrls: ['./nao-text.component.scss']
})
export class NaoTextComponent implements OnInit {

  connector: NaoConnector;

  constructor(private naoService: NaoService, @Inject(CONTAINER_DATA) public id: number) { }

  ngOnInit(): void {
    this.connector = this.naoService.tabs.find(t => t.id == this.id).connector;
  }

}
