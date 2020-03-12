import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { FormsModule } from '@angular/forms';
import { MatDialogModule } from '@angular/material/dialog';
import { MatInputModule } from '@angular/material/input';
import { MatButtonModule } from '@angular/material/button';
import { MatToolbarModule } from '@angular/material/toolbar';
import { MatTabsModule } from '@angular/material/tabs';
import { MatIconModule } from '@angular/material/icon';
import { MatCardModule } from '@angular/material/card';
import { PortalModule } from '@angular/cdk/portal';
import { EcoFabSpeedDialModule } from '@ecodev/fab-speed-dial';
import { MatSlideToggleModule } from '@angular/material/slide-toggle';
import { MatTooltipModule } from '@angular/material/tooltip';
import { MatGridListModule } from '@angular/material/grid-list';
import { MatMenuModule } from '@angular/material/menu';
import {ClipboardModule} from '@angular/cdk/clipboard';
import {MatAutocompleteModule} from '@angular/material/autocomplete';
import {ReactiveFormsModule} from '@angular/forms';

import { NaoComponent } from './components/nao/nao.component';
import { NaoTabComponent } from './components/nao-tab/nao-tab.component';
import { HomeComponent } from './components/home/home.component';
import { RawCardComponent } from './components/elements/raw-card/raw-card.component';
import { NaoElementComponent } from './components/elements/nao-element/nao-element.component';
import { NaoImageComponent } from './components/elements/nao-image/nao-image.component';
import { NaoTextComponent } from './components/elements/nao-text/nao-text.component';
import { NaoMapComponent } from './components/elements/nao-map/nao-map.component';
import { NaoConfigComponent } from './components/elements/nao-config/nao-config.component';

@NgModule({
  declarations: [
    AppComponent,
    NaoComponent,
    NaoTabComponent,
    HomeComponent,
    RawCardComponent,
    NaoElementComponent,
    NaoImageComponent,
    NaoTextComponent,
    NaoMapComponent,
    NaoConfigComponent
  ],
  imports: [
    BrowserModule,
    AppRoutingModule,
    BrowserAnimationsModule,
    FormsModule,
    MatDialogModule,
    MatInputModule,
    MatButtonModule,
    MatToolbarModule,
    MatTabsModule,
    MatIconModule,
    MatCardModule,
    PortalModule,
    EcoFabSpeedDialModule,
    MatSlideToggleModule,
    MatTooltipModule,
    MatGridListModule,
    MatMenuModule,
    ClipboardModule,
    MatAutocompleteModule,
    ReactiveFormsModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
