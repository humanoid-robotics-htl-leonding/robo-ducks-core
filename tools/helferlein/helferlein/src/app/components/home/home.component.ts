import { Component, OnInit } from '@angular/core';
import { NaoComponent } from '../nao/nao.component';
import { Tab } from 'src/app/model/tab';
import { NaoService } from 'src/app/service/nao.service';

@Component({
  selector: 'app-home',
  templateUrl: './home.component.html',
  styleUrls: ['./home.component.scss']
})
export class HomeComponent implements OnInit {
  tabs: Tab[];

  constructor(private naoService: NaoService) { }

  ngOnInit() {
  }

  openTab(){
    this.naoService.addTab();
    this.loadTabs();
  }

  closeTab(id: number){
    this.naoService.removeTab(id);
    this.loadTabs();
  }

  loadTabs(){
    this.tabs = this.naoService.tabs;
  }

}
