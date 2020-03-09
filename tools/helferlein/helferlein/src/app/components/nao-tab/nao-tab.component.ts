import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'app-nao-tab',
  templateUrl: './nao-tab.component.html',
  styleUrls: ['./nao-tab.component.scss']
})
export class NaoTabComponent implements OnInit {

  constructor() { }

  status = 'New';

  ngOnInit() {
  }

}
