import { Component, OnInit, Input, Injector, InjectionToken } from '@angular/core';
import { ComponentPortal, PortalInjector } from '@angular/cdk/portal';
import { NaoImageComponent } from '../nao-image/nao-image.component';
import { NaoTextComponent } from '../nao-text/nao-text.component';
import { NaoMapComponent } from '../nao-map/nao-map.component';
import { NaoConfigComponent } from '../nao-config/nao-config.component';
import { NaoConnector } from 'src/app/model/nao-connector';
import { CONTAINER_DATA } from 'src/app/app.component';
import { NaoService } from 'src/app/service/nao.service';

@Component({
  selector: 'nao-element',
  templateUrl: './nao-element.component.html',
  styleUrls: ['./nao-element.component.scss']
})
export class NaoElementComponent implements OnInit {

  @Input() type = null;
  @Input() id: number;
  connector: NaoConnector;
  portal;

  constructor(private naoService: NaoService, private injector: Injector) { }

  ngOnInit(): void {
    console.log('Element: ',this.id);
    this.connector = this.naoService.tabs.find(t => t.id == this.id).connector;
    if(this.type){
      this.summonElement(this.type);
    }
  }

  getElementInjector(): PortalInjector {
    const injectorTokens = new WeakMap();
    injectorTokens.set(CONTAINER_DATA, this.id);
    return new PortalInjector(this.injector, injectorTokens);
  }

  summonElement(type: string){
    switch (type) {
      case 'text': {
        this.portal = new ComponentPortal<NaoTextComponent>(NaoTextComponent, null, this.getElementInjector());
        break;
      }
      case 'image': {
        this.portal = new ComponentPortal<NaoImageComponent>(NaoImageComponent, null, this.getElementInjector());
        break;
      }
      case 'map': {
        this.portal = new ComponentPortal<NaoMapComponent>(NaoMapComponent, null, this.getElementInjector());
        break;
      }
      case 'config': {
        this.portal = new ComponentPortal<NaoConfigComponent>(NaoConfigComponent, null, this.getElementInjector());
        break;
      }
    }
    this.type = type;
  }
}
