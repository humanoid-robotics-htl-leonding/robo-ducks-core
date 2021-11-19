import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { NaoComponent } from './components/nao/nao.component';
import { HomeComponent } from './components/home/home.component';
import { NaoTabComponent } from './components/nao-tab/nao-tab.component';


const routes: Routes = [
  {
    path: '',
    component: HomeComponent
  }
];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { }
