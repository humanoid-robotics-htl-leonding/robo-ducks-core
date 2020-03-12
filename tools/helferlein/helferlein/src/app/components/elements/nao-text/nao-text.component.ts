import { Component, OnInit, Inject } from '@angular/core';
import { NaoConnector } from 'src/app/model/nao-connector';
import { CONTAINER_DATA } from 'src/app/app.component';
import { NaoService } from 'src/app/service/nao.service';

import {map, startWith} from 'rxjs/operators';
import { FormControl } from '@angular/forms';
import { Observable } from 'rxjs';

@Component({
  selector: 'app-nao-text',
  templateUrl: './nao-text.component.html',
  styleUrls: ['./nao-text.component.scss']
})
export class NaoTextComponent implements OnInit {

  connector: NaoConnector;
  

  myControl = new FormControl();
  filteredOptions: Observable<string[]>;


  constructor(private naoService: NaoService, @Inject(CONTAINER_DATA) public id: number) { }

  ngOnInit(): void {
    this.connector = this.naoService.tabs.find(t => t.id == this.id).connector;
    
    this.filteredOptions = this.myControl.valueChanges
    .pipe(
      startWith(''),
      map(value => this._filter(value))
    );

  }

  private _filter(value: string): string[] {
    const filterValue = value.toLowerCase();


    return this.connector.keys.filter(key => key.toString().toLowerCase().includes(filterValue));

 //   return this.options.filter(option => option.toLowerCase().includes(filterValue));

   // return this.connector.keys;

  }
}
