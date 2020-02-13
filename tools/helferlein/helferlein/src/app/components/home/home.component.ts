import { Component, OnInit, ChangeDetectorRef, ApplicationRef } from '@angular/core';
import { NaoComponent } from '../nao/nao.component';
import { Tab } from 'src/app/model/tab';
import { NaoService } from 'src/app/service/nao.service';
import { FormControl } from '@angular/forms';
declare var electron: any;
const menu = electron.remote.Menu;

@Component({
  selector: 'app-home',
  templateUrl: './home.component.html',
  styleUrls: ['./home.component.scss']
})
export class HomeComponent implements OnInit {
  tabs: Tab[] = [];
  selected = new FormControl(0);

  constructor(private naoService: NaoService, private changeRef: ChangeDetectorRef) { }

  ngOnInit() {
    const customMenu = menu.buildFromTemplate(
      [
        {
          label: 'File',
          submenu: [
            {
              label: 'Tab',
              submenu: [
                {
                  label: 'New',
                  click: () => {
                    this.openTab();
                  },
                  accelerator: 'CmdOrCtrl+N'
                },
                {
                  label: 'Close',
                  click: () => {
                    this.closeTab(this.selected.value);
                  },
                  accelerator: 'CmdOrCtrl+W'
                }
              ]
            },
            {label: 'Delete'},
            {type: 'separator'},

          ]
        }
      ]
    );
    menu.setApplicationMenu(customMenu);
    this.openTab();
  }

  openTab(){
    this.naoService.addTab();
    this.loadTabs();
    this.selectTab(this.tabs.length - 1);
    console.log(this.tabs);
  }

  closeTab(id: number){
    if(id < this.tabs.length){
      this.naoService.removeTab(id);
      this.loadTabs();
      if(id == this.selected.value && id > this.tabs.length - 1){
        this.selected.setValue(this.selected.value-1);
      }
    }
  }

  loadTabs(){
    this.tabs = this.naoService.tabs;
    this.changeRef.detectChanges();
  }

  selectTab(index){
    console.log('Index', index);
    if(index>=this.tabs.length){
      this.selected.setValue(this.tabs.length-1);
    }
    else{
      this.selected.setValue(index);
    }
    console.log('Selected', this.selected);
    console.log(this.tabs);
  }

  // async delay(ms: number) {
  //   await new Promise(resolve => setTimeout(()=>resolve(), ms)).then(()=>console.log("fired"));
  // }

}
