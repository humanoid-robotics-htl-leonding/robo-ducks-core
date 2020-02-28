import { Component, InjectionToken } from '@angular/core';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent {
  title = 'helferlein';
}

export const CONTAINER_DATA = new InjectionToken<{}>('CONTAINER_DATA');
