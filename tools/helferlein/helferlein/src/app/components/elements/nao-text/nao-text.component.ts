import { Component, OnInit, Inject, PipeTransform, Pipe } from '@angular/core';
import { NaoConnector } from 'src/app/model/nao-connector';
import { CONTAINER_DATA } from 'src/app/app.component';
import { NaoService } from 'src/app/service/nao.service';

import {map, startWith} from 'rxjs/operators';
import { FormControl } from '@angular/forms';
import { Observable } from 'rxjs';

@Component({
  selector: 'app-nao-text',
  templateUrl: './nao-text.component.html',
  styleUrls: ['./nao-text.component.scss'],
})
export class NaoTextComponent implements OnInit {

  connector: NaoConnector;

  myControl = new FormControl();
  filteredOptions: Observable<string[]>;
  values: Observable<string[]>;


  constructor(private naoService: NaoService, @Inject(CONTAINER_DATA) public id: number) { }

  ngOnInit(): void {
    this.connector = this.naoService.tabs.find(t => t.id == this.id).connector;
    
    this.filteredOptions = this.myControl.valueChanges
    .pipe(
      startWith(''),
      map(value => this._filter(value))
    );

    this.values = this.myControl.valueChanges
    .pipe(
      startWith(''),
      map(value => this._keyFilter(value))
    );

  }
  _keyFilter(value: string): string[] {

    const filterKey = value;
    const respObj = this.connector.keys.find(k => k.key.toLowerCase() == filterKey.toLowerCase());

    //console.log("Help " + this.connector.keys.find(k => k.key.toLowerCase() == filterKey.toLowerCase()).value);

    return respObj == undefined ? undefined : Array.of(JSON.stringify(respObj.value, undefined, 3).replace(/\n/g, '<br/>').replace(/ /g, '&nbsp;'));

    //return this.connector.keys.map(v => v.value).filter(value => value.toString().toLowerCase().includes(filterKey));

  }

  private _filter(value: string): string[] {
    const filterValue = value.toLowerCase();
  
    return this.connector.keys.map(k => k.key).filter(key => key.toString().toLowerCase().includes(filterValue));

 //   return this.options.filter(option => option.toLowerCase().includes(filterValue));

   // return this.connector.keys;

  }
}
