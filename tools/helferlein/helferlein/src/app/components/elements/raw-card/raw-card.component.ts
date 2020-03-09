import { Component, OnInit, Input } from '@angular/core';

@Component({
  selector: 'app-raw-card',
  templateUrl: './raw-card.component.html',
  styleUrls: ['./raw-card.component.scss']
})
export class RawCardComponent implements OnInit {

  @Input() content;

  constructor() { }

  ngOnInit() {
  }

}
