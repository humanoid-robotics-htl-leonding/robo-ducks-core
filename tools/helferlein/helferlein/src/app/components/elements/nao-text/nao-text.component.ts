import { Component, OnInit, Inject } from '@angular/core';
import { NaoConnector } from 'src/app/model/nao-connector';
import { CONTAINER_DATA } from 'src/app/app.component';

@Component({
  selector: 'app-nao-text',
  templateUrl: './nao-text.component.html',
  styleUrls: ['./nao-text.component.scss']
})
export class NaoTextComponent implements OnInit {

  @Inject(CONTAINER_DATA) public connector: NaoConnector;

  constructor() { }

  ngOnInit(): void {

  }

}
