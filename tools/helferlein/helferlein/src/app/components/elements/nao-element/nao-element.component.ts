import { Component, OnInit, Input, Injector, InjectionToken } from '@angular/core';
import { ComponentPortal, PortalInjector } from '@angular/cdk/portal';
import { NaoImageComponent } from '../nao-image/nao-image.component';
import { NaoTextComponent } from '../nao-text/nao-text.component';
import { NaoMapComponent } from '../nao-map/nao-map.component';
import { NaoConfigComponent } from '../nao-config/nao-config.component';
import { NaoConnector } from 'src/app/model/nao-connector';

export const CONTAINER_DATA = new InjectionToken<{}>('CONTAINER_DATA');

@Component({
  selector: 'app-nao-element',
  templateUrl: './nao-element.component.html',
  styleUrls: ['./nao-element.component.scss']
})
export class NaoElementComponent implements OnInit {

  @Input() type = 'text';
  @Input() connector: NaoConnector;
  portal;

  constructor(private injector: Injector) { }

  ngOnInit(): void {
    switch(this.type){
      case 'text':{
        this.portal = new ComponentPortal<NaoTextComponent>(NaoTextComponent, null, this.getElementInjector());
        break;
      }
      case 'image':{
        this.portal = new ComponentPortal<NaoImageComponent>(NaoImageComponent, null, this.getElementInjector());
        break;
      }
      case 'map':{
        this.portal = new ComponentPortal<NaoMapComponent>(NaoMapComponent, null, this.getElementInjector());
        break;
      }
      case 'config':{
        this.portal = new ComponentPortal<NaoConfigComponent>(NaoConfigComponent, null, this.getElementInjector());
        break;
      }
    }
  }

  getElementInjector(): PortalInjector{
    const injectorTokens = new WeakMap();
    injectorTokens.set(CONTAINER_DATA, this.connector);
    return new PortalInjector(this.injector, injectorTokens);
  }

}
